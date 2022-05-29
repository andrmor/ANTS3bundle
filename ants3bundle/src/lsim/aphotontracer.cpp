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

#include <QDebug>
#include <QTextStream>

#include "TGeoManager.h"
#include "TMath.h"
#include "TH1D.h"

APhotonTracer::APhotonTracer(AOneEvent & event, QTextStream* & streamTracks) :
    MatHub(AMaterialHub::getConstInstance()),
    RuleHub(AInterfaceRuleHub::getConstInstance()),
    SensorHub(ASensorHub::getConstInstance()),
    GridHub(AGridHub::getConstInstance()),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    SimStat(AStatisticsHub::getInstance().SimStat),
    Event(event),
    StreamTracks(streamTracks)
{}

void APhotonTracer::configureTracer()
{
    Track.Positions.reserve(SimSet.OptSet.MaxPhotonTransitions + 1);
    AddedTracks = 0;
}

void APhotonTracer::initTracks()
{
    if (AddedTracks < SimSet.RunSet.MaxTracks)  // !!!*** move to the caller?
    {
        Track.HitSensor = false;
        Track.SecondaryScint = p.SecondaryScint;
        Track.Positions.clear();
        Track.Positions.push_back(AVector3(p.r));
    }
    else APhotonSimHub::getInstance().Settings.RunSet.SaveTracks = false; // SimSet is read-only
}

void APhotonTracer::initPhotonLog()
{
    PhLog.clear();
    PhLog.reserve(SimSet.OptSet.MaxPhotonTransitions);
    PhLog.push_back( APhotonHistoryLog(p.r, Navigator->GetCurrentVolume()->GetName(), p.time, p.waveIndex, APhotonHistoryLog::Created, MatIndexFrom) );
}

bool APhotonTracer::skipTracing(int waveIndex)
{
    rnd = RandomHub.uniform();
    if (rnd > SensorHub.getMaxQE(SimSet.WaveSet.Enabled, waveIndex))
    {
        SimStat.TracingSkipped++;
        return true;
    }
    return false;
}

bool APhotonTracer::initBeforeTracing(const APhoton & Photon)
{
    GeoManager = AGeometryHub::getInstance().GeoManager;
    Navigator = GeoManager->GetCurrentNavigator();
    Navigator->SetCurrentPoint(Photon.r);
    Navigator->SetCurrentDirection(Photon.v);
    Navigator->FindNode();
    if (Navigator->IsOutside())
    {
        SimStat.GeneratedOutside++;
        if (SimSet.RunSet.SavePhotonLog)
        {
            PhLog.clear();
            PhLog.push_back( APhotonHistoryLog(Photon.r, "", Photon.time, Photon.waveIndex, APhotonHistoryLog::GeneratedOutsideGeometry) );
        }
        return false;
    }

//    qDebug()<<"Photon starts from:";
//    qDebug()<<Navigator->GetPath();
//    qDebug()<<"material name: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetName();
//    qDebug()<<"material index: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetIndex();

    p.copyFrom(Photon);

    if (SimSet.RunSet.SaveTracks)    initTracks();
    if (SimSet.RunSet.SavePhotonLog) initPhotonLog();

    return true;
}

void APhotonTracer::tracePhoton(const APhoton & Photon)
{
//    qDebug() << "Photon tracing called";
    if (SimSet.OptSet.CheckQeBeforeTracking && skipTracing(Photon.waveIndex)) return;
    if (!initBeforeTracing(Photon)) return;

    VolumeTo = Navigator->GetCurrentVolume();                       // copied to VolumeFrom   on cycle start
    if (VolumeTo) MatIndexTo = VolumeTo->GetMaterial()->GetIndex(); // copied to MatIndexFrom on cycle start

    TransitionCounter = 0;
    fGridShiftOn = false;

    while (TransitionCounter < SimSet.OptSet.MaxPhotonTransitions)
    {
        TransitionCounter++; //qDebug() << "tracing cycle #" << TransitionCounter;

        VolumeFrom = Navigator->GetCurrentVolume();
        if (VolumeFrom) MatIndexFrom = VolumeFrom->GetMaterial()->GetIndex();
        MaterialFrom = MatHub[MatIndexFrom]; //this is the material where the photon is currently in
        if (SimSet.RunSet.SavePhotonLog) NameFrom = Navigator->GetCurrentVolume()->GetName();

        Navigator->FindNextBoundary();
        Step = Navigator->GetStep();  //qDebug()<<"Step:"<<Step;

        switch ( checkBulkProcesses() )
        {
            case EBulkProcessResult::Absorbed    : endTracing(); return; // finished with this photon
            case EBulkProcessResult::Scattered   : continue;             // next step
            case EBulkProcessResult::WaveShifted : continue;             // next step
            default:; // not triggered
        }

        //----- Making a step towards the interface ------
        Navigator->Step(true, false); //stopped right before the crossing

        double R_afterStep[3];
        if (fGridShiftOn && Step > 0.001) // !!!*** why 0.001?
        {   //if point after Step is outside of grid bulk, kill this photon
            // it is not realistic in respect of Rayleigh for photons travelling in grid plane
            //also, there is a small chance that photon is rejected by this procedure when it hits the grid wire straight on center
            //chance is very small (~0.1%) even for grids with ~25% transmission value

            const double * rc = Navigator->GetCurrentPoint();
            //qDebug() << "...Checking after step of " << Step << "still inside grid bulk?";
            //qDebug() << "...Position in world:"<<rc[0] << rc[1] << rc[2];
            //qDebug() << "...Navigator is supposed to be inside the grid element. Currently in:"<<navigator->GetCurrentNode()->GetVolume()->GetName();

            R_afterStep[0] = rc[0] + FromGridCorrection[0];
            R_afterStep[1] = rc[1] + FromGridCorrection[1];
            R_afterStep[2] = rc[2] + FromGridCorrection[2];
            //qDebug() << "...After correction, coordinates in true global:"<<rc[0] << rc[1] << rc[2];

            double GridLocal[3];
            Navigator->MasterToLocal(rc, GridLocal);
            //qDebug() << "...Position in grid element:"<<GridLocal[0] << GridLocal[1] << GridLocal[2];
            GridLocal[0] += FromGridElementToGridBulk[0];
            GridLocal[1] += FromGridElementToGridBulk[1];
            GridLocal[2] += FromGridElementToGridBulk[2];
            //qDebug() << "...Position in grid bulk:"<<GridLocal[0] << GridLocal[1] << GridLocal[2];
            if (!GridVolume->Contains(GridLocal))
            {
                //qDebug() << "...!!!...Photon would be outside the bulk if it takes the Step! -> killing the photon";
                //qDebug() << "Conflicting position:"<<R_afterStep[0] << R_afterStep[1] << R_afterStep[2];
                //if (fGridShiftOn) track->Nodes.append(TrackNodeStruct(R_afterStep, p.time)); //not drawing end of the track for such photons!
                SimStat.LossOnGrid++;
                endTracing();
                return;
            }
        }

        //placing this location/state to the stack - it will be restored if photon is reflected
        Navigator->PushPoint(); //DO NOT FORGET TO CLEAN IT IF NOT USED!!!

        //can make the track now - the photon made it to the other border in any case
        RefrIndexFrom = MaterialFrom->getRefractiveIndex(p.waveIndex);
        //qDebug() << "Refractive index from:" << RefrIndexFrom;
        p.time += Step / c_in_vac * RefrIndexFrom;
        if (SimSet.RunSet.SaveTracks && Step > 0.001) // !!!*** hard coded 0.001
        {
            if (fGridShiftOn)
            {
                //qDebug() << "Added to track using shifted gridnavigator data:"<<R[0]<<R[1]<<R[2];
                Track.Positions.push_back(AVector3(R_afterStep));
            }
            else
            {
                //qDebug() << "Added to track using normal navigator:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
                Track.Positions.push_back(AVector3(Navigator->GetCurrentPoint()));
            }
        }

        // ----- Now making a step inside the next material volume on the path -----
        NodeAfter = Navigator->FindNextBoundaryAndStep(); //this is the node after crossing the boundary
        //this method MOVES the current position! different from FindNextBoundary method, which only calculates the step
        //now current point is inside the next volume!

        //check if after the border the photon is outside the defined world
        if (Navigator->IsOutside()) //outside of geometry - end tracing
        {
            //qDebug() << "Photon escaped!";
            Navigator->PopDummy();//clean up the stack
            SimStat.Escaped++;
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Escaped) );
            endTracing();
            return;
        }

        //new volume info
        VolumeTo = NodeAfter->GetVolume();
        if (SimSet.RunSet.SavePhotonLog) nameTo = Navigator->GetCurrentVolume()->GetName();
        MatIndexTo = VolumeTo->GetMaterial()->GetIndex();
        MaterialTo = MatHub[MatIndexTo];
        fHaveNormal = false;

//        qDebug()  << "Found border with another volume: " << VolumeTo->GetName();
//        qDebug()  << "Mat index after interface: " << MatIndexTo << " Mat index before: " << MatIndexFrom;
//        qDebug()  << "Coordinates: "<<Navigator->GetCurrentPoint()[0]<<Navigator->GetCurrentPoint()[1]<<Navigator->GetCurrentPoint()[2];

        //--- Check interface rule ---
        const EInterRuleResult res = tryInterfaceRule();
        switch (res)
        {
            case EInterRuleResult::Absorbed     : endTracing(); return;      // stack cleaned inside
            case EInterRuleResult::Reflected    : continue;                  // stack cleaned inside
            case EInterRuleResult::Transmitted  : fDoFresnel = false; break;
            case EInterRuleResult::NotTriggered : fDoFresnel = true;  break;
        }

        //--- Interface rule not set or not triggered ---
        if (fDoFresnel)
        {
            RefrIndexTo = MaterialTo->getRefractiveIndex(p.waveIndex);
            const EFresnelResult res = tryReflection();
            if (res == EFresnelResult::Reflected) continue;
        }

        //--- Photon entered another volume ---
        Navigator->PopDummy(); //clean up the stack

        // photon is considered to exit the grid
        if (fGridShiftOn && Step > 0.001)
        {
            //qDebug() << "++Grid back shift triggered!";
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Grid_ShiftOut) );
            returnFromGridShift();
            Navigator->FindNode();
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Grid_Exit) );
            //qDebug() << "Navigator coordinates: "<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
        }

        bool endTracingFlag;
        checkSpecialVolume(NodeAfter, endTracingFlag);
        if (endTracingFlag)
        {
            endTracing();
            return;
        }

        if (fDoFresnel)
        {
            const bool ok = performRefraction( RefrIndexFrom/RefrIndexTo );
            // true - successful, false - forbidden -> considered that the photon is absorbed at the surface! Should not happen
            if (!ok) qWarning()<<"Error in photon tracker: problem with transmission!";
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
        }

    } //if below max number of transitions, process next (or reflect back to stay in the same) volume

    if (TransitionCounter == SimSet.OptSet.MaxPhotonTransitions) SimStat.MaxTransitions++;  // maximum number of transitions reached
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
    AInterfaceRule * rule = getInterfaceRule();
    if (!rule) return EInterRuleResult::NotTriggered;

    //qDebug() << "Interface rule defined! Model = "<<ov->getType();
    N = Navigator->FindNormal(false); fHaveNormal = true;
    const double * PhPos = Navigator->GetCurrentPoint();
    for (int i=0; i<3; i++) p.r[i] = PhPos[i];

    AInterfaceRule::OpticalOverrideResultEnum result = rule->calculate(&p, N);

    switch (result)
    {
    case AInterfaceRule::Absorbed:
        //qDebug() << "-Override: absorption triggered";
        Navigator->PopDummy(); //clean up the stack
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Override_Loss, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleLoss++;
        return EInterRuleResult::Absorbed;
    case AInterfaceRule::Back:
        //qDebug() << "-Override: photon bounced back";
        Navigator->PopPoint();  //remaining in the original volume
        Navigator->SetCurrentDirection(p.v); //updating direction
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Override_Back, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleBack++;
        return EInterRuleResult::Reflected;
    case AInterfaceRule::Forward:
        Navigator->SetCurrentDirection(p.v);
        //stack cleaned afterwards
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(PhPos, nameTo, p.time, p.waveIndex, APhotonHistoryLog::Override_Forward, MatIndexFrom, MatIndexTo) );
        SimStat.InterfaceRuleForward++;
        return EInterRuleResult::Transmitted;
    case AInterfaceRule::NotTriggered:
        //stack cleaned afterwards
        return EInterRuleResult::NotTriggered;
    default:
        //stack cleaned afterwards
        qCritical() << "override error - doing fresnel instead!";
        return EInterRuleResult::NotTriggered;
    }
}

EFresnelResult APhotonTracer::tryReflection()
{
    //qDebug()<<"Performing fresnel"; //Fresnel equations //http://en.wikipedia.org/wiki/Fresnel_equations
    if (!fHaveNormal)
    {
        N = Navigator->FindNormal(false);
        fHaveNormal = true;
    }
    //qDebug()<<"Normal length is:"<<sqrt(N[0]*N[0] + N[1]*N[1] + N[2]*N[2]);
    //qDebug()<<"Dir vector length is:"<<sqrt(p.v[0]*p.v[0] + p.v[1]*p.v[1] + p.v[2]*p.v[2]);

    const double prob = calculateReflectionProbability();
    if (RandomHub.uniform() < prob)
    {
        SimStat.FresnelReflected++;
        // photon remains in the same volume -> continue to the next iteration
        Navigator->PopPoint(); //restore the point before the border
        performReflection();
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Fresnel_Reflection, MatIndexFrom, MatIndexTo) );
        return EFresnelResult::Reflected;
    }

    SimStat.FresnelTransmitted++;
    return EFresnelResult::Transmitted;
}

void APhotonTracer::checkSpecialVolume(TGeoNode * NodeAfterInterface, bool & returnEndTracingFlag)
{
    //const char* VolName = ThisVolume->GetName();
    //qDebug()<<"Photon entered new volume:" << VolName;
    const char* VolTitle = VolumeTo->GetTitle();
    //qDebug() << "Title:"<<VolTitle;

    const char Selector = VolTitle[0];
    if (Selector == 'S') // Sensor
    {
        const int iSensor = NodeAfterInterface->GetNumber();
        //qDebug()<< "Sensor hit! (" << ThisVolume->GetTitle() <<") Sensor name:"<< ThisVolume->GetName() << "Sensor index" << iSensor;
        if (SimSet.RunSet.SavePhotonLog)
        {
            PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
            PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::HitSensor, -1, -1, iSensor) );
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
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Grid_Enter) );
        GridWasHit(NodeAfterInterface->GetNumber()); // it is assumed that "empty part" of the grid element will have the same refractive index as the material from which photon enters it
        GridVolume = VolumeTo;
        if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::Grid_ShiftIn) );
        returnEndTracingFlag = false;
        return;
    }

    if (Selector == 'M') // Monitor
    {
        AMonitorHub & MonitorHub = AMonitorHub::getInstance();
        const int iMon = NodeAfterInterface->GetNumber();
        //qDebug() << "Monitor hit!" << ThisVolume->GetName() << "Number:"<<iMon;// << MatIndexFrom<<MatIndexTo;
        Double_t local[3];
        const Double_t * global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, local);
        //qDebug()<<local[0]<<local[1];
        //qDebug() << "Monitors:"<<SimStat.Monitors.size();
        if ( (local[2] > 0 && MonitorHub.PhotonMonitors[iMon].Monitor->isUpperSensitive()) ||
             (local[2] < 0 && MonitorHub.PhotonMonitors[iMon].Monitor->isLowerSensitive()) )
        {
            //angle?
            if (!fHaveNormal) N = Navigator->FindNormal(false);
            double cosAngle = 0;
            for (int i=0; i<3; i++) cosAngle += N[i] * p.v[i];
            MonitorHub.PhotonMonitors[iMon].Monitor->fillForPhoton(local[0], local[1], p.time, 180.0/3.1415926535*TMath::ACos(cosAngle), p.waveIndex);
            if (MonitorHub.PhotonMonitors[iMon].Monitor->isStopsTracking())
            {
                SimStat.MonitorKill++;
                if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p.time, p.waveIndex, APhotonHistoryLog::KilledByMonitor) );
                returnEndTracingFlag = true;
                return;
            }
        }
        returnEndTracingFlag = false;
        return;
    }

    // volumes without special role have titles statring with '-'
    returnEndTracingFlag = false;
}

AInterfaceRule * APhotonTracer::getInterfaceRule() const
{
    AInterfaceRule * rule = nullptr;
    if (VolumeFrom->GetTitle()[1] == '*' && VolumeTo->GetTitle()[2] == '*')
    {
        rule = RuleHub.getVolumeRule(VolumeFrom->GetName(), VolumeTo->GetName());
        rule->updateMatIndices(MatIndexFrom, MatIndexTo);
    }
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
    const double AbsCoeff = MatHub[MatIndexFrom]->getAbsorptionCoefficient(p.waveIndex);
    if (AbsCoeff > 0)
    {
        AbsPath = -log(RandomHub.uniform())/AbsCoeff;
        if (AbsPath < Step) DoAbsorption = true;
        else DoAbsorption = false;
    }
    else DoAbsorption = false;

    //prepare Rayleigh
    bool DoRayleigh;
    double RayleighPath;
    if (MaterialFrom->rayleighMFP == 0) DoRayleigh = false;      // == 0 - means undefined
    else
    {
        double RayleighMFP;
        if (p.waveIndex == -1) RayleighMFP = MaterialFrom->rayleighMFP;
        else RayleighMFP = MaterialFrom->rayleighBinned[p.waveIndex];

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
            if (AbsPath<RayleighPath) DoRayleigh = false;
            else DoAbsorption = false;
        }

        if (DoAbsorption)
        {
            //qDebug()<<"Absorption was triggered!";
            double refIndex = MaterialFrom->getRefractiveIndex(p.waveIndex);
            p.time += AbsPath/c_in_vac*refIndex;
            SimStat.Absorbed++;
            SimStat.BulkAbsorption++;

            if (SimSet.RunSet.SaveTracks || SimSet.RunSet.SavePhotonLog)
            {
                Double_t point[3];
                point[0] = Navigator->GetCurrentPoint()[0] + p.v[0]*AbsPath;
                point[1] = Navigator->GetCurrentPoint()[1] + p.v[1]*AbsPath;
                point[2] = Navigator->GetCurrentPoint()[2] + p.v[2]*AbsPath;
                if (SimSet.RunSet.SaveTracks)
                {
                    //track->Nodes.append(TrackNodeStruct(point, p.time));
                    Track.Positions.push_back(AVector3(point));
                }
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.push_back( APhotonHistoryLog(point, NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Absorbed, MatIndexFrom) );
            }

            //check if this material is waveshifter
            const double reemissionProb = MatHub[MatIndexFrom]->getReemissionProbability(p.waveIndex);
            if ( reemissionProb > 0 )
            {
                if (RandomHub.uniform() < reemissionProb)
                {
                    //qDebug() << "Waveshifting! Original index:"<<p.waveIndex;
                    if (p.waveIndex!=-1 && MatHub[MatIndexFrom]->PrimarySpectrumHist)
                    {
                        double wavelength;
                        int waveIndex;
                        int attempts = -1;
                        do
                        {
                            attempts++;
                            if (attempts > 9) return EBulkProcessResult::Absorbed;  // ***!!! absolute number
                            wavelength = MatHub[MatIndexFrom]->PrimarySpectrumHist->GetRandom();
                            //qDebug() << "   "<<wavelength << " MatIndexFrom:"<< MatIndexFrom;
                            waveIndex = SimSet.WaveSet.toIndexFast(wavelength); // !!!*** before was round here:
                            //waveIndex = round( (wavelength - SimSet->WaveFrom)/SimSet->WaveStep );
                        }
                        while (waveIndex < p.waveIndex); //conserving energy

                        //qDebug() << "NewIndex:"<<waveIndex;
                        p.waveIndex = waveIndex;
                    }
                    else p.waveIndex = -1; // this is to allow to use this procedure to make diffuse medium (including -1 waveIndex)

                    double R[3];
                    R[0] = Navigator->GetCurrentPoint()[0] + p.v[0]*AbsPath;
                    R[1] = Navigator->GetCurrentPoint()[1] + p.v[1]*AbsPath;
                    R[2] = Navigator->GetCurrentPoint()[2] + p.v[2]*AbsPath;
                    Navigator->SetCurrentPoint(R);

                    RandomDir();
                    Navigator->SetCurrentDirection(p.v);
                    //qDebug() << "After:"<<p->WaveIndex;

                    //if (SimSet->fTimeResolved)
                    //    p.time += RandGen->Exp(  MaterialFrom->PriScintDecayTime );
                    p.time += MatHub[MatIndexFrom]->generatePrimScintTime(RandomHub);

                    SimStat.Reemission++;
                    if (SimSet.RunSet.SavePhotonLog)
                        PhLog.push_back( APhotonHistoryLog(R, NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Reemission, MatIndexFrom) );
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
            R[0] = Navigator->GetCurrentPoint()[0] + p.v[0]*RayleighPath;
            R[1] = Navigator->GetCurrentPoint()[1] + p.v[1]*RayleighPath;
            R[2] = Navigator->GetCurrentPoint()[2] + p.v[2]*RayleighPath;
            Navigator->SetCurrentPoint(R);
            //navigator->FindNode();  /// not needed - removed

            //new direction
            double v_old[3];
            v_old[0] = p.v[0]; v_old[1] = p.v[1]; v_old[2] = p.v[2];
            double dotProduct;
            do
            {
                RandomDir();
                dotProduct = p.v[0]*v_old[0] + p.v[1]*v_old[1] + p.v[2]*v_old[2];
            }
            while ( (dotProduct*dotProduct + 1.0) < 2.0*RandomHub.uniform());
            Navigator->SetCurrentDirection(p.v);

            double refIndex = MaterialFrom->getRefractiveIndex(p.waveIndex);
            p.time += RayleighPath/c_in_vac*refIndex;
            SimStat.Rayleigh++;

            if (SimSet.RunSet.SaveTracks)    Track.Positions.push_back(AVector3(R));
            if (SimSet.RunSet.SavePhotonLog) PhLog.push_back( APhotonHistoryLog(R, NameFrom, p.time, p.waveIndex, APhotonHistoryLog::Rayleigh, MatIndexFrom) );

            return EBulkProcessResult::Scattered;
        }
    }

    //qDebug() << "Abs and Ray - not triggered";
    return EBulkProcessResult::NotTriggered;
}

double APhotonTracer::calculateReflectionProbability()
{
    const double NK = N[0]*p.v[0] + N[1]*p.v[1] + N[2]*p.v[2]; // NK = cos of the angle of incidence = cos1
    const double cos1 = fabs(NK);
    //qDebug() << "Cos of incidence:"<<cos1;

    //qDebug()<<"Photon wavelength"<<p->wavelength<<"WaveIndex:"<<p->WaveIndex<<"n1 and n2 are: "<<RefrIndexFrom<<RefrIndexTo;
    const double sin1 = sqrt(1.0 - NK*NK);
    const double sin2 = RefrIndexFrom/RefrIndexTo*sin1;

    //         qDebug()<<"cos1 sin1 sin2 are:"<<cos1<<sin1<<sin2;
    if (fabs(sin2)>1.0)
    {
        // qDebug()<<"Total internal reflection, RefCoeff = 1.0";
        return 1.0;
    }
    else
    {
        double cos2 = sqrt(1-sin2*sin2);
        double Rs = (RefrIndexFrom*cos1-RefrIndexTo*cos2) / (RefrIndexFrom*cos1+RefrIndexTo*cos2);
        Rs *= Rs;
        double Rp = (RefrIndexFrom*cos2-RefrIndexTo*cos1) / (RefrIndexFrom*cos2+RefrIndexTo*cos1);
        Rp *= Rp;
        return 0.5*(Rs + Rp);
    }
}

void APhotonTracer::processSensorHit(int iSensor)
{
    const ASensorModel * model = SensorHub.sensorModel(iSensor);
    if (!model)
    {
        // !!!*** report error!
        qWarning() << "Unknown server index or non existent sensor model" << iSensor;
        exit(222);
    }

    double local[3];//if no area dep or not SiPM - local[0] and [1] are undefined!
    if (model->isXYSensitive() || model->SiPM)
    {
        const double * global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, local);
        //qDebug()<<local[0]<<local[1];
    }

    //since we check vs cos of _refracted_:
    if (fDoFresnel) performRefraction(RefrIndexFrom / RefrIndexTo); // true - successful
    if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
    //       qDebug()<<N[0]<<N[1]<<N[2]<<"Normal length is:"<<sqrt(N[0]*N[0]+N[1]*N[1]+N[2]*N[2]);
    //       qDebug()<<K[0]<<K[1]<<K[2]<<"Dir vector length is:"<<sqrt(K[0]*K[0]+K[1]*K[1]+K[2]*K[2]);
    double cosAngle = 0;
    for (int i=0; i<3; i++) cosAngle += N[i] * p.v[i];
    //       qDebug()<<"cos() = "<<cosAngle;

    if (!SimSet.OptSet.CheckQeBeforeTracking) rnd = RandomHub.uniform(); //else already calculated

    const bool bDetected = Event.checkSensorHit(iSensor, p.time, p.waveIndex, local[0], local[1], cosAngle, TransitionCounter, rnd);

    if (SimSet.RunSet.SavePhotonLog)
        PhLog.push_back( APhotonHistoryLog(Navigator->GetCurrentPoint(), Navigator->GetCurrentVolume()->GetName(), p.time, p.waveIndex, (bDetected ? APhotonHistoryLog::Detected : APhotonHistoryLog::NotDetected), -1, -1, iSensor) );
}

bool APhotonTracer::performRefraction(double nn) // nn = nFrom / nTo
{
    //qDebug()<<"refraction triggered, n1/n2 ="<<nn;
    //N - normal vector, K - origial photon direction vector
    // nn = n(in)/n(tr)
    // (NK) - scalar product
    // T = -( nn*(NK) - sqrt(1-nn*nn*[1-(NK)*(NK)]))*N + nn*K

    if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
    const Double_t NK = N[0]*p.v[0] + N[1]*p.v[1] + N[2]*p.v[2];

    const Double_t UnderRoot = 1.0 - nn*nn*(1.0 - NK*NK);
    if (UnderRoot<0)
    {
        //        qDebug()<<"total internal detected - will assume this photon is abosrbed at the surface";
        return false;    //total internal reflection! //previous reflection test should catch this situation
    }
    const Double_t tmp = nn*NK - sqrt(UnderRoot);

    p.v[0] = -tmp*N[0] + nn*p.v[0];
    p.v[1] = -tmp*N[1] + nn*p.v[1];
    p.v[2] = -tmp*N[2] + nn*p.v[2];

    Navigator->SetCurrentDirection(p.v);
    return true;
}

void APhotonTracer::performReflection()
{
    if (!fHaveNormal)
    {
        N = Navigator->FindNormal(kFALSE);
        fHaveNormal = true;
    }
    //qDebug() << "Normal:"<<N[0]<<N[1]<<N[2];
    //qDebug() << "Vector before:"<<p.v[0]<<p.v[1]<<p.v[2];

    //rotating the vector
    //K = K - 2*(NK)*N    where K is the photon direction vector
    const double NK = N[0]*p.v[0] + N[1]*p.v[1] + N[2]*p.v[2];
    p.v[0] -= 2.0 * NK * N[0];
    p.v[1] -= 2.0 * NK * N[1];
    p.v[2] -= 2.0 * NK * N[2];

    //qDebug() << "Vector after:"<<p.v[0]<<p.v[1]<<p.v[2];
    //qDebug() << "Photon position:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];

    Navigator->SetCurrentDirection(p.v);
}

void APhotonTracer::RandomDir()
{
    //Sphere function of Root:
    double a=0, b=0, r2=1;
    while (r2 > 0.25)
    {
        a  = RandomHub.uniform() - 0.5;
        b  = RandomHub.uniform() - 0.5;
        r2 =  a*a + b*b;
    }
    p.v[2] = ( -1.0 + 8.0 * r2 );
    double scale = 8.0 * TMath::Sqrt(0.25 - r2);
    p.v[0] = a*scale;
    p.v[1] = b*scale;
}

bool APhotonTracer::GridWasHit(int GridNumber)
{
    //calculating where we are in respect to the grid elementary cell
    const Double_t *global = Navigator->GetCurrentPoint();
    //init only:
    FromGridCorrection[0] = global[0];
    FromGridCorrection[1] = global[1];
    FromGridCorrection[2] = global[2];

    if (GridNumber < 0 || GridNumber > GridHub.GridRecords.size()-1)
    {
        qCritical() << "Grid hit: Bad grid number!";
        return false;
    }
    //qDebug() << "------------grid record number"<<GridNumber << "Shape, dx, dy:"<<(*grids)[GridNumber]->shape<<(*grids)[GridNumber]->dx<<(*grids)[GridNumber]->dy;

    //qDebug() << "-->Global coordinates where entered the grid bulk:"<<global[0]<<global[1]<<global[2];

    double local[3]; // to local coordinates in the grid
    Navigator->MasterToLocal(global,local);
    //qDebug() << "-->Local in grid bulk:" <<local[0]<<local[1]<<local[2];
    FromGridElementToGridBulk[0] = local[0];
    FromGridElementToGridBulk[1] = local[1];
    FromGridElementToGridBulk[2] = local[2];

    if (GridHub.GridRecords[GridNumber]->shape < 2)
    {
        //rectangular grid
        double & dx = GridHub.GridRecords[GridNumber]->dx;
        double & dy = GridHub.GridRecords[GridNumber]->dy;
        //qDebug() << "-->grid cell half size in x and y"<<dx<<dy;

        int periodX = 0.5*fabs(local[0])/dx + 0.5;
        //qDebug() << "-->periodX"<<periodX;
        if (local[0]>0) local[0] -= periodX*2.0*dx;
        else local[0] += periodX*2.0*dx;
        //qDebug() << "-->local x"<<local[0];

        int periodY = 0.5*fabs(local[1])/dy + 0.5;
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
        int ix = fabs(local[0]/dx/1.5);
        int iy = fabs(local[1]/dy);
        bool fXisEven = (ix % 2 == 0);
        bool fYisEven = (iy % 2 == 0);
        //        qDebug() << "-->ix, iy:" << ix << iy;
        double x = fabs(local[0]) - ix*1.5*dx;
        double y = fabs(local[1]) - iy*dy;
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
    FromGridElementToGridBulk[0] -= local[0];
    FromGridElementToGridBulk[1] -= local[1];
    FromGridElementToGridBulk[2] -= local[2];

    double master[3];
    Navigator->LocalToMaster(local, master);
    //  qDebug() << "-->global for the elementary block:"<<master[0]<<master[1]<<master[2];
    Navigator->SetCurrentPoint(master);
    Navigator->FindNode();

    FromGridCorrection[0] -= master[0];
    FromGridCorrection[1] -= master[1];
    FromGridCorrection[2] -= master[2];

    fGridShiftOn = true;

    //qDebug() << "-->Grid shift performed!";
    //qDebug() << "-->  Current node:"<<navigator->GetCurrentNode()->GetName();
    //qDebug() << "-->  Current point:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "-->  Shifts in XYZ to get to normal:"<<FromGridCorrection[0]<<FromGridCorrection[1]<<FromGridCorrection[2];
    return true;
}

void APhotonTracer::returnFromGridShift()
{
    //qDebug() << "<--Returning from grid shift!";
    //qDebug() << "<--Shifted coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--Currently in"<<navigator->FindNode()->GetName();
    const double * r = Navigator->GetCurrentPoint();
    double R[3];
    R[0] = r[0] + FromGridCorrection[0];
    R[1] = r[1] + FromGridCorrection[1];
    R[2] = r[2] + FromGridCorrection[2];
    Navigator->SetCurrentPoint(R);
    Navigator->FindNode();
    fGridShiftOn = false;
    //qDebug() << "<--True coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--After back shift in"<<navigator->FindNode()->GetName();
}
