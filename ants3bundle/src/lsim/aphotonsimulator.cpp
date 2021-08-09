#include "aphotonsimulator.h"
#include "alogger.h"
#include "ajsontools.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ageometryhub.h"
#include "asensorhub.h"
#include "aphotonsimhub.h"
#include "anoderecord.h"
#include "aoneevent.h"
#include "aphotontracer.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

APhotonSimulator::APhotonSimulator(const QString & fileName, const QString & dir, int id) :
    ConfigFN(fileName), WorkingDir(dir), ID(id),
    SimSet(APhotonSimHub::getConstInstance().Settings)
{
    ALogger::getInstance().open(QString("PhotonSimLog-%0.log").arg(ID));
    LOG << "Working Dir: " << WorkingDir << "\n";
    LOG << "Config file: " << ConfigFN   << "\n";

    Event = new AOneEvent();
    Tracer = new APhotonTracer(*Event);

    // Progress reporter (and the following times) should live in another thread
    //QTimer * Timer = new QTimer(this);
    //QObject::connect(Timer, &QTimer::timeout, this, &APhotonSimulator::onProgressTimer);
    //Timer.start(300);

    //prepare simulator thread, start on start()
}

APhotonSimulator::~APhotonSimulator()
{
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
        break;
    case EPhotSimType::IndividualPhotons :
        break;
    default:;
    }

    QCoreApplication::exit();
}

void APhotonSimulator::setupCommonProperties()
{
    AMaterialHub::getInstance().updateRuntimeProperties();

    Event->init();
    Tracer->init();
}

QString APhotonSimulator::openOutput()
{
    if (SimSet.RunSet.SaveSensorSignals)
    {
        FileSensorSignals = new QFile(SimSet.RunSet.FileNameSensorSignals, this);
        if (!FileSensorSignals->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save sensor signals: " + SimSet.RunSet.FileNameSensorSignals;
        StreamSensorSignals = new QTextStream(FileSensorSignals);
    }

    if (SimSet.RunSet.SavePhotonBombs)
    {
        FilePhotonBombs = new QFile(SimSet.RunSet.FileNamePhotonBombs, this);
        if (!FilePhotonBombs->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save photon bombs' data: " + SimSet.RunSet.FileNamePhotonBombs;
        StreamPhotonBombs = new QTextStream(FilePhotonBombs);
    }

    if (SimSet.RunSet.SaveTracks)
    {
        FileTracks = new QFile(SimSet.RunSet.FileNameTracks, this);
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
}

void APhotonSimulator::savePhotonBomb(ANodeRecord * node)
{
    *StreamPhotonBombs << node->R[0] << ' ' << node->R[1] << ' ' << node->R[2] << ' ' << node->Time << ' ' << node->NumPhot << '\n';
}

#include "ajsontools.h"
#include <cmath>
void APhotonSimulator::saveTrack()
{
    QJsonObject json;
    if (Tracer->Track.SecondaryScint) json["s"] = 1;
    if (Tracer->Track.HitSensor)      json["h"] = 1;

    QJsonArray ar;
    for (AVector3 & pos : Tracer->Track.Positions)
    {
        QJsonArray el;
        for (int i=0; i<3; i++) el.push_back( 1.0 / TrackOutputPrecision * round(TrackOutputPrecision * pos[i]));
        ar.push_back(el);
    }
    json["P"] = ar;

    *StreamTracks << jstools::jsonToString(json) << '\n';
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
//        EventsToDo = PhotSimSettings.FloodSettings.Nodes;
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
//        fSuccess = simulateRegularGrid();
        break;
    case EBombGen::Flood :
//        fSuccess = simulateFlood();
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

    if (bHardAbortWasTriggered) fSuccess = false;
}

// ---

int APhotonSimulator::getNumPhotonsThisBomb()
{
    int num = 0;

    switch (SimSet.BombSet.PhotonNumberMode)
    {
    case EBombPhNumber::Constant :
        num = 10;
        break;
    case EBombPhNumber::Uniform :
//        num = RandGen->Rndm() * (PhotSimSettings.PerNodeSettings.Max - PhotSimSettings.PerNodeSettings.Min + 1) + PhotSimSettings.PerNodeSettings.Min;
        break;
    case EBombPhNumber::Normal :
//        num = std::round( RandGen->Gaus(PhotSimSettings.PerNodeSettings.Mean, PhotSimSettings.PerNodeSettings.Sigma) );
        break;
    case EBombPhNumber::Custom :
//        num = CustomHist->GetRandom();
        break;
    case EBombPhNumber::Poisson :
//        num = RandGen->Poisson(PhotSimSettings.PerNodeSettings.PoisMean);
        break;
    }

    if (num < 0) num = 0;
    return num;
}

bool APhotonSimulator::simulateSingle()
{
    const double * Position = SimSet.BombSet.Position;
    std::unique_ptr<ANodeRecord> node(ANodeRecord::createV(Position, 0, -1));

    simulatePhotonBombCluster(*node);
    if (bStopRequested) return false;
    return true;
}

bool APhotonSimulator::simulateGrid()
{
/*
    const APhotonSim_ScanSettings & ScanSet = PhotSimSettings.ScanSettings;

    //extracting grid parameters
    double RegGridOrigin[3]; //grid origin
    RegGridOrigin[0] = ScanSet.X0;
    RegGridOrigin[1] = ScanSet.Y0;
    RegGridOrigin[2] = ScanSet.Z0;
    //
    double RegGridStep[3][3]; //vector [axis] [step]
    int RegGridNodes[3]; //number of nodes along the 3 axes
    bool RegGridFlagPositive[3]; //Axes option
    //
    for (int ic = 0; ic < 3; ic++)
    {
        const APhScanRecord & rec = ScanSet.ScanRecords.at(ic);
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
    int currentNode = 0;
    eventCurrent = 0;
    double updateFactor = 100.0 / ( NumRuns * (eventEnd - eventBegin) );
    //Do the scan
    int iAxis[3];
    for (iAxis[0]=0; iAxis[0]<RegGridNodes[0]; iAxis[0]++)
        for (iAxis[1]=0; iAxis[1]<RegGridNodes[1]; iAxis[1]++)
            for (iAxis[2]=0; iAxis[2]<RegGridNodes[2]; iAxis[2]++)  //iAxis - counters along the axes!!!
            {
                if (currentNode < eventBegin)
                { //this node is taken care of by another thread
                    currentNode++;
                    continue;
                }

                //calculating node coordinates
                for (int i=0; i<3; i++) node->R[i] = RegGridOrigin[i];
                //shift from the origin
                for (int axis=0; axis<3; axis++)
                { //going axis by axis
                    double ioffset = 0;
                    if (!RegGridFlagPositive[axis]) ioffset = -0.5*( RegGridNodes[axis] - 1 );
                    for (int i=0; i<3; i++) node->R[i] += (ioffset + iAxis[axis]) * RegGridStep[axis][i];
                }

                //running this node
                for (int irun = 0; irun < NumRuns; irun++)
                {
                    simulateOneNode(*node);
                    eventCurrent++;
                    progress = eventCurrent * updateFactor;
                    if (fStopRequested) return false;
                }
                currentNode++;
                if (currentNode >= eventEnd) return true;
            }
*/
    return true;
}

bool APhotonSimulator::simulateFlood()
{
/*
    const APhotonSim_FloodSettings & FloodSet = PhotSimSettings.FloodSettings;

    //extracting flood parameters
    double Xfrom, Xto, Yfrom, Yto, CenterX, CenterY, RadiusIn, RadiusOut;
    double Rad2in, Rad2out;
    if (FloodSet.Shape == APhotonSim_FloodSettings::Rectangular)
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
        RadiusIn = 0.5 * FloodSet.InnerD;
        RadiusOut = 0.5 * FloodSet.OuterD;

        Rad2in  = RadiusIn  * RadiusIn;
        Rad2out = RadiusOut * RadiusOut;
        Xfrom   = CenterX - RadiusOut;
        Xto     = CenterX + RadiusOut;
        Yfrom   = CenterY - RadiusOut;
        Yto     = CenterY + RadiusOut;
    }
    double Zfixed, Zfrom, Zto;
    if (FloodSet.ZMode == APhotonSim_FloodSettings::Fixed)
    {
        Zfixed = FloodSet.Zfixed;
    }
    else
    {
        Zfrom = FloodSet.Zfrom;
        Zto =   FloodSet.Zto;
    }

    //Do flood
    std::unique_ptr<ANodeRecord> node(ANodeRecord::createS(0, 0, 0));
    int nodeCount = (eventEnd - eventBegin);
    eventCurrent = 0;
    int WatchdogThreshold = 100000;
    double updateFactor = 100.0 / (NumRuns*nodeCount);
    for (int inode = 0; inode < nodeCount; inode++)
    {
        if(fStopRequested) return false;

        //choosing node coordinates
        node->R[0] = Xfrom + (Xto - Xfrom) * RandGen->Rndm();
        node->R[1] = Yfrom + (Yto - Yfrom) * RandGen->Rndm();

        //running this node
        if (FloodSet.Shape == APhotonSim_FloodSettings::Ring)
        {
            double r2  = (node->R[0] - CenterX)*(node->R[0] - CenterX) + (node->R[1] - CenterY)*(node->R[1] - CenterY);
            if ( r2 > Rad2out || r2 < Rad2in )
            {
                inode--;
                continue;
            }
        }

        if (FloodSet.ZMode == APhotonSim_FloodSettings::Fixed)
            node->R[2] = Zfixed;
        else
            node->R[2] = Zfrom + (Zto - Zfrom) * RandGen->Rndm();

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

        for (int irun = 0; irun < NumRuns; irun++)
        {
            simulateOneNode(*node);
            eventCurrent++;
            progress = eventCurrent * updateFactor;
            if (fStopRequested) return false;
        }
    }
*/
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
    jstools::loadJsonFromFile(json, ConfigFN);

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

        if (SimSet.RunSet.SaveTracks) saveTrack();
    }
}

void APhotonSimulator::onProgressTimer()
{
    std::cout << "$$>" << EventsDone << "<$$\n";
    std::cout.flush();
    LOG << EventsDone << '\n';
}

void APhotonSimulator::terminate(const QString & reason)
{
    LOG << reason;
    exit(1);
}
