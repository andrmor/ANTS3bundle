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
//#include "agridelementrecord.h"
#include "aphoton.h"
#include "amonitor.h"
#include "amonitorhub.h"

#include <QDebug>

#include "TGeoManager.h"
#include "TMath.h"
#include "TH1I.h"

APhotonTracer::APhotonTracer(AOneEvent & event) :
    MatHub(AMaterialHub::getConstInstance()),
    RuleHub(AInterfaceRuleHub::getConstInstance()),
    SensorHub(ASensorHub::getConstInstance()),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    SimStat(AStatisticsHub::getInstance().SimStat),
    Event(event)
{
//    grids = Grids;
    p = new APhoton();
}

APhotonTracer::~APhotonTracer()
{
    delete p;
}

void APhotonTracer::init()
{
    Track.Positions.reserve(SimSet.OptSet.MaxPhotonTransitions + 1);
    AddedTracks  = 0;
}

void APhotonTracer::tracePhoton(const APhoton * Photon)
{
    qDebug() << "Tracing photon" << SimSet.RunSet.SavePhotonLog;
    if (bAbort) return;

    if (SimSet.OptSet.CheckQeBeforeTracking)
    {
        rnd = RandomHub.uniform();
        if (SimSet.WaveSet.Enabled && Photon->waveIndex != -1)
        {
            if (rnd > SensorHub.getMaxQEvsWave(Photon->waveIndex))  // !!!*** merge methods to one?
            {
                SimStat.TracingSkipped++;
                return; //no need to trace this photon
            }
        }
        else
        {
            if (rnd > SensorHub.getMaxQE())                         // !!!*** merge methods to one?
            {
                SimStat.TracingSkipped++;
                return; //no need to trace this photon
            }
        }
    }

    //=====inits=====
    GeoManager = AGeometryHub::getInstance().GeoManager;
    Navigator = GeoManager->GetCurrentNavigator();
    //qDebug() << "Navigator:" << Navigator;
    if (!Navigator)
    {
        qDebug() << "Photon tracer: navigator does not exist, creating new";
        Navigator = GeoManager->AddNavigator();
    }
    Navigator->SetCurrentPoint(Photon->r);
    Navigator->SetCurrentDirection(Photon->v);
    Navigator->FindNode();
    fGridShiftOn = false;

    if (Navigator->IsOutside())
    {
        //     qDebug()<<"Generated outside geometry!";
        SimStat.GeneratedOutside++;
        if (SimSet.RunSet.SavePhotonLog)
        {
            PhLog.clear();
            PhLog.append( APhotonHistoryLog(p->r, "", p->time, p->waveIndex, APhotonHistoryLog::GeneratedOutsideGeometry) );
        }
        return;
    }

    p->copyFrom(Photon);
    //qDebug()<<"Photon starts from:";
    //qDebug()<<Navigator->GetPath();
    //qDebug()<<"material name: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetName();
    //qDebug()<<"material index: "<<Navigator->GetCurrentVolume()->GetMaterial()->GetIndex();

    if (SimSet.RunSet.SaveTracks)
    {
        if (AddedTracks < SimSet.RunSet.MaxTracks)
        {
            Track.HitSensor = false;
            Track.SecondaryScint = p->SecondaryScint;
            Track.Positions.clear();
            Track.Positions.push_back(AVector3(p->r));
        }
        else APhotonSimHub::getInstance().Settings.RunSet.SaveTracks = false;
    }

    VolumeTo   = Navigator->GetCurrentVolume();
    MatIndexTo = VolumeTo->GetMaterial()->GetIndex();

    if (SimSet.RunSet.SavePhotonLog)
    {
        PhLog.clear();
        PhLog.reserve(SimSet.OptSet.MaxPhotonTransitions);
        PhLog.append( APhotonHistoryLog(p->r, Navigator->GetCurrentVolume()->GetName(), p->time, p->waveIndex, APhotonHistoryLog::Created, MatIndexFrom) );
    }

    TGeoNode * NodeAfterInterface;
    Counter = 0; //number of photon transitions - there is a limit on this set by user
    //---------------------------------------------=====cycle=====-----------------------------------------
    while (Counter < SimSet.OptSet.MaxPhotonTransitions)
    {
        Counter++;
        //qDebug() << "tracing cycle #" << Counter;

        VolumeFrom   = VolumeTo;
        MatIndexFrom = MatIndexTo;

        MaterialFrom = MatHub[MatIndexFrom]; //this is the material where the photon is currently in
        if (SimSet.RunSet.SavePhotonLog) nameFrom = Navigator->GetCurrentVolume()->GetName();

        Navigator->FindNextBoundary();
        Step = Navigator->GetStep();
        //qDebug()<<"Step:"<<Step;

        //----- Bulk Absorption or Rayleigh scattering -----
        switch ( AbsorptionAndRayleigh() )
        {
        case AbsTriggered: goto force_stop_tracing; //finished with this photon
        case RayTriggered: continue; //next step
        case WaveShifted: continue; //next step
        default:; // not triggered
        }

        //----- Making a step towards the interface ------
        Navigator->Step(kTRUE, kFALSE); //stopped right before the crossing

        Double_t R_afterStep[3];
        if (fGridShiftOn && Step>0.001) // !!!*** why 0.001?
        {   //if point after Step is outside of grid bulk, kill this photon
            // it is not realistic in respect of Rayleigh for photons travelling in grid plane
            //also, there is a small chance that photon is rejected by this procedure when it hits the grid wire straight on center
            //chance is very small (~0.1%) even for grids with ~25% transmission value

            const Double_t* rc = Navigator->GetCurrentPoint();
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
                //if (fGridShiftOn) track->Nodes.append(TrackNodeStruct(R_afterStep, p->time)); //not drawing end of the track for such photons!
                SimStat.LossOnGrid++;
                goto force_stop_tracing;
            }
        }

        //placing this location/state to the stack - it will be restored if photon is reflected
        Navigator->PushPoint(); //DO NOT FORGET TO CLEAN IT IF NOT USED!!!

        //can make the track now - the photon made it to the other border in any case
        double refIndex = MaterialFrom->getRefractiveIndex(p->waveIndex);
        //qDebug() << "Refractive index from:" << refIndex;
        p->time += Step / c_in_vac * refIndex;
        if (SimSet.RunSet.SaveTracks && Step > 0.001)
        {
            if (fGridShiftOn)
            {
                //qDebug() << "Added to track using shifted gridnavigator data:"<<R[0]<<R[1]<<R[2];
                //track->Nodes.append(TrackNodeStruct(R_afterStep, p->time));
                Track.Positions.push_back(AVector3(R_afterStep));
            }
            else
            {
                //qDebug() << "Added to track using normal navigator:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
                //track->Nodes.append(TrackNodeStruct(Navigator->GetCurrentPoint(), p->time));
                Track.Positions.push_back(AVector3(Navigator->GetCurrentPoint()));
            }
        }

        // ----- Now making a step inside the next material volume on the path -----
        NodeAfterInterface = Navigator->FindNextBoundaryAndStep(); //this is the node after crossing the boundary
        //this method MOVES the current position! different from FindNextBoundary method, which only calculates the step
        //now current point is inside the next volume!

        //check if after the border the photon is outside the defined world
        if (Navigator->IsOutside()) //outside of geometry - end tracing
        {
            //qDebug() << "Photon escaped!";
            Navigator->PopDummy();//clean up the stack
            SimStat.Escaped++;
            if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Escaped) );
            goto force_stop_tracing; //finished with this photon
        }

        //new volume info
        VolumeTo = NodeAfterInterface->GetVolume();
        if (SimSet.RunSet.SavePhotonLog) nameTo = Navigator->GetCurrentVolume()->GetName();
        MatIndexTo = VolumeTo->GetMaterial()->GetIndex();
        MaterialTo = MatHub[MatIndexTo];
        fHaveNormal = false;

        //qDebug()<<"Found border with another volume: "<<ThisVolume->GetName();
        //qDebug()<<"Mat index after interface: "<<MatIndexTo<<" Mat index before: "<<MatIndexFrom;
        //qDebug()<<"coordinates: "<<Navigator->GetCurrentPoint()[0]<<Navigator->GetCurrentPoint()[1]<<Navigator->GetCurrentPoint()[2];

        //-----Checking overrides-----
        AInterfaceRule * rule = nullptr;

        if (VolumeFrom->GetTitle()[1] == '*' && VolumeTo->GetTitle()[2] == '*')
        {
            rule = RuleHub.getVolumeRule(VolumeFrom->GetName(), VolumeTo->GetName());
            rule->updateMatIndices(MatIndexFrom, MatIndexTo);
        }
        if (!rule) rule = RuleHub.getMaterialRuleFast(MatIndexFrom, MatIndexTo);  // !!!*** to const

        if (rule)
        {
            //qDebug() << "Interface rule defined! Model = "<<ov->getType();
            N = Navigator->FindNormal(kFALSE);
            fHaveNormal = true;
            const double* PhPos = Navigator->GetCurrentPoint();
            for (int i=0; i<3; i++) p->r[i] = PhPos[i];
            AInterfaceRule::OpticalOverrideResultEnum result = rule->calculate(p, N);
            if (bAbort) return;

            switch (result)
            {
            case AInterfaceRule::Absorbed:
                //qDebug() << "-Override: absorption triggered";
                Navigator->PopDummy(); //clean up the stack
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.append( APhotonHistoryLog(PhPos, nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Override_Loss, MatIndexFrom, MatIndexTo) );
                SimStat.InterfaceRuleLoss++;
                goto force_stop_tracing; //finished with this photon
            case AInterfaceRule::Back:
                //qDebug() << "-Override: photon bounced back";
                Navigator->PopPoint();  //remaining in the original volume
                Navigator->SetCurrentDirection(p->v); //updating direction
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.append( APhotonHistoryLog(PhPos, nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Override_Back, MatIndexFrom, MatIndexTo) );
                SimStat.InterfaceRuleBack++;
                continue; //send to the next iteration
            case AInterfaceRule::Forward:
                Navigator->SetCurrentDirection(p->v); //updating direction
                fDoFresnel = false; //stack cleaned afterwards
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.append( APhotonHistoryLog(PhPos, nameTo, p->time, p->waveIndex, APhotonHistoryLog::Override_Forward, MatIndexFrom, MatIndexTo) );
                SimStat.InterfaceRuleForward++;
                break; //switch break
            case AInterfaceRule::NotTriggered:
                fDoFresnel = true;
                break; //switch break
            default:
                qCritical() << "override error - doing fresnel instead!";
                fDoFresnel = true;
            }
        }
        else fDoFresnel = true;


        //-- Interface rule not set or not triggered --

        if (fDoFresnel)
        {
            //qDebug()<<"Performing fresnel"; //Fresnel equations //http://en.wikipedia.org/wiki/Fresnel_equations
            if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
            fHaveNormal = true;
            //qDebug()<<"Normal length is:"<<sqrt(N[0]*N[0] + N[1]*N[1] + N[2]*N[2]);
            //qDebug()<<"Dir vector length is:"<<sqrt(p->v[0]*p->v[0] + p->v[1]*p->v[1] + p->v[2]*p->v[2]);

            const double prob = CalculateReflectionCoefficient(); //reflection probability
            if (RandomHub.uniform() < prob)
            { //-----Reflection-----
                //qDebug()<<"Fresnel - reflection!";
                SimStat.FresnelReflected++;
                // photon remains in the same volume -> continue to the next iteration
                Navigator->PopPoint(); //restore the point before the border
                PerformReflection();
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Fresnel_Reflection, MatIndexFrom, MatIndexTo) );
                continue;
            }
            //otherwise transmission
            SimStat.FresnelTransmitted++;
        }


        // --------------- Photon entered another volume ----------------------
        Navigator->PopDummy(); //clean up the stack

/*      !!!***
        //if passed overrides and gridNavigator!=0, abandon it - photon is considered to exit the grid
        if (fGridShiftOn && Step >0.001)
        {
            //qDebug() << "++Grid back shift triggered!";
            if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Grid_ShiftOut) );
            ReturnFromGridShift();
            Navigator->FindNode();
            if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Grid_Exit) );
            //         qDebug() << "Navigator coordinates: "<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
        }
*/

        //const char* VolName = ThisVolume->GetName();
        //qDebug()<<"Photon entered new volume:" << VolName;
        const char* VolTitle = VolumeTo->GetTitle();
        //qDebug() << "Title:"<<VolTitle;

        switch (VolTitle[0])
        {
        case 'S': // Sensor hit
            {
                const int iSensor = NodeAfterInterface->GetNumber();
                //qDebug()<< "Sensor hit! (" << ThisVolume->GetTitle() <<") Sensor name:"<< ThisVolume->GetName() << "Sensor index" << iSensor;
                if (SimSet.RunSet.SavePhotonLog)
                {
                    PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
                    PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::HitSensor, -1, -1, iSensor) );
                }
                Track.HitSensor = true;
                processSensorHit(iSensor);
                SimStat.HitSensor++;
                goto force_stop_tracing; //finished with this photon
            }
/*      !!!***
        case 'G': // grid hit
            {
            //qDebug() << "Grid hit!" << ThisVolume->GetName() << ThisVolume->GetTitle()<< "Number:"<<NodeAfterInterface->GetNumber();
            if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Grid_Enter) );
            GridWasHit(NodeAfterInterface->GetNumber()); // it is assumed that "empty part" of the grid element will have the same refractive index as the material from which photon enters it
            GridVolume = ThisVolume;
            if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Grid_ShiftIn) );
            break;
            }
*/
        case 'M': //monitor
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
                    if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
                    double cosAngle = 0;
                    for (int i=0; i<3; i++) cosAngle += N[i] * p->v[i];
                    MonitorHub.PhotonMonitors[iMon].Monitor->fillForPhoton(local[0], local[1], p->time, 180.0/3.1415926535*TMath::ACos(cosAngle), p->waveIndex);
                    if (MonitorHub.PhotonMonitors[iMon].Monitor->isStopsTracking())
                    {
                        SimStat.MonitorKill++;
                        if (SimSet.RunSet.SavePhotonLog) PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::KilledByMonitor) );
                        goto force_stop_tracing; //finished with this photon
                    }
                }
                break;
            }
        default:
            {
            //other volumes have title statring with '-'
            }
        }


        //if doing fresnel, perform refraction
        if (fDoFresnel)
        {
            //-----Refraction-----
            const bool ok = PerformRefraction( RefrIndexFrom/RefrIndexTo); // true - successful
            // true - successful, false - forbidden -> considered that the photon is absorbed at the surface! Should not happen
            if (!ok) qWarning()<<"Error in photon tracker: problem with transmission!";
            if (SimSet.RunSet.SavePhotonLog)
                PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), nameTo, p->time, p->waveIndex, APhotonHistoryLog::Fresnel_Transmition, MatIndexFrom, MatIndexTo) );
        }

    } //while cycle: going to the next iteration

    // maximum number of transitions reached
    if (Counter == SimSet.OptSet.MaxPhotonTransitions) SimStat.MaxTransitions++;

    //here all tracing terminators end
force_stop_tracing:
    if (SimSet.RunSet.SavePhotonLog) AppendHistoryRecord(); //Add tracks is also there, it has extra filtering   !!!*** not parallel!!!
    if (SimSet.RunSet.SaveTracks)    AppendTrack();

    //qDebug()<<"Finished with the photon";
}

void APhotonTracer::hardAbort()
{
    bAbort = true;
}

void APhotonTracer::AppendHistoryRecord()
{
    const APhotonLogSettings & LogSet = SimSet.RunSet.LogSet;
    bool bVeto = false;
    //by process
    if (!LogSet.MustNotInclude_Processes.isEmpty())
    {
        for (int i=0; i<PhLog.size(); i++)
            if ( LogSet.MustNotInclude_Processes.contains(PhLog[i].process) )
            {
                bVeto = true;
                break;
            }
    }
    //by Volume
    if (!bVeto && !LogSet.MustNotInclude_Volumes.isEmpty())
    {
        for (int i=0; i<PhLog.size(); i++)
            if ( LogSet.MustNotInclude_Volumes.contains(PhLog[i].volumeName) )
            {
                bVeto = true;
                break;
            }
    }

    if (!bVeto)
    {
        bool bFound = true;
        //in processes
        for (int im = 0; im < LogSet.MustInclude_Processes.size(); im++)
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
            for (int im = 0; im < LogSet.MustInclude_Volumes.size(); im++)
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

void APhotonTracer::AppendTrack()
{
//    if ( SimSet->TrackBuildOptions.bSkipPhotonsMissingPMs && fMissPM )
//        delete track;  !!!*** TODO
//    else
    {
        saveTrack();
        AddedTracks++;
    }
}

APhotonTracer::AbsRayEnum APhotonTracer::AbsorptionAndRayleigh()
{
    //Optimized assuming rare use of Rayleigh and very rare use when both abs and Rayleigh are defined!
    //if both active, extract distances at which the processes would trigger, then select the process with the shortest path
    //qDebug() << "Abs and Ray tests";

    //prepare abs
    bool DoAbsorption;
    double AbsPath;
    const double AbsCoeff = MatHub[MatIndexFrom]->getAbsorptionCoefficient(p->waveIndex);
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
        if (p->waveIndex == -1) RayleighMFP = MaterialFrom->rayleighMFP;
        else RayleighMFP = MaterialFrom->rayleighBinned[p->waveIndex];

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
            double refIndex = MaterialFrom->getRefractiveIndex(p->waveIndex);
            p->time += AbsPath/c_in_vac*refIndex;
            SimStat.Absorbed++;
            SimStat.BulkAbsorption++;

            if (SimSet.RunSet.SaveTracks || SimSet.RunSet.SavePhotonLog)
            {
                Double_t point[3];
                point[0] = Navigator->GetCurrentPoint()[0] + p->v[0]*AbsPath;
                point[1] = Navigator->GetCurrentPoint()[1] + p->v[1]*AbsPath;
                point[2] = Navigator->GetCurrentPoint()[2] + p->v[2]*AbsPath;
                if (SimSet.RunSet.SaveTracks)
                {
                    //track->Nodes.append(TrackNodeStruct(point, p->time));
                    Track.Positions.push_back(AVector3(point));
                }
                if (SimSet.RunSet.SavePhotonLog)
                    PhLog.append( APhotonHistoryLog(point, nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Absorbed, MatIndexFrom) );
            }

            //check if this material is waveshifter
            const double reemissionProb = MatHub[MatIndexFrom]->getReemissionProbability(p->waveIndex);
            if ( reemissionProb > 0 )
            {
                if (RandomHub.uniform() < reemissionProb)
                {
                    //qDebug() << "Waveshifting! Original index:"<<p->waveIndex;
                    if (p->waveIndex!=-1 && MatHub[MatIndexFrom]->PrimarySpectrumHist)
                    {
                        double wavelength;
                        int waveIndex;
                        int attempts = -1;
                        do
                        {
                            attempts++;
                            if (attempts > 9) return AbsTriggered;  // ***!!! absolute number
                            wavelength = MatHub[MatIndexFrom]->PrimarySpectrumHist->GetRandom();
                            //qDebug() << "   "<<wavelength << " MatIndexFrom:"<< MatIndexFrom;
                            waveIndex = SimSet.WaveSet.toIndexFast(wavelength); // !!!*** before was round here:
                            //waveIndex = round( (wavelength - SimSet->WaveFrom)/SimSet->WaveStep );
                        }
                        while (waveIndex < p->waveIndex); //conserving energy

                        //qDebug() << "NewIndex:"<<waveIndex;
                        p->waveIndex = waveIndex;
                    }
                    else p->waveIndex = -1; // this is to allow to use this procedure to make diffuse medium (including -1 waveIndex)

                    double R[3];
                    R[0] = Navigator->GetCurrentPoint()[0] + p->v[0]*AbsPath;
                    R[1] = Navigator->GetCurrentPoint()[1] + p->v[1]*AbsPath;
                    R[2] = Navigator->GetCurrentPoint()[2] + p->v[2]*AbsPath;
                    Navigator->SetCurrentPoint(R);

                    RandomDir();
                    Navigator->SetCurrentDirection(p->v);
                    //qDebug() << "After:"<<p->WaveIndex;

                    //if (SimSet->fTimeResolved)
                    //    p->time += RandGen->Exp(  MaterialFrom->PriScintDecayTime );
                    p->time += MatHub[MatIndexFrom]->generatePrimScintTime(RandomHub);

                    SimStat.Reemission++;
                    if (SimSet.RunSet.SavePhotonLog)
                        PhLog.append( APhotonHistoryLog(R, nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Reemission, MatIndexFrom) );
                    return WaveShifted;
                }
            }
            //else absorption
            return AbsTriggered;
        }

        if (DoRayleigh)
        {
            //qDebug()<<"Scattering was triggered";
            //interaction position
            double R[3];
            R[0] = Navigator->GetCurrentPoint()[0] + p->v[0]*RayleighPath;
            R[1] = Navigator->GetCurrentPoint()[1] + p->v[1]*RayleighPath;
            R[2] = Navigator->GetCurrentPoint()[2] + p->v[2]*RayleighPath;
            Navigator->SetCurrentPoint(R);
            //navigator->FindNode();  /// not needed - removed

            //new direction
            double v_old[3];
            v_old[0] = p->v[0]; v_old[1] = p->v[1]; v_old[2] = p->v[2];
            double dotProduct;
            do
            {
                RandomDir();
                dotProduct = p->v[0]*v_old[0] + p->v[1]*v_old[1] + p->v[2]*v_old[2];
            }
            while ( (dotProduct*dotProduct + 1.0) < 2.0*RandomHub.uniform());
            Navigator->SetCurrentDirection(p->v);

            double refIndex = MaterialFrom->getRefractiveIndex(p->waveIndex);
            p->time += RayleighPath/c_in_vac*refIndex;
            SimStat.Rayleigh++;

            //updating track if needed
            if (SimSet.RunSet.SaveTracks)
            {
                //track->Nodes.append(TrackNodeStruct(R, p->time));
                Track.Positions.push_back(AVector3(R));
            }
            if (SimSet.RunSet.SavePhotonLog)
                PhLog.append( APhotonHistoryLog(R, nameFrom, p->time, p->waveIndex, APhotonHistoryLog::Rayleigh, MatIndexFrom) );

            return RayTriggered;
        }
    }

    //qDebug() << "Abs and Ray - not triggered";
    return AbsRayNotTriggered;
}

double APhotonTracer::CalculateReflectionCoefficient()
{
    const double NK = N[0]*p->v[0] + N[1]*p->v[1] + N[2]*p->v[2]; // NK = cos of the angle of incidence = cos1
    const double cos1 = fabs(NK);
    //qDebug() << "Cos of incidence:"<<cos1;

    RefrIndexFrom = MaterialFrom->getRefractiveIndex(p->waveIndex);
    RefrIndexTo   = MaterialTo->getRefractiveIndex(p->waveIndex);

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

void APhotonTracer::processSensorHit(int PMnumber)
{
    const bool fSiPM = SensorHub.isSiPM(PMnumber);

    double local[3];//if no area dep or not SiPM - local[0] and [1] are undefined!
    if (SensorHub.isAreaResolvedPDE(PMnumber) || fSiPM)
    {
        const Double_t *global = Navigator->GetCurrentPoint();
        Navigator->MasterToLocal(global, local);
        //qDebug()<<local[0]<<local[1];
    }

    //since we check vs cos of _refracted_:
    if (fDoFresnel) PerformRefraction( RefrIndexFrom / RefrIndexTo); // true - successful
    if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
    //       qDebug()<<N[0]<<N[1]<<N[2]<<"Normal length is:"<<sqrt(N[0]*N[0]+N[1]*N[1]+N[2]*N[2]);
    //       qDebug()<<K[0]<<K[1]<<K[2]<<"Dir vector length is:"<<sqrt(K[0]*K[0]+K[1]*K[1]+K[2]*K[2]);
    double cosAngle = 0;
    for (int i=0; i<3; i++) cosAngle += N[i] * p->v[i];
    //       qDebug()<<"cos() = "<<cosAngle;

    if (!SimSet.OptSet.CheckQeBeforeTracking) rnd = RandomHub.uniform(); //else already calculated

    bool bDetected;
    if (fSiPM)
        bDetected = Event.CheckSiPMhit(PMnumber, p->time, p->waveIndex, local[0], local[1], cosAngle, Counter, rnd);
    else
        bDetected = Event.CheckPMThit(PMnumber, p->time, p->waveIndex, local[0], local[1], cosAngle, Counter, rnd);

    if (SimSet.RunSet.SavePhotonLog)
        PhLog.append( APhotonHistoryLog(Navigator->GetCurrentPoint(), Navigator->GetCurrentVolume()->GetName(), p->time, p->waveIndex, (bDetected ? APhotonHistoryLog::Detected : APhotonHistoryLog::NotDetected), -1, -1, PMnumber) );
}

bool APhotonTracer::PerformRefraction(double nn) // nn = nFrom / nTo
{
    //qDebug()<<"refraction triggered, n1/n2 ="<<nn;
    //N - normal vector, K - origial photon direction vector
    // nn = n(in)/n(tr)
    // (NK) - scalar product
    // T = -( nn*(NK) - sqrt(1-nn*nn*[1-(NK)*(NK)]))*N + nn*K

    if (!fHaveNormal) N = Navigator->FindNormal(kFALSE);
    const Double_t NK = N[0]*p->v[0] + N[1]*p->v[1] + N[2]*p->v[2];

    const Double_t UnderRoot = 1.0 - nn*nn*(1.0 - NK*NK);
    if (UnderRoot<0)
    {
        //        qDebug()<<"total internal detected - will assume this photon is abosrbed at the surface";
        return false;    //total internal reflection! //previous reflection test should catch this situation
    }
    const Double_t tmp = nn*NK - sqrt(UnderRoot);

    p->v[0] = -tmp*N[0] + nn*p->v[0];
    p->v[1] = -tmp*N[1] + nn*p->v[1];
    p->v[2] = -tmp*N[2] + nn*p->v[2];

    Navigator->SetCurrentDirection(p->v);
    return true;
}

void APhotonTracer::PerformReflection()
{
    if (!fHaveNormal)
    {
        N = Navigator->FindNormal(kFALSE);
        fHaveNormal = true;
    }
    //qDebug() << "Normal:"<<N[0]<<N[1]<<N[2];
    //qDebug() << "Vector before:"<<p->v[0]<<p->v[1]<<p->v[2];

    //rotating the vector
    //K = K - 2*(NK)*N    where K is the photon direction vector
    const double NK = N[0]*p->v[0] + N[1]*p->v[1] + N[2]*p->v[2];
    p->v[0] -= 2.0 * NK * N[0];
    p->v[1] -= 2.0 * NK * N[1];
    p->v[2] -= 2.0 * NK * N[2];

    //qDebug() << "Vector after:"<<p->v[0]<<p->v[1]<<p->v[2];
    //qDebug() << "Photon position:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];

    Navigator->SetCurrentDirection(p->v);
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
    p->v[2] = ( -1.0 + 8.0 * r2 );
    double scale = 8.0 * TMath::Sqrt(0.25 - r2);
    p->v[0] = a*scale;
    p->v[1] = b*scale;
}

bool APhotonTracer::GridWasHit(int GridNumber)
{
    /*

    //calculating where we are in respect to the grid element
    const Double_t *global = Navigator->GetCurrentPoint();
    //init only:
    FromGridCorrection[0] = global[0];
    FromGridCorrection[1] = global[1];
    FromGridCorrection[2] = global[2];

    if (GridNumber<0 || GridNumber>grids->size()-1)
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

    if ((*grids)[GridNumber]->shape < 2)
    {
        //rectangular grid
        double& dx = (*grids)[GridNumber]->dx;
        double& dy = (*grids)[GridNumber]->dy;
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
        double dx = (*grids)[GridNumber]->dx*1.1547; // 1.0/cos(30)
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

    */
    return false;
}

void APhotonTracer::ReturnFromGridShift()
{
/*
    //qDebug() << "<--Returning from grid shift!";
    //qDebug() << "<--Shifted coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--Currently in"<<navigator->FindNode()->GetName();
    const Double_t* r = Navigator->GetCurrentPoint();
    Double_t R[3];
    R[0] = r[0] + FromGridCorrection[0];
    R[1] = r[1] + FromGridCorrection[1];
    R[2] = r[2] + FromGridCorrection[2];
    Navigator->SetCurrentPoint(R);
    Navigator->FindNode();
    fGridShiftOn = false;
    //qDebug() << "<--True coordinates:"<<navigator->GetCurrentPoint()[0]<<navigator->GetCurrentPoint()[1]<<navigator->GetCurrentPoint()[2];
    //qDebug() << "<--After back shift in"<<navigator->FindNode()->GetName();
*/
}
