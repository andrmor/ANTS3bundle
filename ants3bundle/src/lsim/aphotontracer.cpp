#include "aphotontracer.h"
#include "asensorhub.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "ageometryhub.h"
#include "aphotonsimhub.h"
#include "arandomhub.h"
#include "astatisticshub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "aphotonstatistics.h"
#include "aoneevent.h"
#include "agridhub.h"
#include "agridelementrecord.h"
#include "aphoton.h"
#include "amonitor.h"
#include "amonitorhub.h"
#include "aphotontunnelhub.h"

#include <QDebug>
#include <QTextStream>

#include "TGeoManager.h"
#include "TMath.h"
#include "TH1D.h"

APhotonTracer::APhotonTracer(AOneEvent & event, QTextStream* & streamTracks, QTextStream* & streamSensorLog) :
    MatHub(AMaterialHub::getConstInstance()),
    RuleHub(AInterfaceRuleHub::getConstInstance()),
    SensorHub(ASensorHub::getConstInstance()),
    GridHub(AGridHub::getConstInstance()),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    SimStat(AStatisticsHub::getInstance().SimStat),
    Event(event),
    StreamTracks(streamTracks),
    StreamSensorLog(streamSensorLog)
{}

void APhotonTracer::configureTracer()
{
    Track.Positions.reserve(SimSet.OptSet.MaxPhotonTransitions + 1);
    AddedTracks = 0;

    _MaxQE = SensorHub.getMaxQE(SimSet.WaveSet.Enabled);
}

void APhotonTracer::initTracks()
{
    if (AddedTracks < SimSet.RunSet.MaxTracks)  // !!!*** move to the caller?
    {
        Track.HitSensor = false;
        Track.SecondaryScint = Photon.SecondaryScint;
        Track.Positions.clear();
        Track.Positions.push_back(AVector3(Photon.r));
    }
    else APhotonSimHub::getInstance().Settings.RunSet.SaveTracks = false; // SimSet is read-only
}

void APhotonTracer::initPhotonLog()
{
    PhLog.clear();
    PhLog.reserve(SimSet.OptSet.MaxPhotonTransitions);
    PhLog.push_back( APhotonHistoryLog(Photon.r, Navigator->GetCurrentVolume()->GetName(), Photon.time, Photon.waveIndex, APhotonHistoryLog::Created, MatIndexFrom) );
}

bool APhotonTracer::initBeforeTracing(const APhoton & phot)
{
    GeoManager = AGeometryHub::getInstance().GeoManager;
    Navigator = GeoManager->GetCurrentNavigator();
    Navigator->SetCurrentPoint(phot.r);
    Navigator->SetCurrentDirection(phot.v);
    Navigator->FindNode();
    if (Navigator->IsOutside())
    {
        SimStat.GeneratedOutside++;
        if (SimSet.RunSet.SavePhotonLog)
        {
            PhLog.clear();
            PhLog.push_back( APhotonHistoryLog(phot.r, "", phot.time, phot.waveIndex, APhotonHistoryLog::GeneratedOutsideGeometry) );
        }
        return false;
    }

//    qDebug()<<"Photon starts from:";
//    qDebug()<<Navigator->GetPath();
//    qDebug()<<"material name: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetName();
//    qDebug()<<"material index: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetIndex();

    Photon.copyFrom(phot);

    if (SimSet.RunSet.SaveTracks)    initTracks();
    if (SimSet.RunSet.SavePhotonLog) initPhotonLog();

    bGridShiftOn = false;

    return true;
}

void APhotonTracer::tracePhoton(const APhoton & phot)
{
//    qDebug() << "Photon tracing started";

    if (SimSet.OptSet.CheckQeBeforeTracking)
    {
        //if (skipTracing(phot.waveIndex)) return;  // in contrast to ANTS2, do not assume that wavelength will be constant during tracing
        Rnd = RandomHub.uniform();
        if (Rnd > _MaxQE)
        {
            SimStat.TracingSkipped++;
            return;
        }
    }

    if (!initBeforeTracing(phot)) return;

    TransitionCounter = 0;
    while (TransitionCounter < SimSet.OptSet.MaxPhotonTransitions)
    {
        TransitionCounter++;

        VolumeFrom = Navigator->GetCurrentVolume();
        if (VolumeFrom)
            MatIndexFrom = VolumeFrom->GetMaterial()->GetIndex();
        MaterialFrom = MatHub[MatIndexFrom];                             // this is the material where the photon is currently in
        if (SimSet.RunSet.SavePhotonLog)
            NameFrom = Navigator->GetCurrentVolume()->GetName();
        SpeedOfLight = MaterialFrom->getSpeedOfLight(Photon.waveIndex);  // [mm/ns]

        Navigator->FindNextBoundary();
        Step = Navigator->GetStep();  //qDebug()<<"Step:"<<Step;

        switch ( checkBulkProcesses() )
        {
            case EBulkProcessResult::Absorbed    : endTracing(); return; // finished with this photon
            case EBulkProcessResult::Scattered   : continue;             // next transition
            case EBulkProcessResult::WaveShifted : continue;             // next transition
            default:; // not triggered
        }

        //--- Making a step towards the interface ---
        Navigator->Step(true, false); //stopped right before the crossing

        if (bGridShiftOn && Step > 0.001 && isOutsideGridBulk()) // !!!*** why 0.001?
        {
            endTracing();
            return;
        }

        //--- Placing this location/state to the stack. It will be restored if photon is reflected, otherwise dumped ---
        Navigator->PushPoint(); //DO NOT FORGET TO POP OR CLEAN !

        //--- Can make the track now - the photon made it to the other border in any case ---
        Photon.time += Step / SpeedOfLight;
        if (SimSet.RunSet.SaveTracks && Step > 0.001) // !!!*** hard coded 0.001
        {
            if (bGridShiftOn) Track.Positions.push_back(AVector3(R_afterStep));
            else              Track.Positions.push_back(AVector3(Navigator->GetCurrentPoint()));
        }

        //--- Now entering the next material volume on the path ---
        TGeoNode * NodeAfter = Navigator->FindNextBoundaryAndStep(); //this is the node after crossing the boundary
        //this method MOVES the current position! different from FindNextBoundary method, which only calculates the step
        //now the current point is inside the next volume!

        if (isPhotonEscaped())
        {
            endTracing();
            return;
        }

        VolumeTo = NodeAfter->GetVolume();
        if (SimSet.RunSet.SavePhotonLog) NameTo = Navigator->GetCurrentVolume()->GetName();
        MatIndexTo = VolumeTo->GetMaterial()->GetIndex();
        MaterialTo = MatHub[MatIndexTo];
        bHaveNormal = false;
//        qDebug()  << "Found border with another volume: " << VolumeTo->GetName();
//        qDebug()  << "Mat index after interface: " << MatIndexTo << " Mat index before: " << MatIndexFrom;
//        qDebug()  << "Coordinates: "<<Navigator->GetCurrentPoint()[0]<<Navigator->GetCurrentPoint()[1]<<Navigator->GetCurrentPoint()[2];

        //--- Check interface rule ---
        const EInterRuleResult res = tryInterfaceRule();
        bUseLocalNormal = (res == EInterRuleResult::DelegateLocalNormal);
        switch (res)
        {
            case EInterRuleResult::NotTriggered        : bDoFresnel = true;  break;
            case EInterRuleResult::DelegateLocalNormal : bDoFresnel = true;  break; // same as NotTriggered, but with bUseLocalNormal on
            case EInterRuleResult::Transmitted         : bDoFresnel = false; break;
            case EInterRuleResult::Reflected           : continue;                  // stack cleaned inside
            case EInterRuleResult::Absorbed            : endTracing(); return;      // stack cleaned inside
        }

        //--- Interface rule not set or not triggered (but local normal might have changed!) ---
        if (bDoFresnel)
        {
            const EFresnelResult res = tryReflection();
            if (res == EFresnelResult::Reflected) continue;
        }

        //--- Photon entered another volume ---
        Navigator->PopDummy(); //clean up the stack

        if (bGridShiftOn && Step > 0.001) exitGrid();

        bool returnEndTracingFlag;
        checkSpecialVolume(NodeAfter, returnEndTracingFlag);
        if (returnEndTracingFlag)
        {
            endTracing();
            return;
        }

        if (bDoFresnel)
        {
            const bool ok = performRefraction(); // if local normal is defined, the photon direction can be pointing towards the interface!
            // true - successful, false - forbidden -> considered that the photon is absorbed at the surface! Should not happen
            if (!ok) qWarning()<<"Error in photon tracker: problem with transmission!";
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
        }

    } //if below max number of transitions, process next (or reflect back to stay in the same) volume

    if (TransitionCounter == SimSet.OptSet.MaxPhotonTransitions) SimStat.MaxTransitions++;
    endTracing();
}

void APhotonTracer::endTracing()
{
    if (SimSet.RunSet.SavePhotonLog) appendHistoryRecord(); //Add tracks is also there, it has extra filtering   !!!*** not parallel!!!
    if (SimSet.RunSet.SaveTracks)    saveTrack();
//    qDebug() << "Finished with the photon\n\n";
}

EInterRuleResult APhotonTracer::tryInterfaceRule()
{
    InterfaceRule = getInterfaceRule();
    if (!InterfaceRule) return EInterRuleResult::NotTriggered;

    //qDebug() << "Interface rule defined! Model = "<<ov->getType();
    N = Navigator->FindNormal(false); bHaveNormal = true;
    const double * PhPos = Navigator->GetCurrentPoint();
    for (int i=0; i<3; i++) Photon.r[i] = PhPos[i];

tryAgainLabel:
    AInterfaceRule::OpticalOverrideResultEnum result = InterfaceRule->calculate(&Photon, N);

    switch (result)
    {
    case AInterfaceRule::Absorbed:
        Navigator->PopDummy();
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Override_Loss, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleLoss++;
        return EInterRuleResult::Absorbed;
    case AInterfaceRule::Back:
        if (InterfaceRule->isNotPolishedSurface())
        {
            if (Photon.v[0]*N[0] + Photon.v[1]*N[1] + Photon.v[2]*N[2] > 0) //back only in respect to the local normal but actually forward considering global one
            {
                //qDebug() << "Rule result is 'Back', but direction is actually 'Forward' --> re-running the rule";
                goto tryAgainLabel;
            }
        }
        Navigator->PopPoint();
        Navigator->SetCurrentDirection(Photon.v);
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Override_Back, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleBack++;
        return EInterRuleResult::Reflected;
    case AInterfaceRule::Forward:
        // with local normal modified, it can happen that the photon direction 'points' towards the interface again --> this situation is handled normally
        Navigator->SetCurrentDirection(Photon.v);
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Override_Forward, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleForward++;
        return EInterRuleResult::Transmitted;  // stack cleaned afterwards
    case AInterfaceRule::DelegateLocalNormal:
        return EInterRuleResult::DelegateLocalNormal;
    case AInterfaceRule::NotTriggered:
        // if nothing at all happened (also local normal was NOT introduced)
        return EInterRuleResult::NotTriggered; // stack cleaned afterwards
    default:
        qCritical() << "override error - doing fresnel instead!";
        return EInterRuleResult::NotTriggered; // stack cleaned afterwards
    }
}

EFresnelResult APhotonTracer::tryReflection()
{
    const double prob = calculateReflectionProbability();
    if (RandomHub.uniform() < prob)
    {
        SimStat.FresnelReflected++;
        Navigator->PopPoint();       // restore the point before the border
        performReflection();         // in the case of modified local normal, photon can point towards the interface again!
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Fresnel_Reflection, MatIndexFrom, MatIndexTo) );
        return EFresnelResult::Reflected;
    }

    SimStat.FresnelTransmitted++;
    return EFresnelResult::Transmitted;  // stack cleaned afterwards
}

bool APhotonTracer::isOutsideGridBulk()
{   //if point after Step is outside of grid bulk, kill this photon
    // it is not realistic in respect of Rayleigh for photons travelling in grid plane
    //also, there is a small chance that photon is rejected by this procedure when it hits the grid wire straight on center
    //chance is very small (~0.1%) even for grids with ~25% transmission value

    const double * rc = Navigator->GetCurrentPoint();
    //qDebug() << "...Checking after step of " << Step << "still inside grid bulk?";
    //qDebug() << "...Position in world:"<<rc[0] << rc[1] << rc[2];
    //qDebug() << "...Navigator is supposed to be inside the grid element. Currently in:"<<navigator->GetCurrentNode()->GetVolume()->GetName();

    for (int i = 0; i < 3; i++) R_afterStep[i] = rc[i] + FromGridToGlobal[i];
    //qDebug() << "...After correction, coordinates in true global:"<<rc[0] << rc[1] << rc[2];

    double GridLocal[3];
    Navigator->MasterToLocal(rc, GridLocal);
    //qDebug() << "...Position in grid element:"<<GridLocal[0] << GridLocal[1] << GridLocal[2];
    for (int i = 0; i < 3; i++) GridLocal[i] += FromGridElementToGridBulk[i];
    //qDebug() << "...Position in grid bulk:"<<GridLocal[0] << GridLocal[1] << GridLocal[2];

    if (GridVolume->Contains(GridLocal)) return false;

    //qDebug() << "...!!!...Photon would be outside the bulk if it takes the Step! -> killing the photon";
    //qDebug() << "Conflicting position:"<<R_afterStep[0] << R_afterStep[1] << R_afterStep[2];
    SimStat.LossOnGrid++;
    return true;
}

void APhotonTracer::exitGrid()
{
    //qDebug() << "++Grid back shift triggered!";
    if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Grid_ShiftOut) );

    //qDebug() << "<--Returning from grid shift!";
    //qDebug() << "<--Shifted coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--Currently in"<<navigator->FindNode()->GetName();

    const double * r = Navigator->GetCurrentPoint();
    double R[3];
    R[0] = r[0] + FromGridToGlobal[0];
    R[1] = r[1] + FromGridToGlobal[1];
    R[2] = r[2] + FromGridToGlobal[2];
    Navigator->SetCurrentPoint(R);
    Navigator->FindNode();

    if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Grid_Exit) );
    //qDebug() << "<--True coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--After back shift in"<<navigator->FindNode()->GetName();
    bGridShiftOn = false;
}

void APhotonTracer::checkSpecialVolume(TGeoNode * NodeAfterInterface, bool & returnEndTracingFlag)
{
    //const char* VolName = ThisVolume->GetName();
    //qDebug()<<"Photon entered new volume:" << VolName;
    const char* VolTitle = VolumeTo->GetTitle();
    //qDebug() << "Title:"<<VolTitle;

    // volumes without special role have titles statring with '-'
    const char Selector = VolTitle[0];

    if (Selector == '-') return; // Not a special volume

    if (Selector == 'S') // Sensor
    {
        const int iSensor = NodeAfterInterface->GetNumber();
        //qDebug()<< "Sensor hit! (" << ThisVolume->GetTitle() <<") Sensor name:"<< ThisVolume->GetName() << "Sensor index" << iSensor;
        if (SimSet.RunSet.SavePhotonLog)
        {
            PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
            PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::HitSensor, -1, -1, iSensor) );
        }
        Track.HitSensor = true;
        processSensorHit(iSensor);
        SimStat.HitSensor++;
        returnEndTracingFlag = true;
        return;
    }

    if (Selector == 'G') // Optical grid
    {
        //qDebug() << "Grid hit!" << ThisVolume->GetName() << ThisVolume->GetTitle()<< "Number:"<<NodeAfterInterface->GetNumber();
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Grid_Enter) );
        enterGrid(NodeAfterInterface->GetNumber()); // it is assumed that "empty part" of the grid element will have the same refractive index as the material from which photon enters it
        GridVolume = VolumeTo;
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::Grid_ShiftIn) );
        returnEndTracingFlag = false;
        return;
    }

    if (Selector == 'M') // Monitor
    {
        AMonitorHub & MonitorHub = AMonitorHub::getInstance();
        const int iMon = NodeAfterInterface->GetNumber();
        //qDebug() << "Monitor hit!" << ThisVolume->GetName() << "Number:"<<iMon;// << MatIndexFrom<<MatIndexTo;
        double local[3];
        const double * global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, local);
        //qDebug()<<local[0]<<local[1];
        //qDebug() << "Monitors:"<<SimStat.Monitors.size();
        if ( (local[2] > 0 && MonitorHub.PhotonMonitors[iMon].Monitor->isUpperSensitive()) ||
             (local[2] < 0 && MonitorHub.PhotonMonitors[iMon].Monitor->isLowerSensitive()) )
        {
            //angle?
            if (!bHaveNormal) N = Navigator->FindNormal(false);
            double cosAngle = 0;
            for (int i=0; i<3; i++) cosAngle += N[i] * Photon.v[i];
            MonitorHub.PhotonMonitors[iMon].Monitor->fillForPhoton(local[0], local[1], Photon.time, 180.0/3.1415926535*TMath::ACos(cosAngle), Photon.waveIndex);
            if (MonitorHub.PhotonMonitors[iMon].Monitor->isStopsTracking())
            {
                SimStat.MonitorKill++;
                if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameTo, Photon.time, Photon.waveIndex, APhotonHistoryLog::KilledByMonitor) );
                returnEndTracingFlag = true;
                return;
            }
        }
        returnEndTracingFlag = false;
        return;
    }

    if (Selector == 'T') // Photon tunnel In
    {
        const int inNum = NodeAfterInterface->GetNumber();
        //qDebug() << "Enter photon tunnel #" << inNum;
        const APhotonTunnelHub & PhTunHub = APhotonTunnelHub::getConstInstance();
        const size_t outNum =  PhTunHub.RuntimeData[inNum].ExitIndex;
        //qDebug() << "Exit photon tunnel #" << outNum;
        const std::tuple<AGeoObject*,TGeoNode*, AVector3> & out = AGeometryHub::getConstInstance().PhotonTunnelsOut[outNum];

        //TGeoNode * nodeOut = std::get<1>(out);
        //qDebug() << "out node name:" <<  nodeOut->GetVolume()->GetName();

        // get "In" local position
        double localPos[3];
        const double * global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, localPos);
          // set center Z in tunnel out
        localPos[2] = 0;

        // get "In" local vector
        const double * globalDir = Navigator->GetCurrentDirection();
        double localDir[3];
        Navigator->MasterToLocalVect(globalDir, localDir);

        //qDebug() << "local coordinates" << localPos[0] << localPos[1] << localPos[2];
        //qDebug() << "local vector" << localDir[0] << localDir[1] << localDir[2];

        // find exit node
        const AVector3 & globPosNodeCenter = std::get<2>(out);
        Navigator->FindNode(globPosNodeCenter[0], globPosNodeCenter[1], globPosNodeCenter[2]);

        // set new position
        Navigator->LocalToMaster(localPos, Photon.r);
        Navigator->SetCurrentPoint(Photon.r);

        // set new vector
        Navigator->LocalToMasterVect(localDir, Photon.v);
        Navigator->SetCurrentDirection(Photon.v);

        Track.Positions.push_back(AVector3(Navigator->GetCurrentPoint()));
    }

    returnEndTracingFlag = false;
}

bool APhotonTracer::isPhotonEscaped()
{
    if (Navigator->IsOutside())
    {
        Navigator->PopDummy();
        SimStat.Escaped++;
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Escaped) );
        return true;
    }
    return false;
}

AInterfaceRule * APhotonTracer::getInterfaceRule() const
{
    AInterfaceRule * rule = nullptr;

    // 1. Check Name-to-Name rule exists
    // Title: ---- for "normal", adds base name for instances
    // Title[0]!='-' volume has a special role, Title[1]='*' there are named rules from this volume, Title[2]='*' there are named rules from this vilume
    if (VolumeFrom->GetTitle()[1] == '*' && VolumeTo->GetTitle()[2] == '*')
    {
        //qDebug() << VolumeFrom->GetName() << VolumeFrom->GetTitle() << (int)VolumeFrom->GetTitle()[4];
        //qDebug() << "->" << VolumeTo->GetName() << VolumeTo->GetTitle() << (int)VolumeTo->GetTitle()[4];

        TString fromName;
        if (VolumeFrom->GetTitle()[4] == 0)
        {
            // not an intance
            fromName = VolumeFrom->GetName();
            if (VolumeFrom->GetTitle()[0] != '-') // can be monitor or calorimeter, then the name contains _-_ and index
                AGeometryHub::getConstInstance().removeNameDecorators(fromName);
        }
        else
        {
            // instance, Title from index 4 contains base name
            fromName = VolumeFrom->GetTitle();
            fromName.Remove(0, 4);
        }
        TString toName;
        if (VolumeTo->GetTitle()[4] == 0)
        {
            // not an intance
            toName = VolumeTo->GetName();
            if (VolumeTo->GetTitle()[0] != '-') // can be monitor or calorimeter, then the name contains _-_ and index
                AGeometryHub::getConstInstance().removeNameDecorators(toName);
        }
        else
        {
            // instance, Title from index 4 contains base name
            toName = VolumeTo->GetTitle();
            toName.Remove(0, 4);
        }

        rule = RuleHub.getVolumeRule(fromName, toName);
        if (rule) rule->updateMatIndices(MatIndexFrom, MatIndexTo);
    }

    // 2. if not, check Materil-tomaterial rule exists
    if (!rule) rule = RuleHub.getMaterialRuleFast(MatIndexFrom, MatIndexTo);
    return rule;
}

#include "ajsontools.h"
void APhotonTracer::saveTrack()
{
    AddedTracks++;

    QJsonObject json;
    if (Track.SecondaryScint) json["s"] = 1;
    if (Track.HitSensor)      json["h"] = 1;

    QJsonArray ar;
    for (AVector3 & pos : Track.Positions)
    {
        QJsonArray el;
            for (int i=0; i<3; i++) el.push_back(pos[i]);
        ar.push_back(el);
    }
    json["P"] = ar;

    *StreamTracks << jstools::jsonToString(json) << '\n';
}

void APhotonTracer::appendHistoryRecord()
{
    const APhotonLogSettings & LogSet = SimSet.RunSet.LogSet;
    bool bVeto = false;
    //by process
    if (!LogSet.MustNotInclude_Processes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSet.MustNotInclude_Processes.find(log.process) != LogSet.MustNotInclude_Processes.end() )
            {
                bVeto = true;
                break;
            }
    }
    //by Volume
    if (!bVeto && !LogSet.MustNotInclude_Volumes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSet.MustNotInclude_Volumes.find(log.volumeName) != LogSet.MustNotInclude_Volumes.end() )
            {
                bVeto = true;
                break;
            }
    }

    if (!bVeto)
    {
        bool bFound = true;
        //in processes
        for (size_t im = 0; im < LogSet.MustInclude_Processes.size(); im++)
        {
            bool bFoundThis = false;
            for (int i=PhLog.size()-1; i>-1; i--)
                if ( LogSet.MustInclude_Processes[im] == PhLog[i].process)
                {
                    bFoundThis = true;
                    break;
                }
            if (!bFoundThis)
            {
                bFound = false;
                break;
            }
        }

        //in volumes
        if (bFound)
        {
            for (size_t im = 0; im < LogSet.MustInclude_Volumes.size(); im++)
            {
                bool bFoundThis = false;
                for (int i=PhLog.size()-1; i>-1; i--)
                    if ( LogSet.MustInclude_Volumes[im] == PhLog[i].volumeName)
                    {
                        bFoundThis = true;
                        break;
                    }
                if (!bFoundThis)
                {
                    bFound = false;
                    break;
                }
            }
        }

        if (bFound) savePhotonLogRecord();
    }
}

EBulkProcessResult APhotonTracer::checkBulkProcesses()
{
    //Optimized assuming rare use of Rayleigh and very rare use when both abs and Rayleigh are defined!
    //if both active, extract distances at which the processes would trigger, then select the process with the shortest path
    //qDebug() << "Abs and Ray tests";

    //prepare abs
    bool DoAbsorption;
    double AbsPath;
    const double AbsCoeff = MatHub[MatIndexFrom]->getAbsorptionCoefficient(Photon.waveIndex);
    if (AbsCoeff > 0)
    {
        AbsPath = -log(RandomHub.uniform()) / AbsCoeff;
        if (AbsPath < Step) DoAbsorption = true;
        else DoAbsorption = false;
    }
    else DoAbsorption = false;

    //prepare Rayleigh
    bool DoRayleigh;
    double RayleighPath;
    if (MaterialFrom->RayleighMFP == 0) DoRayleigh = false;      // == 0 - means undefined
    else
    {
        double RayleighMFP;
        if (Photon.waveIndex == -1) RayleighMFP = MaterialFrom->RayleighMFP;
        else RayleighMFP = MaterialFrom->_Rayleigh_WaveBinned[Photon.waveIndex];

        RayleighPath = -RayleighMFP * log(RandomHub.uniform());
        if (RayleighPath < Step) DoRayleigh = true;
        else DoRayleigh = false;
    }

    //checking abs and rayleigh
    //qDebug() << "Abs, Ray, AbsPath, RayPath"<<DoAbsorption<<DoRayleigh<<AbsPath<<RayleighPath;
    if (DoAbsorption || DoRayleigh)
    {
        if (DoAbsorption && DoRayleigh)
        {
            //slecting the one having the shortest path
            if (AbsPath < RayleighPath) DoRayleigh = false;
            else DoAbsorption = false;
        }

        if (DoAbsorption)
        {
            //qDebug()<<"Absorption was triggered!";
            Photon.time += AbsPath / SpeedOfLight;
            SimStat.Absorbed++;
            SimStat.BulkAbsorption++;

            if (SimSet.RunSet.SaveTracks || SimSet.RunSet.SavePhotonLog)
            {
                double point[3];
                point[0] = Navigator->GetCurrentPoint()[0] + Photon.v[0]*AbsPath;
                point[1] = Navigator->GetCurrentPoint()[1] + Photon.v[1]*AbsPath;
                point[2] = Navigator->GetCurrentPoint()[2] + Photon.v[2]*AbsPath;
                if (SimSet.RunSet.SaveTracks) Track.Positions.push_back(AVector3(point));
                if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(point, NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Absorbed, MatIndexFrom) );
            }

            //check if this material is waveshifter
            const double reemissionProb = MatHub[MatIndexFrom]->getReemissionProbability(Photon.waveIndex);
            if ( reemissionProb > 0 )
            {
                if (RandomHub.uniform() < reemissionProb)
                {
                    //qDebug() << "Waveshifting! Original index:"<<p.waveIndex;
                    if (Photon.waveIndex!=-1 && MatHub[MatIndexFrom]->_PrimarySpectrumHist)
                    {
                        double wavelength;
                        int waveIndex;
                        int attempts = -1;
                        do
                        {
                            attempts++;
                            if (attempts > 9) return EBulkProcessResult::Absorbed;  // ***!!! absolute number
                            wavelength = MatHub[MatIndexFrom]->_PrimarySpectrumHist->GetRandom();
                            //qDebug() << "   "<<wavelength << " MatIndexFrom:"<< MatIndexFrom;
                            waveIndex = SimSet.WaveSet.toIndexFast(wavelength); // !!!*** before was round here:
                            //waveIndex = round( (wavelength - SimSet->WaveFrom)/SimSet->WaveStep );
                        }
                        while (waveIndex < Photon.waveIndex); //conserving energy

                        //qDebug() << "NewIndex:"<<waveIndex;
                        Photon.waveIndex = waveIndex;
                    }
                    else Photon.waveIndex = -1; // this is to allow to use this procedure to make diffuse medium (including -1 waveIndex)

                    double R[3];
                    R[0] = Navigator->GetCurrentPoint()[0] + Photon.v[0]*AbsPath;
                    R[1] = Navigator->GetCurrentPoint()[1] + Photon.v[1]*AbsPath;
                    R[2] = Navigator->GetCurrentPoint()[2] + Photon.v[2]*AbsPath;
                    Navigator->SetCurrentPoint(R);

                    Photon.generateRandomDir();
                    Navigator->SetCurrentDirection(Photon.v);
                    //qDebug() << "After:"<<p->WaveIndex;

                    Photon.time += MatHub[MatIndexFrom]->generatePrimScintTime(RandomHub);

                    SimStat.Reemission++;
                    if (SimSet.RunSet.SavePhotonLog)
                        PhLog.push_back( APhotonHistoryLog(R, NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Reemission, MatIndexFrom) );
                    return EBulkProcessResult::WaveShifted;
                }
            }
            //else absorption
            return EBulkProcessResult::Absorbed;
        }

        if (DoRayleigh)
        {
            //qDebug()<<"Scattering was triggered";
            //interaction position
            double R[3];
            R[0] = Navigator->GetCurrentPoint()[0] + Photon.v[0]*RayleighPath;
            R[1] = Navigator->GetCurrentPoint()[1] + Photon.v[1]*RayleighPath;
            R[2] = Navigator->GetCurrentPoint()[2] + Photon.v[2]*RayleighPath;
            Navigator->SetCurrentPoint(R);
            //navigator->FindNode();  /// not needed - removed

            //new direction
            double v_old[3];
            v_old[0] = Photon.v[0]; v_old[1] = Photon.v[1]; v_old[2] = Photon.v[2];
            double dotProduct;
            do
            {
                Photon.generateRandomDir();
                dotProduct = Photon.v[0]*v_old[0] + Photon.v[1]*v_old[1] + Photon.v[2]*v_old[2];
            }
            while ( (dotProduct*dotProduct + 1.0) < 2.0*RandomHub.uniform());
            Navigator->SetCurrentDirection(Photon.v);

            Photon.time += RayleighPath / SpeedOfLight;
            SimStat.Rayleigh++;

            if (SimSet.RunSet.SaveTracks)    Track.Positions.push_back(AVector3(R));
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(R, NameFrom, Photon.time, Photon.waveIndex, APhotonHistoryLog::Rayleigh, MatIndexFrom) );

            return EBulkProcessResult::Scattered;
        }
    }

    //qDebug() << "Abs and Ray - not triggered";
    return EBulkProcessResult::NotTriggered;
}

#include <complex>
double APhotonTracer::calculateReflectionProbability()
{
    if (!bHaveNormal)
    {
        N = Navigator->FindNormal(false);
        bHaveNormal = true;
    }

    double NK = 0;
    if (bUseLocalNormal) for (int i = 0; i < 3; i++) NK += Photon.v[i] * InterfaceRule->LocalNormal[i];
    else                 for (int i = 0; i < 3; i++) NK += Photon.v[i] * N[i];
    const double cos1 = fabs(NK); // cos of the angle of incidence
    //qDebug() << "Cos of incidence:"<<cos1;
    const double sin1 = (cos1 < 0.9999999) ? sqrt(1.0 - cos1*cos1) : 0;
    //qDebug() << "sin1" << sin1;

    if (MaterialTo->Dielectric)
    {
        const double RefrIndexFrom = MaterialFrom->getRefractiveIndex(Photon.waveIndex); //qDebug() << "Refractive index from:" << RefrIndexFrom;
        const double RefrIndexTo   = MaterialTo->getRefractiveIndex(Photon.waveIndex);

        //qDebug()<<"Dir vector length is:"<<sqrt(p.v[0]*p.v[0] + p.v[1]*p.v[1] + p.v[2]*p.v[2]);
        //qDebug()<<"Photon wavelength"<<p->wavelength<<"WaveIndex:"<<p->WaveIndex<<"n1 and n2 are: "<<RefrIndexFrom<<RefrIndexTo;

        const double sin2 = RefrIndexFrom / RefrIndexTo * sin1;
        //qDebug()<<"cos1 sin1 sin2 are:"<<cos1<<sin1<<sin2;
        if (fabs(sin2) > 1.0)
        {
            // qDebug()<<"Total internal reflection, RefCoeff = 1.0";
            return 1.0;
        }
        else
        {
            const double cos2 = sqrt(1.0 - sin2*sin2);
            double Rs = (RefrIndexFrom*cos1 - RefrIndexTo*cos2) / (RefrIndexFrom*cos1 + RefrIndexTo*cos2);
            Rs *= Rs;
            double Rp = (RefrIndexFrom*cos2 - RefrIndexTo*cos1) / (RefrIndexFrom*cos2 + RefrIndexTo*cos1);
            Rp *= Rp;
            return 0.5 * (Rs + Rp);
        }
    }
    else
    {
        //using namespace std::complex_literals;
        const double nFrom = MaterialFrom->getRefractiveIndex(Photon.waveIndex);
        const std::complex<double> & NTo = MaterialTo->getComplexRefractiveIndex(Photon.waveIndex);
        //qDebug() << "cosTheta:"<< cos1 << "  from:" << nFrom << "  to:" << NTo.real() << NTo.imag();

        const std::complex<double> sin2 = sin1 / NTo * nFrom;
        //qDebug() << "sin2" << sin2.real() << sin2.imag();
        const std::complex<double> cos2 = sqrt( 1.0 - sin2*sin2 );
        //qDebug() << "cos2" << cos2.real() << cos2.imag();

        const std::complex<double> rs = (nFrom*cos1 -   NTo*cos2) / (nFrom*cos1 +   NTo*cos2);
        const std::complex<double> rp = ( -NTo*cos1 + nFrom*cos2) / (  NTo*cos1 + nFrom*cos2);

        const double RS = std::norm(rs);
        const double RP = std::norm(rp);
        //qDebug() << "rs" << rs.real() << rs.imag() << RS;

        const double R = 0.5 * (RS + RP);
        //qDebug() << "Refl coeff = "<< R;

        return R;
    }
}

void APhotonTracer::processSensorHit(int iSensor)
{
    const ASensorModel * model = SensorHub.sensorModel(iSensor);
    if (!model)
    {
        // !!!*** report error!
        qWarning() << "Unknown sensor index or non existent sensor model" << iSensor;
        exit(222);
    }

    double local[3];//if no area dep or not SiPM - local content is undefined!
    if (model->isAreaSensitive() || model->SiPM || (SimSet.RunSet.SaveSensorLog && SimSet.RunSet.SensorLogXY) )
    {
        const double * global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, local);
        //qDebug()<<local[0]<<local[1];
    }

    //since we check vs cos of _refracted_:
    if (bDoFresnel) performRefraction(); // true - successful  // !!!*** now can use local normal! - check
    if (!bHaveNormal) N = Navigator->FindNormal(false);
    //       qDebug()<<N[0]<<N[1]<<N[2]<<"Normal length is:"<<sqrt(N[0]*N[0]+N[1]*N[1]+N[2]*N[2]);
    //       qDebug()<<K[0]<<K[1]<<K[2]<<"Dir vector length is:"<<sqrt(K[0]*K[0]+K[1]*K[1]+K[2]*K[2]);
    double cosAngle = 0;
    for (int i=0; i<3; i++) cosAngle += N[i] * Photon.v[i];
    //       qDebug()<<"cos() = "<<cosAngle;
    double angle = 0;
    if ( !model->AngularBinned.empty() ||
         SimSet.RunSet.SaveStatistics ||
         (SimSet.RunSet.SaveSensorLog && SimSet.RunSet.SensorLogAngle) )
    {
        // !!!*** TODO for metals!
        angle = TMath::ACos(cosAngle)*180.0/3.1415926535;
    }

    if (!SimSet.OptSet.CheckQeBeforeTracking) Rnd = RandomHub.uniform(); //else already calculated
    const bool bDetected = Event.checkSensorHit(iSensor, Photon.time, Photon.waveIndex, local[0], local[1], angle, TransitionCounter, Rnd);

    if (bDetected && SimSet.RunSet.SaveSensorLog)
        appendToSensorLog(iSensor, Photon.time, local[0], local[1], angle, Photon.waveIndex);

    if (SimSet.RunSet.SavePhotonLog)
        PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), Navigator->GetCurrentVolume()->GetName(), Photon.time, Photon.waveIndex, (bDetected ? APhotonHistoryLog::Detected : APhotonHistoryLog::NotDetected), -1, -1, iSensor) );
}

void APhotonTracer::appendToSensorLog(int ipm, double time, double x, double y, double angle, int waveIndex)
{
    *StreamSensorLog << ipm;
    if (SimSet.RunSet.SensorLogTime)  *StreamSensorLog << ' ' << time;
    if (SimSet.RunSet.SensorLogXY)    *StreamSensorLog << ' ' << x << ' ' << y;
    if (SimSet.RunSet.SensorLogAngle) *StreamSensorLog << ' ' << angle;
    if (SimSet.RunSet.SensorLogWave)  *StreamSensorLog << ' ' << waveIndex;
    *StreamSensorLog << '\n';
}

bool APhotonTracer::performRefraction()
{
    if (MaterialTo->Dielectric)
    {
        const double RefrIndexFrom = MaterialFrom->getRefractiveIndex(Photon.waveIndex); //qDebug() << "Refractive index from:" << RefrIndexFrom;
        const double RefrIndexTo   = MaterialTo->getRefractiveIndex(Photon.waveIndex);

        const double nn = RefrIndexFrom / RefrIndexTo;
        //qDebug()<<"refraction triggered, n1/n2 ="<<nn;
        //N - normal vector, K - origial photon direction vector
        // nn = n(in)/n(tr)
        // (NK) - scalar product
        // T = -( nn*(NK) - sqrt(1-nn*nn*[1-(NK)*(NK)]))*N + nn*K

        if (!bHaveNormal) N = Navigator->FindNormal(false);
        //const double NK = N[0]*Photon.v[0] + N[1]*Photon.v[1] + N[2]*Photon.v[2];
        double NK = 0;
        if (bUseLocalNormal) for (int i = 0; i < 3; i++) NK += Photon.v[i] * InterfaceRule->LocalNormal[i];
        else                 for (int i = 0; i < 3; i++) NK += Photon.v[i] * N[i];

        const double UnderRoot = 1.0 - nn*nn*(1.0 - NK*NK);
        if (UnderRoot < 0)
        {
            //        qDebug()<<"total internal detected - will assume this photon is abosrbed at the surface";
            return false;    //total internal reflection! //previous reflection test should catch this situation
        }
        const double tmp = nn * NK - sqrt(UnderRoot);

        //Photon.v[0] = -tmp*N[0] + nn*Photon.v[0]; Photon.v[1] = -tmp*N[1] + nn*Photon.v[1]; Photon.v[2] = -tmp*N[2] + nn*Photon.v[2];
        if (bUseLocalNormal) for (int i = 0; i < 3; i++) Photon.v[i] = nn * Photon.v[i] - tmp * InterfaceRule->LocalNormal[i];
        else                 for (int i = 0; i < 3; i++) Photon.v[i] = nn * Photon.v[i] - tmp * N[i];

        Navigator->SetCurrentDirection(Photon.v);
    }
    // for metals do nothing -> anyway geometric optics is not a proper model to use

    return true;
}

void APhotonTracer::performReflection()
{
    if (!bHaveNormal)
    {
        N = Navigator->FindNormal(false);
        bHaveNormal = true;
    }
    //qDebug() << "Normal:"<<N[0]<<N[1]<<N[2];
    //qDebug() << "Vector before:"<<p.v[0]<<p.v[1]<<p.v[2];

    //rotating the vector
    //K = K - 2*(NK)*N    where K is the photon direction vector
    if (bUseLocalNormal)
    {
        double NK = 0;
        for (int i = 0; i < 3; i++) NK += InterfaceRule->LocalNormal[i] * Photon.v[i];
        for (int i = 0; i < 3; i++) Photon.v[i] -= 2.0 * NK * InterfaceRule->LocalNormal[i];
    }
    else
    {
        const double NK = N[0]*Photon.v[0] + N[1]*Photon.v[1] + N[2]*Photon.v[2];
        Photon.v[0] -= 2.0 * NK * N[0];
        Photon.v[1] -= 2.0 * NK * N[1];
        Photon.v[2] -= 2.0 * NK * N[2];
    }

    //qDebug() << "Vector after:"<<p.v[0]<<p.v[1]<<p.v[2];
    //qDebug() << "Photon position:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];

    Navigator->SetCurrentDirection(Photon.v);
}

bool APhotonTracer::enterGrid(int GridNumber)
{
    //calculating where we are in respect to the grid elementary cell
    const double * global = Navigator->GetCurrentPoint();
    for (int i = 0; i < 3; i++) FromGridToGlobal[i] = global[i]; // init only, later adjusted

    if (GridNumber < 0 || GridNumber >= (int)GridHub.GridRecords.size())
    {
        qCritical() << "Grid hit: Bad grid number!";
        return false;
    }
    //qDebug() << "------------grid record number"<<GridNumber << "Shape, dx, dy:"<<(*grids)[GridNumber]->shape<<(*grids)[GridNumber]->dx<<(*grids)[GridNumber]->dy;
    //qDebug() << "-->Global coordinates of the entrance to the grid bulk:"<<global[0]<<global[1]<<global[2];

    double local[3]; // to local coordinates in the grid
    Navigator->MasterToLocal(global,local);
    //qDebug() << "-->Local in grid bulk:" <<local[0]<<local[1]<<local[2];
    for (int i = 0; i < 3; i++) FromGridElementToGridBulk[i] = local[i];

    if (GridHub.GridRecords[GridNumber]->shape < 2)
    {
        //rectangular grid
        double & dx = GridHub.GridRecords[GridNumber]->dx;
        double & dy = GridHub.GridRecords[GridNumber]->dy;
        //qDebug() << "-->grid cell half size in x and y"<<dx<<dy;

        const int periodX = 0.5*fabs(local[0])/dx + 0.5;
        //qDebug() << "-->periodX"<<periodX;
        if (local[0]>0) local[0] -= periodX*2.0*dx;
        else local[0] += periodX*2.0*dx;
        //qDebug() << "-->local x"<<local[0];

        const int periodY = 0.5*fabs(local[1])/dy + 0.5;
        //qDebug() << "-->periodY"<<periodX;
        if (local[1]>0) local[1] -= periodY*2.0*dy;
        else local[1] += periodY*2.0*dy;
        //qDebug() << "-->local y"<<local[0];
    }
    else
    {
        //hexagonal grid - the size is given in radius of circle inside!
        double dx = GridHub.GridRecords[GridNumber]->dx * 1.1547; // 1.0/cos(30)
        double dy = dx*0.866; //sqrt(3)*2
        //        qDebug() << "-->dx,dy:"<<dx<<dy;
        const int ix = fabs(local[0]/dx/1.5);
        const int iy = fabs(local[1]/dy);
        bool fXisEven = (ix % 2 == 0);
        bool fYisEven = (iy % 2 == 0);
        //        qDebug() << "-->ix, iy:" << ix << iy;
        const double x = fabs(local[0]) - ix*1.5*dx;
        const double y = fabs(local[1]) - iy*dy;
        //        qDebug() << "-->x,y"<<x<<y<<"odd/even for ix and iy"<<fXisEven<<fYisEven;
        double CenterX, CenterY;
        if ( (fXisEven && fYisEven) || (!fXisEven && !fYisEven) )
        {
            CenterX = ix*1.5*dx;
            CenterY = iy*dy;
            //            qDebug() << "-->aaaaaa"<<y<<1.7320508*(dx-x);
            if (y > 1.7320508*(dx-x) ) //sqrt(3)
            {
                CenterX += 1.5*dx;
                CenterY += dy;
            }
        }
        else
        {
            CenterX = ix*1.5*dx;
            CenterY = (iy+1)*dy;
            if (y < 1.7320508*(x-0.5*dx) ) //sqrt(3)
            {
                CenterX += 1.5*dx;
                CenterY -= dy;
            }
        }
        // for negative coordinates - just inversion
        if (local[0]<0) CenterX = -CenterX;
        if (local[1]<0) CenterY = -CenterY;
        //        qDebug() << "-->Closest center:"<<CenterX << CenterY;
        // to coordinates inside an elementary block of the grid
        local[0] -= CenterX;
        local[1] -= CenterY;
    }

    //  qDebug() << "-->new local inside grid elementary block:" << local[0] << local[1] << local[2];
    for (int i = 0; i < 3; i++) FromGridElementToGridBulk[i] -= local[i];

    double master[3];
    Navigator->LocalToMaster(local, master);
    //  qDebug() << "-->global for the elementary block:"<<master[0]<<master[1]<<master[2];
    Navigator->SetCurrentPoint(master);
    Navigator->FindNode();

    for (int i = 0; i < 3; i++) FromGridToGlobal[i] -= master[i];

    bGridShiftOn = true;

    //qDebug() << "-->Grid shift performed!";
    //qDebug() << "-->  Current node:"<<navigator->GetCurrentNode()->GetName();
    //qDebug() << "-->  Current point:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "-->  Shifts in XYZ to get to normal:"<<FromGridCorrection[0]<<FromGridCorrection[1]<<FromGridCorrection[2];
    return true;
}
