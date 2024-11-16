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
#include "aphotonbombfilehandler.h"
#include "aphotonfilehandler.h"
#include "aerrorhub.h"
#include "as1generator.h"
#include "as2generator.h"
#include "ageometryhub.h"
#include "aphotongenerator.h"
#include "aphotonfunctionalhub.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>
#include <cmath>

#include "TGeoManager.h"
#include "TGeoNavigator.h"

APhotonSimulator::APhotonSimulator(const QString & dir, const QString & fileName, int id) :
    WorkingDir(dir), ConfigFN(fileName), ID(id),
    SimSet(APhotonSimHub::getInstance().Settings),
    RandomHub(ARandomHub::getInstance())
{
    ALogger::getInstance().open(QString("PhotonSimLog-%0.log").arg(ID));
    LOG << "Working Dir: " << WorkingDir << "\n";
    LOG << "Config file: " << ConfigFN   << "\n";

    Event = new AOneEvent();
    Tracer = new APhotonTracer(*Event, StreamTracks, StreamSensorLog, StreamPhotonLog);
}

APhotonSimulator::~APhotonSimulator()
{
    delete S2Gen;
    delete S1Gen;

    delete PhotFileHandler;
    delete DepoHandler;

    if (FileSensorSignals) FileSensorSignals->close();
    delete StreamSensorHits;
    delete FileSensorSignals;

    if (FileSensorLog) FileSensorLog->close();
    delete StreamSensorLog;
    delete FileSensorLog;

    if (FilePhotonLog) FilePhotonLog->close();
    delete StreamPhotonLog;
    delete FilePhotonLog;

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
        setupIndividualPhotons();
        simulateIndividualPhotons();
        break;
    default:;
    }

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

    generateReceipt();

    QCoreApplication::exit();
}

#include "TRandom.h"
void APhotonSimulator::setupCommonProperties()
{
    gRandom->SetSeed(SimSet.RunSet.Seed); // !!!*** can be removed after random samplers use the same generator (see CustomHist->GetRandom())
    RandomHub.setSeed(SimSet.RunSet.Seed);

    AMaterialHub::getInstance().updateRuntimeProperties();
    AInterfaceRuleHub::getInstance().updateRuntimeProperties();
    ASensorHub::getInstance().updateRuntimeProperties();
    AStatisticsHub::getInstance().SimStat.init();

    Event->init();
    Tracer->configureTracer();  // Should be called after ASensorHub.updateRuntimeProperties()
}

QString APhotonSimulator::openOutput()
{
    if (SimSet.RunSet.SaveSensorSignals)
    {
        FileSensorSignals = new QFile(WorkingDir + '/' + SimSet.RunSet.FileNameSensorSignals, this);
        if (!FileSensorSignals->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save sensor signals: " + SimSet.RunSet.FileNameSensorSignals;
        StreamSensorHits = new QTextStream(FileSensorSignals);
    }

    if (SimSet.RunSet.SaveSensorLog)
    {
        FileSensorLog = new QFile(WorkingDir + '/' + SimSet.RunSet.FileNameSensorLog, this);
        if (!FileSensorLog->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save sensor log: " + SimSet.RunSet.FileNameSensorLog;
        StreamSensorLog = new QTextStream(FileSensorLog);
    }

    if (SimSet.RunSet.PhotonLogSet.Save)
    {
        FilePhotonLog = new QFile(WorkingDir + '/' + SimSet.RunSet.PhotonLogSet.FileName, this);
        if (!FilePhotonLog->open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open file to save photon log: " + SimSet.RunSet.PhotonLogSet.FileName;
        StreamPhotonLog = new QTextStream(FilePhotonLog);
    }
//
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
    if (SimSet.RunSet.SavePhotonBombs)
    {
        *StreamPhotonBombs << '#' << CurrentEvent << '\n';
    }

    if (SimSet.RunSet.SaveTracks)
    {
        *StreamTracks << '#' << CurrentEvent << '\n';
    }

    if (SimSet.RunSet.SaveSensorSignals)
    {
        *StreamSensorHits << '#' << CurrentEvent << '\n';
    }

    if (SimSet.RunSet.SaveSensorLog)
    {
        *StreamSensorLog << '#' << CurrentEvent << '\n';
    }
}

void APhotonSimulator::saveSensorHits()
{
    Event->addDarkCounts();
    Event->convertHitsToSignals();

    for (float sig : Event->PMhits) *StreamSensorHits << sig << ' ';
    *StreamSensorHits << '\n';
}

void APhotonSimulator::savePhotonBomb(ANodeRecord & node)
{
    node.writeAscii(*StreamPhotonBombs);
}

void APhotonSimulator::reportProgress()
{
    std::cout << "$$>" << EventsDone << "<$$\n";
    std::cout.flush();
}

#include "TH1D.h"
void APhotonSimulator::setupPhotonBombs()
{
    Photon.SecondaryScint = false;

    CurrentEvent = SimSet.RunSet.EventFrom;

    const APhotonAdvancedSettings & AdvSet = SimSet.BombSet.AdvancedSettings;

    // Direction inits
    Photon.v[0] = AdvSet.DirDX;
    Photon.v[1] = AdvSet.DirDY;
    Photon.v[2] = AdvSet.DirDZ;
    Photon.ensureUnitaryLength();    // if fixed direction, it will be this always. otherwise override later
    ColDirUnitary = TVector3(Photon.v);
    CosConeAngle = cos(AdvSet.ConeAngle * TMath::Pi() / 180.0);

    // Wavelength
    Photon.waveIndex = -1;
    if (AdvSet.bFixWave)
        Photon.waveIndex = SimSet.WaveSet.toIndex(AdvSet.FixedWavelength);

    // Limiters
    if (AdvSet.bOnlyVolume)   LimitToVolume   = TString(AdvSet.Volume.toLatin1().data());
    if (AdvSet.bOnlyMaterial) LimitToMaterial = AMaterialHub::getConstInstance().findMaterial(AdvSet.Material);

    // custom distribution of photons per bomb
    if (SimSet.BombSet.PhotonsPerBomb.Mode == APhotonsPerBombSettings::Custom)
    {
        delete CustomHist; CustomHist = nullptr;

        const std::vector<std::pair<int,double>> & dist = SimSet.BombSet.PhotonsPerBomb.CustomDist;
        const size_t size = dist.size();
        if (size == 0) terminate("Custom distribution of photons per bomb is empty!");
        // !!!*** do other checks (positive, increasing order)

        createCustomDist(dist);
        CustomHist->GetIntegral();
    }
}

double interpolateHere(double a, double b, double fraction)
{
    //out("    interpolation->", a, b, fraction);
    if (fraction == 0.0) return a;
    if (fraction == 1.0) return b;
    return a + fraction * (b - a);
}

void APhotonSimulator::createCustomDist(const std::vector<std::pair<int, double>> & dist)
{
    int From = dist.front().first;
    int To   = dist.back().first;
    size_t num = To - From + 1;
    CustomHist = new TH1D("", "NumPhotDist", num, From, To+1);

    int numPhot = From;
    size_t positionInDist = 0;
    while (numPhot <= To)
    {
        while (dist[positionInDist].first < numPhot) positionInDist++; // can be just ++ as dist contains unique non-decreasing ints?

        double val;
        if (dist[positionInDist].first == numPhot) val = dist[positionInDist].second; // exact match
        else
        {
            // need to interpolate

            //const double interpolationFactor = (r - data[indexOriginal].first) / ( data[indexOriginal+1].first - data[indexOriginal].first );
            const double interpolationFactor = (numPhot - dist[positionInDist-1].first) / (dist[positionInDist].first - dist[positionInDist-1].first);
            //const double interpolatedValue   = interpolateHere(data[indexOriginal].second, data[indexOriginal+1].second, interpolationFactor);
            val = interpolateHere(dist[positionInDist-1].second, dist[positionInDist].second, interpolationFactor);
        }
        CustomHist->Fill(numPhot, val);
        numPhot++;
    }
}

void APhotonSimulator::simulatePhotonBombs()
{
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
        fSuccess = simulateBombsFromFile();
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
    S2Gen = new AS2Generator(*Tracer);
}

#include "adeporecord.h"
void APhotonSimulator::simulateFromDepo()
{
    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        doBeforeEvent();

        if (SimSet.DepoSet.Primary)   S1Gen->clearRemainer();
        if (SimSet.DepoSet.Secondary) S2Gen->clearRemainer();

        ADepoRecord depoRec;
        while (DepoHandler->readNextRecordSameEvent(depoRec))
        {
            // error control in generators? !!!***
            if (SimSet.DepoSet.Primary)
            {
                S1Gen->generate(depoRec);
                //ErrorString = "Error executing S1 generation!";
                //return false;
            }

            if (SimSet.DepoSet.Secondary)
            {
                S2Gen->generate(depoRec);
                //ErrorString = "Error executing S2 generation!";
                //return false;
            }
        }

        DepoHandler->acknowledgeNextEvent();
        DepoHandler->skipToNextEventRecord();

        // !!!*** is end of file reached when it should not yet?

        doAfterEvent();
    }

    fSuccess = true;
}

// ---

void APhotonSimulator::setupIndividualPhotons()
{
    SimSet.PhotFileSet.FileName = WorkingDir + '/' + SimSet.PhotFileSet.FileName;
    PhotFileHandler = new APhotonFileHandler(SimSet.PhotFileSet);
    bool ok = PhotFileHandler->init();
    if (!ok) terminate(AErrorHub::getQError());
}

void APhotonSimulator::simulateIndividualPhotons()
{
    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        doBeforeEvent();

        APhoton phot;
        while (PhotFileHandler->readNextRecordSameEvent(phot))
            Tracer->tracePhoton(phot);
        PhotFileHandler->acknowledgeNextEvent();  // !!!*** is end of file reached when it should not yet?

        doAfterEvent();
    }

    fSuccess = true;
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
        num = CustomHist->GetRandom();
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
    const double * r = SimSet.BombSet.SingleSettings.Position;
    ANodeRecord node(r[0], r[1], r[2]);
    doBeforeEvent();
    simulatePhotonBomb(node, true);
    doAfterEvent();
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

    ANodeRecord node(0, 0, 0);
    CurrentEvent = -1;
    EventsDone = 0;
    int iAxis[3];
    for (iAxis[0]=0; iAxis[0]<RegGridNodes[0]; iAxis[0]++)
        for (iAxis[1]=0; iAxis[1]<RegGridNodes[1]; iAxis[1]++)
            for (iAxis[2]=0; iAxis[2]<RegGridNodes[2]; iAxis[2]++)  //iAxis - counters along the axes!!!
            {
                CurrentEvent++;
                if (CurrentEvent < SimSet.RunSet.EventFrom) continue;
                if (CurrentEvent >= SimSet.RunSet.EventTo) return true;

                for (int i = 0; i < 3; i++) node.R[i] = RegGridOrigin[i];
                //shift from the origin
                for (int axis = 0; axis < 3; axis++)
                {
                    double ioffset = 0;
                    if (!RegGridFlagPositive[axis]) ioffset = -0.5*( RegGridNodes[axis] - 1 );
                    for (int i = 0; i < 3; i++) node.R[i] += (ioffset + iAxis[axis]) * RegGridStep[axis][i];
                }

                doBeforeEvent();
                simulatePhotonBomb(node, true);
                doAfterEvent();
            }

    return true;
}

bool APhotonSimulator::simulateFlood()
{
    const AFloodSettings          & FloodSet = SimSet.BombSet.FloodSettings;
    const APhotonAdvancedSettings & AdvSet   = SimSet.BombSet.AdvancedSettings;

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
        CenterX   = FloodSet.X0;
        CenterY   = FloodSet.Y0;
        RadiusIn  = 0.5 * FloodSet.InnerDiameter;
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

    ANodeRecord node(0, 0, 0);
    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        //qDebug() << "Simulating flood, Event#" << CurrentEvent;
        int Watchdog = AdvSet.MaxNodeAttempts;
        while (true)
        {
            node.R[0] = Xfrom + (Xto - Xfrom) * RandomHub.uniform();
            node.R[1] = Yfrom + (Yto - Yfrom) * RandomHub.uniform();
            if (FloodSet.Shape == AFloodSettings::Ring)
            {
                double r2  = (node.R[0] - CenterX)*(node.R[0] - CenterX) + (node.R[1] - CenterY)*(node.R[1] - CenterY);
                if ( r2 > Rad2out || r2 < Rad2in )
                    continue;
            }

            if (FloodSet.Zmode == AFloodSettings::Fixed)
                node.R[2] = Zfixed;
            else
                node.R[2] = Zfrom + (Zto - Zfrom) * RandomHub.uniform();

            if ( (AdvSet.bOnlyVolume   && !isInsideLimitingVolume(node.R)) ||
                 (AdvSet.bOnlyMaterial && !isInsideLimitingMaterial(node.R)) )
            {
                Watchdog--;
                if (Watchdog < 0)
                {
                    AErrorHub::addError("Failed to generate a point inside the limiting volume/material!");
                    return false;
                }
                continue;
            }

            // acceptable position!
            break;
        }

        doBeforeEvent();
        simulatePhotonBomb(node, true);
        doAfterEvent();
    }
    return true;
}

bool APhotonSimulator::isInsideLimitingVolume(const double * r)
{
    TGeoNode * node = AGeometryHub::getInstance().GeoManager->FindNode(r[0], r[1], r[2]);
    if (!node) return false;
    return (node->GetVolume() && node->GetVolume()->GetName() == LimitToVolume);
}

bool APhotonSimulator::isInsideLimitingMaterial(const double *r)
{
    TGeoNode * node = AGeometryHub::getInstance().GeoManager->FindNode(r[0], r[1], r[2]);
    if (!node) return false;
    return (node->GetVolume() && node->GetVolume()->GetMaterial()->GetIndex() == LimitToMaterial);
}

bool APhotonSimulator::simulateBombsFromFile()
{
    ABombFileSettings & nfs = SimSet.BombSet.BombFileSettings;
    nfs.FileName = WorkingDir + '/' + nfs.FileName;

    APhotonBombFileHandler fh(nfs);
    bool ok = fh.init();
    if (!ok) return false;
    ok = fh.gotoEvent(SimSet.RunSet.EventFrom);
    if (!ok) return false;

    ANodeRecord node;
    EventsDone = 0;
    for (CurrentEvent = SimSet.RunSet.EventFrom; CurrentEvent < SimSet.RunSet.EventTo; CurrentEvent++)
    {
        doBeforeEvent();
        while (fh.readNextRecordSameEvent(node))
        {
            const bool overrideNumPhot = (node.NumPhot == -1);
            simulatePhotonBomb(node, overrideNumPhot);
        }
        doAfterEvent();

        fh.acknowledgeNextEvent();
    }

    return true;
}

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

// ---

void APhotonSimulator::loadConfig()
{
    QJsonObject json;
    jstools::loadJsonFromFile(json, WorkingDir + "/" + ConfigFN);

    // this block was the last!
    QString Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sim  settings. Simulation type: " << (int)SimSet.SimType << "\n";
    LOG.flush();

    Error = AMaterialHub::getInstance().readFromJson(json);
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
    LOG << "Loaded sensor hub. Defined models: " << ASensorHub::getInstance().countModels() << " Defined sensors:" << ASensorHub::getInstance().countSensors() << '\n';
    LOG.flush();

    Error         = APhotonFunctionalHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded photon tunnel hub.";// Defined models: " << ASensorHub::getInstance().countModels() << " Defined sensors:" << ASensorHub::getInstance().countSensors() << '\n';
    bool ok = APhotonFunctionalHub::getInstance().updateRuntimeProperties();
    if (!ok) terminate(AErrorHub::getQError());
    LOG.flush();

    /*
    Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sim  settings. Simulation type: " << (int)SimSet.SimType << "\n";
    LOG.flush();
    */
}

void APhotonSimulator::doBeforeEvent()
{
    Event->clearHits();
    saveEventMarker();
}

void APhotonSimulator::doAfterEvent()
{
    if (SimSet.RunSet.SaveSensorSignals) saveSensorHits();
    EventsDone++;
    reportProgress();
}

void APhotonSimulator::simulatePhotonBomb(ANodeRecord & node, bool overrideNumPhotons)
{
    const APhotonAdvancedSettings & AdvSet = SimSet.BombSet.AdvancedSettings;
    if ( (AdvSet.bOnlyVolume   && !isInsideLimitingVolume(node.R)) ||
         (AdvSet.bOnlyMaterial && !isInsideLimitingMaterial(node.R)) )
    {
        node.NumPhot = 0; // outside of the limiting volume/material
    }
    else
    {
        if (overrideNumPhotons) node.NumPhot = getNumPhotonsThisBomb();
    }

    generateAndTracePhotons(node);
    if (SimSet.RunSet.SavePhotonBombs) savePhotonBomb(node);
}

void APhotonSimulator::generateAndTracePhotons(const ANodeRecord & node)
{
    const APhotonAdvancedSettings & AdvSet = SimSet.BombSet.AdvancedSettings;

    for (int i = 0; i < 3; i++) Photon.r[i] = node.R[i];

    TGeoNavigator * navigator = AGeometryHub::getInstance().GeoManager->GetCurrentNavigator();
    for (int i = 0; i < node.NumPhot; i++)
    {
        // Direction
        if      (AdvSet.DirectionMode == APhotonAdvancedSettings::Isotropic)
            Photon.generateRandomDir();
        else if (AdvSet.DirectionMode == APhotonAdvancedSettings::Cone)
        {
            const double z = CosConeAngle + RandomHub.uniform() * (1.0 - CosConeAngle);
            const double tmp = sqrt(1.0 - z*z);
            const double phi = RandomHub.uniform() * 2.0 * TMath::Pi();
            TVector3 K1(tmp*cos(phi), tmp*sin(phi), z);
            K1.RotateUz(ColDirUnitary);
            for (int i = 0; i < 3; i++) Photon.v[i] = K1[i];
        }
        //else it is already set

        // Material at the emission position
        int MatIndex = 0;
        TGeoNode * GeoNode = navigator->FindNode(Photon.r[0], Photon.r[1], Photon.r[2]);
        if (GeoNode) MatIndex = GeoNode->GetVolume()->GetMaterial()->GetIndex();
        else
        {
            MatIndex = AGeometryHub::getInstance().Top->GetMaterial()->GetIndex(); //get material of the world
            qWarning() << "Node not found when generating photons, using material of the world";
        }

        // Wavelength
        if (!AdvSet.bFixWave)
            APhotonGenerator::generateWave(Photon, MatIndex); // else waveindex is already set

        // Time
        Photon.time = node.Time;
        if (!AdvSet.bFixDecay)
            APhotonGenerator::generateTime(Photon, MatIndex);
        else
            Photon.time += RandomHub.exp(AdvSet.DecayTime);

        Tracer->tracePhoton(Photon);
    }
}

void APhotonSimulator::terminate(const QString & reason)
{
    LOG << reason;

    std::cout << "$$>" << reason.toLatin1().data() << std::endl; // will flush
    generateReceipt(reason);

    exit(1);
}

void APhotonSimulator::generateReceipt(const QString & optionalError)
{
    QJsonObject receipt;

    bool bSuccess = optionalError.isEmpty();
    receipt["Success"] = bSuccess;
    if (!bSuccess) receipt["Error"] = optionalError;

    jstools::saveJsonToFile(receipt, WorkingDir + "/" + SimSet.RunSet.FileNameReceipt);
}
