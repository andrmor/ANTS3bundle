#include "aphotonsimulator.h"
#include "alogger.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ageometryhub.h"
#include "asensorhub.h"
#include "aphotonsimhub.h"
#include "astatisticshub.h"
#include "amonitorhub.h"
#include "anoderecord.h"
#include "aoneevent.h"
#include "aphotontracer.h"
#include "arandomhub.h"
#include "ajsontools.h"
#include "adepositionfilehandler.h"
#include "aerrorhub.h"
#include "as1generator.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>
#include <cmath>

APhotonSimulator::APhotonSimulator(const QString & dir, const QString & fileName, int id) :
    WorkingDir(dir), ConfigFN(fileName), ID(id),
    SimSet(APhotonSimHub::getInstance().Settings),
    RandomHub(ARandomHub::getInstance())
{
    ALogger::getInstance().open(QString("PhotonSimLog-%0.log").arg(ID));
    LOG << "Working Dir: " << WorkingDir << "\n";
    LOG << "Config file: " << ConfigFN   << "\n";

    Event = new AOneEvent();
    Tracer = new APhotonTracer(*Event, StreamTracks);
}

APhotonSimulator::~APhotonSimulator()
{
    delete S1Gen;
    delete DepoHandler;

    if (FileSensorSignals) FileSensorSignals->close();
    delete StreamSensorSignals;
    delete FileSensorSignals;

    if (FilePhotonBombs) FilePhotonBombs->close();
    delete StreamPhotonBombs;
    delete FilePhotonBombs;

    if (FileTracks) FileTracks->close();
    delete StreamTracks;
    delete FileTracks;
}

void APhotonSimulator::start()
{
    loadConfig();

    setupCommonProperties();

    openOutput();

    switch (SimSet.SimType)
    {
    case EPhotSimType::PhotonBombs :
        setupPhotonBombs();
        simulatePhotonBombs();
        break;
    case EPhotSimType::FromEnergyDepo :
        setupFromDepo();
        simulateFromDepo();
        break;
    case EPhotSimType::IndividualPhotons :
        break;
    default:;
    }

    if (bHardAbortWasTriggered) fSuccess = false;

    if (SimSet.RunSet.SaveStatistics)
    {
        QJsonObject json;
        AStatisticsHub::getInstance().SimStat.writeToJson(json);
        jstools::saveJsonToFile(json, WorkingDir + '/' + SimSet.RunSet.FileNameStatistics);
    }

    if (SimSet.RunSet.SaveMonitors)
    {
        QJsonObject json;
        AMonitorHub::getInstance().writeDataToJson(AMonitorHub::Photon, json);
        jstools::saveJsonToFile(json, WorkingDir + '/' + SimSet.RunSet.FileNameMonitors);
    }

    QCoreApplication::exit();
}

void APhotonSimulator::setupCommonProperties()
{
    RandomHub.setSeed(SimSet.RunSet.Seed);

    AMaterialHub::getInstance().updateRuntimeProperties();
    AInterfaceRuleHub::getInstance().updateRuntimeProperties();
    AStatisticsHub::getInstance().SimStat.init();

    Event->init();
    Tracer->init();
}

QString APhotonSimulator::openOutput()
{
    if (SimSet.RunSet.SaveSensorSignals)
    {
        FileSensorSignals = new QFile(WorkingDir + '/' + SimSet.RunSet.FileNameSensorSignals, this);
        if (!FileSensorSignals->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save sensor signals: " + SimSet.RunSet.FileNameSensorSignals;
        StreamSensorSignals = new QTextStream(FileSensorSignals);
    }

    if (SimSet.RunSet.SavePhotonBombs)
    {
        FilePhotonBombs = new QFile(WorkingDir + '/' + SimSet.RunSet.FileNamePhotonBombs, this);
        if (!FilePhotonBombs->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save photon bombs' data: " + SimSet.RunSet.FileNamePhotonBombs;
        StreamPhotonBombs = new QTextStream(FilePhotonBombs);
    }

    if (SimSet.RunSet.SaveTracks)
    {
        FileTracks = new QFile(WorkingDir + '/' + SimSet.RunSet.FileNameTracks, this);
        if (!FileTracks->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save photon tracks: " + SimSet.RunSet.FileNameTracks;
        StreamTracks = new QTextStream(FileTracks);
    }



    return "";
}

void APhotonSimulator::saveEventMarker()
{
    // no need to make marker for SensorSignals?

    if (SimSet.RunSet.SavePhotonBombs)
    {
        *StreamPhotonBombs << '#' << ' ' << CurrentEvent << '\n';
    }

    if (SimSet.RunSet.SaveTracks)
    {
        *StreamTracks << '#' << ' ' << CurrentEvent << '\n';
    }
}

void APhotonSimulator::saveSensorSignals()
{
    for (auto sig : qAsConst(Event->PMhits))
        *StreamSensorSignals << sig << ' ';
    *StreamSensorSignals << '\n';
}

void APhotonSimulator::savePhotonBomb(ANodeRecord * node)
{
    *StreamPhotonBombs << node->R[0] << ' ' << node->R[1] << ' ' << node->R[2] << ' ' << node->Time << ' ' << node->NumPhot << '\n';
}

void APhotonSimulator::reportProgress()
{
    std::cout << "$$>" << EventsDone << "<$$\n";
    std::cout.flush();
}

void APhotonSimulator::setupPhotonBombs()
{
    Photon.SecondaryScint = false;

    CurrentEvent = SimSet.RunSet.EventFrom;
/*
    bLimitToVolume = PhotSimSettings.bLimitToVol;
    if (bLimitToVolume)
    {
        const QString & Vol = PhotSimSettings.LimitVolume;
        if ( !Vol.isEmpty() && detector.Sandwich->World->findObjectByName(Vol) )
            LimitToVolume = PhotSimSettings.LimitVolume.toLocal8Bit().data();
        else bLimitToVolume = false;
    }
*/

/*
    if (PhotSimSettings.PerNodeSettings.Mode == APhotonSim_PerNodeSettings::Custom)
    {
        delete CustomHist; CustomHist = nullptr;

        const QVector<ADPair> Dist = PhotSimSettings.PerNodeSettings.CustomDist;
        const int size = Dist.size();
        if (size == 0)
        {
            ErrorString = "Config does not contain per-node photon distribution";
            return false;
        }

        double X[size];
        for (int i = 0; i < size; i++) X[i] = Dist.at(i).first;

        CustomHist = new TH1D("", "NumPhotDist", size-1, X);
        for (int i = 1; i < size + 1; i++) CustomHist->SetBinContent(i, Dist.at(i-1).second);
        CustomHist->GetIntegral(); //will be thread safe after this
    }
*/

/*
    const APhotonSim_FixedPhotSettings & FS = PhotSimSettings.FixedPhotSettings;
    Photon.waveIndex = FS.FixWaveIndex;
    if (!SimSet.WaveSet.Enabled) Photon.waveIndex = -1;
*/

/*
    bIsotropic = FS.bIsotropic;
    if (!bIsotropic)
    {
        if (FS.DirectionMode == APhotonSim_FixedPhotSettings::Vector)
        {
            bCone = false;
            Photon.v[0] = FS.FixDX;
            Photon.v[1] = FS.FixDY;
            Photon.v[2] = FS.FixDZ;
            NormalizeVectorSilent(Photon.v);
        }
        else //Cone
        {
            bCone = true;
            double v[3];
            v[0] = FS.FixDX;
            v[1] = FS.FixDY;
            v[2] = FS.FixDZ;
            NormalizeVectorSilent(v);
            ConeDir = TVector3(v[0], v[1], v[2]);
            CosConeAngle = cos(FS.FixConeAngle * TMath::Pi() / 180.0);
        }
    }
*/

/*
    EventsToDo = 0;
    switch (SimSet.BombSet.GenerationMode)
    {
    case EBombGen::Single :
        EventsToDo = 1; //the only case when we can split runs between threads
        break;
    case EBombGen::Grid :
    {
//        EventsToDo = PhotSimSettings.ScanSettings.countEvents();
        break;
    }
    case EBombGen::Flood :
        EventsToDo = SimSet.BombSet.FloodSettings.Number;
        break;
    case EBombGen::File :
//        if (PhotSimSettings.CustomNodeSettings.Mode == APhotonSim_CustomNodeSettings::CustomNodes)
//             EventsToDo = Nodes.size();
//        else
//               EventsToDo = PhotSimSettings.CustomNodeSettings.NumEventsInFile;
        break;
    case EBombGen::Script :
//        EventsToDo = Nodes.size();
        break;
    default:
//        ErrorString = "Unknown or not implemented photon simulation mode";
        break;
    }
*/

}

void APhotonSimulator::simulatePhotonBombs()
{
    bStopRequested = false;
    bHardAbortWasTriggered = false;

    switch (SimSet.BombSet.GenerationMode)
    {
    case EBombGen::Single :
        fSuccess = simulateSingle();
        break;
    case EBombGen::Grid :
        fSuccess = simulateGrid();
        break;
    case EBombGen::Flood :
        fSuccess = simulateFlood();
        break;
    case EBombGen::File :
    {
//        if (PhotSimSettings.CustomNodeSettings.Mode == APhotonSim_CustomNodeSettings::CustomNodes)
//             fSuccess = simulateCustomNodes();
//        else
//             fSuccess = simulatePhotonsFromFile();
        break;
    }
    case EBombGen::Script :
//        fSuccess = simulateCustomNodes();
        break;
    default:
        fSuccess = false;
        break;
    }
}

void APhotonSimulator::setupFromDepo()
{
    SimSet.DepoSet.FileName = WorkingDir + '/' + SimSet.DepoSet.FileName;
    DepoHandler = new ADepositionFileHandler(SimSet.DepoSet);
    bool ok = DepoHandler->init();
    if (!ok) terminate(AErrorHub::getQError());

    S1Gen = new AS1Generator(*Tracer);

}

void APhotonSimulator::simulateFromDepo()
{
    bStopRequested = false;
    bHardAbortWasTriggered = false;

    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        Event->clearHits();
        saveEventMarker();

        if (SimSet.DepoSet.Primary) S1Gen->clearRemainer();

        ADepoRecord depoRec;
        while (DepoHandler->readNextRecordOfSameEvent(depoRec))
        {
            if (bStopRequested) break;

            //qDebug() << "aaaaaaaaaaaaaaaaaaaa" << CurrentEvent << depoRec.Particle << depoRec.Energy;
            if (SimSet.DepoSet.Primary)
            {
                S1Gen->generate(depoRec);
                //ErrorString = "Error executing S1 generation!";
                //return false;
            }

            if (SimSet.DepoSet.Secondary) // !!!***
            {
                //S2Gen->generate(depoRecord);
                //ErrorString = "Error executing S2 generation!";
                //return false;
            }
        }

        DepoHandler->acknowledgeNextEvent();  // !!!*** end of file reached when it should not yet?

        Event->HitsToSignal();
        if (SimSet.RunSet.SaveSensorSignals) saveSensorSignals();

        EventsDone++;
        reportProgress();
    }
    qDebug() << "Done!";

    fSuccess = true; // !!!***
}

// ---

int APhotonSimulator::getNumPhotonsThisBomb()
{
    int num = 0;

    const APhotonsPerBombSettings & PS = SimSet.BombSet.PhotonsPerBomb;
    switch (PS.Mode)
    {
    case APhotonsPerBombSettings::Constant :
        num = PS.FixedNumber;
        break;
    case APhotonsPerBombSettings::Uniform :
        num = RandomHub.uniform() * (PS.UniformMax - PS.UniformMin + 1) + PS.UniformMin;
        break;
    case APhotonsPerBombSettings::Normal :
        num = std::round( RandomHub.gauss(PS.NormalMean, PS.NormalSigma) );
        break;
    case APhotonsPerBombSettings::Custom :
//        num = CustomHist->GetRandom();
        break;
    case APhotonsPerBombSettings::Poisson :
        num = RandomHub.poisson(PS.PoissonMean);
        break;
    }

    if (num < 0) num = 0;
    return num;
}

bool APhotonSimulator::simulateSingle()
{

    const double * Position = SimSet.BombSet.SingleSettings.Position;
    std::unique_ptr<ANodeRecord> node(ANodeRecord::createV(Position, 0, -1));

    simulatePhotonBombCluster(*node);
    if (bStopRequested) return false;
    return true;
}

bool APhotonSimulator::simulateGrid()
{
    const AGridSettings & ScanSet = SimSet.BombSet.GridSettings;

    double RegGridOrigin[3];
    RegGridOrigin[0] = ScanSet.X0;
    RegGridOrigin[1] = ScanSet.Y0;
    RegGridOrigin[2] = ScanSet.Z0;

    double RegGridStep[3][3]; //vector [axis] [step]
    int    RegGridNodes[3];
    bool   RegGridFlagPositive[3];

    for (int ic = 0; ic < 3; ic++)
    {
        const APhScanRecord & rec = ScanSet.ScanRecords[ic];
        if (rec.bEnabled)
        {
            RegGridStep[ic][0] = rec.DX;
            RegGridStep[ic][1] = rec.DY;
            RegGridStep[ic][2] = rec.DZ;
            RegGridNodes[ic]   = rec.Nodes;
            RegGridFlagPositive[ic] = rec.bBiDirect;
        }
        else
        {
            RegGridStep[ic][0] = 0;
            RegGridStep[ic][1] = 0;
            RegGridStep[ic][2] = 0;
            RegGridNodes[ic] = 1; //1 is disabled axis
            RegGridFlagPositive[ic] = true;
        }
    }

    std::unique_ptr<ANodeRecord> node(ANodeRecord::createS(0, 0, 0));
    int currentNode = -1;
    EventsDone = 0;
    int iAxis[3];
    for (iAxis[0]=0; iAxis[0]<RegGridNodes[0]; iAxis[0]++)
        for (iAxis[1]=0; iAxis[1]<RegGridNodes[1]; iAxis[1]++)
            for (iAxis[2]=0; iAxis[2]<RegGridNodes[2]; iAxis[2]++)  //iAxis - counters along the axes!!!
            {
                currentNode++;
                if (currentNode < SimSet.RunSet.EventFrom) continue;

                for (int i = 0; i < 3; i++) node->R[i] = RegGridOrigin[i];
                //shift from the origin
                for (int axis = 0; axis < 3; axis++)
                {
                    double ioffset = 0;
                    if (!RegGridFlagPositive[axis]) ioffset = -0.5*( RegGridNodes[axis] - 1 );
                    for (int i = 0; i < 3; i++) node->R[i] += (ioffset + iAxis[axis]) * RegGridStep[axis][i];
                }

                simulatePhotonBombCluster(*node);

                EventsDone++;
                reportProgress();

                if (currentNode >= SimSet.RunSet.EventTo) return true;
            }

    return true;
}

bool APhotonSimulator::simulateFlood()
{
    const AFloodSettings & FloodSet = SimSet.BombSet.FloodSettings;

    //extracting flood parameters
    double Xfrom, Xto, Yfrom, Yto, CenterX, CenterY, RadiusIn, RadiusOut;
    double Rad2in, Rad2out;
    if (FloodSet.Shape == AFloodSettings::Rectangular)
    {
        Xfrom = FloodSet.Xfrom;
        Xto =   FloodSet.Xto;
        Yfrom = FloodSet.Yfrom;
        Yto =   FloodSet.Yto;
    }
    else
    {
        CenterX = FloodSet.X0;
        CenterY = FloodSet.Y0;
        RadiusIn = 0.5 * FloodSet.InnerDiameter;
        RadiusOut = 0.5 * FloodSet.OuterDiameter;

        Rad2in  = RadiusIn  * RadiusIn;
        Rad2out = RadiusOut * RadiusOut;
        Xfrom   = CenterX - RadiusOut;
        Xto     = CenterX + RadiusOut;
        Yfrom   = CenterY - RadiusOut;
        Yto     = CenterY + RadiusOut;
    }
    double Zfixed, Zfrom, Zto;
    if (FloodSet.Zmode == AFloodSettings::Fixed)
        Zfixed = FloodSet.Zfixed;
    else
    {
        Zfrom = FloodSet.Zfrom;
        Zto =   FloodSet.Zto;
    }

    std::unique_ptr<ANodeRecord> node(ANodeRecord::createS(0, 0, 0));
//    int WatchdogThreshold = 100000;
//    double UpdateFactor = 100.0 / EventsToDo;
    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        qDebug() << "Simulating flood, Event#" << CurrentEvent;
        while (true)
        {
            if (bStopRequested) return false;

            node->R[0] = Xfrom + (Xto - Xfrom) * RandomHub.uniform();
            node->R[1] = Yfrom + (Yto - Yfrom) * RandomHub.uniform();

            if (FloodSet.Shape == AFloodSettings::Ring)
            {
                double r2  = (node->R[0] - CenterX)*(node->R[0] - CenterX) + (node->R[1] - CenterY)*(node->R[1] - CenterY);
                if ( r2 > Rad2out || r2 < Rad2in )
                    continue;
            }
            break;
        }

        if (FloodSet.Zmode == AFloodSettings::Fixed)
            node->R[2] = Zfixed;
        else
            node->R[2] = Zfrom + (Zto - Zfrom) * RandomHub.uniform();

/*
        if (bLimitToVolume && !isInsideLimitingObject(node->R))
        {
            WatchdogThreshold--;
            if (WatchdogThreshold < 0 && inode == 0)
            {
                ErrorString = "100000 attempts to generate a point inside the limiting object has failed!";
                return false;
            }

            inode--;
            continue;
        }
*/

        simulatePhotonBombCluster(*node);

        EventsDone++;
        reportProgress();
    }

    return true;
}

bool APhotonSimulator::simulateCustomNodes()
{
/*
    int nodeCount = (eventEnd - eventBegin);
    int currentNode = eventBegin;
    eventCurrent = 0;
    double updateFactor = 100.0 / ( NumRuns * nodeCount );

    for (int inode = 0; inode < nodeCount; inode++)
    {
        ANodeRecord * thisNode = Nodes.at(currentNode);

        for (int irun = 0; irun<NumRuns; irun++)
        {
            simulateOneNode(*thisNode);
            eventCurrent++;
            progress = eventCurrent * updateFactor;
            if(fStopRequested) return false;
        }
        currentNode++;
    }
*/
    return true;
}

bool APhotonSimulator::simulateBombsFromFile()
{
/*
    eventCurrent = 0;
    double updateFactor = 100.0 / (eventEnd - eventBegin);

    const QString FileName = PhotSimSettings.CustomNodeSettings.FileName;
    QFile file(FileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        ErrorString = "Cannot open file " + FileName;
        return false;
    }
    QTextStream in(&file);

    OneEvent->clearHits();

    for (int iEvent = -1; iEvent < eventEnd; iEvent++)
    {
        while (!in.atEnd())
        {
            if (fStopRequested)
            {
                in.flush();
                file.close();
                return false;
            }

            QString line = in.readLine().simplified();
            if (line.startsWith('#')) break; //next event in the next line of the file

            if (iEvent < eventBegin) continue;

            QStringList sl = line.split(' ', QString::SkipEmptyParts);
            if (sl.size() < 8)
            {
                ErrorString = "Format error in the file with photon records";
                file.close();
                return false;
            }

            APhoton p;
            p.r[0]      = sl.at(0).toDouble();
            p.r[1]      = sl.at(1).toDouble();
            p.r[2]      = sl.at(2).toDouble();
            p.v[0]      = sl.at(3).toDouble();
            p.v[1]      = sl.at(4).toDouble();
            p.v[2]      = sl.at(5).toDouble();
            p.waveIndex = sl.at(6).toInt();
            p.time      = sl.at(7).toDouble();

            p.scint_type = 0;
            p.SimStat    = OneEvent->SimStat;

            if (p.waveIndex < -1 || p.waveIndex >= GenSimSettings.WaveNodes) p.waveIndex = -1;

            photonTracker->TracePhoton(&p);
        }

        if (iEvent < eventBegin) continue;

        OneEvent->HitsToSignal();
        dataHub->Events.append(OneEvent->PMsignals);
        if (GenSimSettings.fTimeResolved)
            dataHub->TimedEvents.append(OneEvent->TimedPMsignals);

        AScanRecord * sr = new AScanRecord();
        sr->Points.Reinitialize(0);
        sr->ScintType = 0;
        dataHub->Scan.append(sr);

        OneEvent->clearHits();
        progress = eventCurrent * updateFactor;
        eventCurrent++;
    }
*/
    return true;
}

// ---

void APhotonSimulator::loadConfig()
{
    QJsonObject json;
    jstools::loadJsonFromFile(json, WorkingDir + "/" + ConfigFN);

    QString Error = AMaterialHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded materials: " << AMaterialHub::getInstance().countMaterials() << '\n';
    LOG.flush();

    Error         = AInterfaceRuleHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded optical rules" << '\n';
    LOG.flush();

    Error         = AGeometryHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Geometry loaded\n";
    LOG << "World: " << AGeometryHub::getInstance().World << "\n";
    LOG << "GeoManager: " << AGeometryHub::getInstance().GeoManager << "\n";
    LOG.flush();

    Error         = ASensorHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sensor hub. Defined models: " << ASensorHub::getInstance().countSensorModels() << " Defined sensors:" << ASensorHub::getInstance().countSensors() << '\n';
    LOG.flush();

    Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sim  settings. Simulation type: " << (int)SimSet.SimType << "\n";
    LOG.flush();
}

void APhotonSimulator::simulatePhotonBombCluster(ANodeRecord & node)
{
    ANodeRecord * thisNode = &node;

    const int numBombs = 1 + thisNode->getNumberOfLinkedNodes();
    Event->clearHits();

    saveEventMarker();

    for (int iPoint = 0; iPoint < numBombs; iPoint++)
    {
        const bool bInside = true; // TODO:    !(bLimitToVolume && !isInsideLimitingObject(thisNode->R));
        if (bInside)
        {
            if (thisNode->NumPhot == -1)
                thisNode->NumPhot = getNumPhotonsThisBomb();
        }
        else
            thisNode->NumPhot = 0;

        generateAndTracePhotons(thisNode);

        if (SimSet.RunSet.SavePhotonBombs) savePhotonBomb(thisNode);

        //if exists, continue to work with the linked node(s)
        thisNode = thisNode->getLinkedNode();
        if (!thisNode) break; //paranoic
    }

    Event->HitsToSignal();

    if (SimSet.RunSet.SaveSensorSignals) saveSensorSignals();
}

#include "ageometryhub.h"
#include "aphotongenerator.h"
#include "TGeoNavigator.h"
#include "TGeoManager.h"
#include "TVector3.h"
void APhotonSimulator::generateAndTracePhotons(const ANodeRecord * node)
{
    for (int i = 0; i < 3; i++) Photon.r[i] = node->R[i];
    Photon.time = node->Time;

    TGeoNavigator * navigator = AGeometryHub::getInstance().GeoManager->GetCurrentNavigator();
    for (int i = 0; i < node->NumPhot; i++)
    {
        //photon direction
//        if (bIsotropic)
            Photon.generateRandomDir();
/*
        else if (bCone)
        {
            double z = CosConeAngle + RandGen->Rndm() * (1.0 - CosConeAngle);
            double tmp = sqrt(1.0 - z*z);
            double phi = RandGen->Rndm()*3.1415926535*2.0;
            TVector3 K1(tmp*cos(phi), tmp*sin(phi), z);
            TVector3 Coll(ConeDir);
            K1.RotateUz(Coll);
            Photon.v[0] = K1[0];
            Photon.v[1] = K1[1];
            Photon.v[2] = K1[2];
        }
        //else it is already set
*/

        int MatIndex = 0;
        TGeoNode * GeoNode = navigator->FindNode(Photon.r[0], Photon.r[1], Photon.r[2]);
        if (GeoNode) MatIndex = GeoNode->GetVolume()->GetMaterial()->GetIndex();
        else
        {
            MatIndex = AGeometryHub::getInstance().Top->GetMaterial()->GetIndex(); //get material of the world
            qWarning() << "Node not found when generating photons, using material of the world";
        }

//        if (!PhotSimSettings.FixedPhotSettings.bFixWave)
            APhotonGenerator::generateWave(Photon, MatIndex);//if directly given wavelength -> waveindex is already set in PhotonOnStart

//        APhotonGenerator::generateTime(Photon, MatIndex);

        Tracer->tracePhoton(&Photon);
    }
}

void APhotonSimulator::terminate(const QString & reason)
{
    LOG << reason;
    exit(1);
}
