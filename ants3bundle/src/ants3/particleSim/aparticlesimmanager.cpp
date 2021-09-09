#include "aparticlesimmanager.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "asourceparticlegenerator.h"
#include "ageometryhub.h"
#include "a3farmnoderecord.h"
#include "a3workdistrconfig.h"
#include "adispatcherinterface.h"
#include "a3global.h"
#include "arandomhub.h"
#include "ajsontools.h"

#include <QDebug>

AParticleSimManager & AParticleSimManager::getInstance()
{
    static AParticleSimManager instance;
    return instance;
}

AParticleSimManager::AParticleSimManager() :
    SimSet(AParticleSimHub::getInstance().Settings),
    Geometry(AGeometryHub::getConstInstance())
{
    Generator_Sources = new ASourceParticleGenerator(SimSet.SourceGenSettings);
}

AParticleSimManager::~AParticleSimManager()
{
    delete Generator_Sources;
}

bool AParticleSimManager::simulate(int numLocalProc)
{
    ErrorString.clear();

    doPreSimChecks();
    if (!ErrorString.isEmpty()) return false;

    const int numEvents = getNumberEvents();
    if (numEvents == 0)
    {
        ErrorString = "Configuration resports zero events to simulate";
        return false;
    }

//    removeOutputFiles();  // note that output files in exchange dir will be deleted in adispatcherinterface

    // configure number of local/remote processes to run
    std::vector<A3FarmNodeRecord> RunPlan;
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QString err = Dispatcher.fillRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;

    bool ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    qDebug() << "Running simulation...";
    QJsonObject Reply = Dispatcher.performTask(Request);

//    processReply(Reply);

//    if (ErrorString.isEmpty()) mergeOutput();

    return ErrorString.isEmpty();
}

void AParticleSimManager::doPreSimChecks()
{
    checkG4Settings();
    checkDirectories();
    addErrorLine(Geometry.checkVolumesExist(SimSet.G4Set.SensitiveVolumes));
}

#include "amaterialhub.h"
void AParticleSimManager::checkG4Settings()
{
    // TODO: no sensitive volumes but photon sim scheduled

    // TODO: optical grids will not be expanded

    AMaterialHub::getInstance().checkReadyForGeant4Sim(ErrorString);
}

int AParticleSimManager::getNumberEvents() const
{
    int numEvents = SimSet.Events;
    if (SimSet.GenerationMode == AParticleSimSettings::File)
    {
        //limit to max in file
    }
    return numEvents;
}

bool AParticleSimManager::configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "g4ants3";

    const QString ExchangeDir = A3Global::getConstInstance().ExchangeDir;

    bool ok = configureGDML(Request, ExchangeDir);
    if (!ok) return false;

    ok = configureMonitors(Request, ExchangeDir); // !!!***
    if (!ok) return false;

    configureMaterials();

    ARandomHub & RandomHub = ARandomHub::getInstance();
    RandomHub.setSeed(SimSet.RunSet.Seed);

    int iEvent = 0;
    int iProcess = 0;

    AParticleGun * ParticleGun = nullptr;
    switch (SimSet.GenerationMode)
    {
    case AParticleSimSettings::Sources :
        ParticleGun = Generator_Sources;
        break;
    case AParticleSimSettings::File :
        //break;
    case AParticleSimSettings::Script :
        //break;
    default:;
    }
    if (!ParticleGun)
    {
        ErrorString = "Unknown or not implemented particle generation mode";
        return false;
    }
    if ( !ParticleGun->init() )
    {
        ErrorString = ParticleGun->ErrorString.data();
        return false;
    }

    for (const A3FarmNodeRecord & r : RunPlan)   // per node server
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)            // per process at the node server
        {
            if (num == 0) break;

            A3NodeWorkerConfig Worker;
            AParticleSimSettings WorkSet;

            switch (SimSet.GenerationMode)
            {
            case AParticleSimSettings::Sources :
                WorkSet = SimSet;
                WorkSet.RunSet.EventFrom = iEvent;
                WorkSet.RunSet.EventTo   = iEvent + num;
                iEvent += num;
                break;
            default:
                ErrorString = "Not yet implemented!";
                return false;
            }

            WorkSet.RunSet.Seed = INT_MAX * RandomHub.uniform();

            WorkSet.RunSet.Receipt = "receipt-" + std::to_string(iProcess) + ".txt";
/*
            if (SimSet.RunSet.SaveSensorSignals)
            {
                WorkSet.RunSet.FileNameSensorSignals = QString("signals-%0").arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameSensorSignals);
                SignalFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameSensorSignals);
            }
            if (SimSet.RunSet.SaveTracks)
            {
                WorkSet.RunSet.FileNameTracks       = QString("tracks-%0").arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameTracks);
                TrackFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameTracks);
            }
            if (SimSet.RunSet.SavePhotonBombs)
            {
                WorkSet.RunSet.FileNamePhotonBombs  = QString("bombs-%0").arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNamePhotonBombs);
                BombFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNamePhotonBombs);
            }

            if (SimSet.RunSet.SaveStatistics)
            {
                WorkSet.RunSet.FileNameStatistics   = QString("stats-%0").arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameStatistics);
                StatisticsFiles.push_back(ExchangeDir + '/' + WorkSet.RunSet.FileNameStatistics);
            }
            if (SimSet.RunSet.SaveMonitors)
            {
                WorkSet.RunSet.FileNameMonitors     = QString("monitors-%0").arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameMonitors);
                MonitorFiles.push_back(ExchangeDir + '/' + WorkSet.RunSet.FileNameMonitors);
            }
*/

            QJsonObject json;
            WorkSet.writeToJson(json, true);
            QString ConfigFN = QString("config-%0.json").arg(iProcess);
            jstools::saveJsonToFile(json, ExchangeDir + '/' + ConfigFN);
            Worker.ConfigFile = ConfigFN;

            nc.Workers.push_back(Worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return true;
}

bool AParticleSimManager::configureGDML(A3WorkDistrConfig & Request, const QString & ExchangeDir)
{
    SimSet.RunSet.GDML = "detector.gdml";

    const QString LocalGdmlName = ExchangeDir + "/" + SimSet.RunSet.GDML.data();
    Request.CommonFiles.push_back(LocalGdmlName);
    QString err = Geometry.exportToGDML(LocalGdmlName);

    if (err.isEmpty()) return true;
    else
    {
        ErrorString = err;
        return false;
    }
}

bool AParticleSimManager::configureMonitors(A3WorkDistrConfig & Request, const QString & ExchangeDir)
{
    // !!!***
    //    MonitorFiles.clear();
    return true;
}

void AParticleSimManager::configureMaterials()
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    SimSet.RunSet.Materials         = MatHub.getMaterialNames();
    SimSet.RunSet.MaterialsFromNist = MatHub.getMaterialsFromNist();
}

#include "ageoobject.h"
void AParticleSimManager::generateG4antsConfigCommon(AParticleRunSettings  & RunSet, int ThreadIndex, QJsonObject & json)
{
//    const AG4SimulationSettings & G4SimSet = SimSet.G4Set;
/*
    bool bG4Primaries = false;
    bool bBinaryPrimaries = false;
    if (PartSimSet.GenerationMode == AParticleSimSettings::File)
    {
        bG4Primaries     = PartSimSet.FileGenSettings.isFormatG4();
        bBinaryPrimaries = PartSimSet.FileGenSettings.isFormatBinary();
    }
    json["Primaries_G4ants"] = bG4Primaries;
    json["Primaries_Binary"] = bBinaryPrimaries;
    QString primFN = QString("primaries-%1.txt").arg(ThreadIndex);
    json["File_Primaries"] = primFN;
//    removeOldFile(primFN, "primaries");
*/


//    QString depoFN = G4SimSet.getDepositionFileName(ThreadIndex);
//    json["File_Deposition"] = depoFN;
//    removeOldFile(depoFN, "deposition");

//    QString recFN = G4SimSet.getReceitFileName(ThreadIndex);
//    json["File_Receipt"] = recFN;
//    removeOldFile(recFN, "receipt");

//    QString tracFN = G4SimSet.getTracksFileName(ThreadIndex);
//    json["File_Tracks"] = tracFN;
//    removeOldFile(tracFN, "tracking");

//    QString monFeedbackFN = G4SimSet.getMonitorDataFileName(ThreadIndex);
//    json["File_Monitors"] = monFeedbackFN;
//    removeOldFile(monFeedbackFN, "monitor data");

/*
    const ASaveParticlesToFileSettings & ExitSimSet = GenSimSettings.ExitParticleSettings;
    QString exitParticleFN  = G4SimSet.getExitParticleFileName(ThreadIndex);
    QJsonObject jsExit;
    jsExit["Enabled" ]      = ExitSimSet.SaveParticles;
    jsExit["VolumeName"]    = ExitSimSet.VolumeName;
    jsExit["FileName"]      = exitParticleFN;
    jsExit["UseBinary"]     = ExitSimSet.UseBinary;
    jsExit["UseTimeWindow"] = ExitSimSet.UseTimeWindow;
    jsExit["TimeFrom"]      = ExitSimSet.TimeFrom;
    jsExit["TimeTo"]        = ExitSimSet.TimeTo;
    jsExit["StopTrack"]     = ExitSimSet.StopTrack;
    json["SaveExitParticles"] = jsExit;
*/

/*
    QJsonArray arMon;
    const QVector<const AGeoObject*> & MonitorsRecords = detector.Sandwich->MonitorsRecords;
    for (int iMon = 0; iMon <  MonitorsRecords.size(); iMon++)
    {
        const AGeoObject * obj = MonitorsRecords.at(iMon);
        const AMonitorConfig * mc = obj->getMonitorConfig();
        if (mc && mc->PhotonOrParticle == 1)
        {
            const QStringList ParticleList = MpCollection.getListOfParticleNames();
            const int particleIndex = mc->ParticleIndex;
            if ( particleIndex >= -1 && particleIndex < ParticleList.size() )
            {
                QJsonObject mjs;
                mc->writeToJson(mjs);
                mjs["Name"] = obj->Name + "_-_" + QString::number(iMon);
                mjs["ParticleName"] = ( particleIndex == -1 ? "" : ParticleList.at(particleIndex) );
                mjs["MonitorIndex"] = iMon;
                arMon.append(mjs);
            }
        }
    }
    json["Monitors"] = arMon;
*/
}

// ---

void AParticleSimManager::addErrorLine(const QString &error)
{
    if (error.isEmpty()) return;

    if (ErrorString.isEmpty()) ErrorString = error;
    else                       ErrorString += QString("\n%0").arg(error);
}

#include "a3global.h"
#include <QDir>
void AParticleSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.empty())                addErrorLine("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory.data()).exists()) addErrorLine("Output directory does not exist!");

    addErrorLine(A3Global::getInstance().checkExchangeDir());
}
