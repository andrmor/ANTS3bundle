#include "aparticlesimmanager.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "asourceparticlegenerator.h"
#include "ageometryhub.h"

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
    Generator_Sources = new ASourceParticleGenerator();
}

AParticleSimManager::~AParticleSimManager()
{
    delete Generator_Sources;
}

#include "a3farmnoderecord.h"
#include "a3workdistrconfig.h"
#include "adispatcherinterface.h"
#include "a3global.h"
bool AParticleSimManager::simulate(int numLocalProc)
{
    qDebug() << "Particle sim triggered";
    ErrorString.clear();

    checkG4Settings();
    checkDirectories();
    addErrorLine(Geometry.checkVolumesExist(SimSet.G4Set.SensitiveVolumes));
    if (!ErrorString.isEmpty()) return false;

//    removeOutputFiles();  // note that output files in exchange dir will be deleted in adispatcherinterface

    int numEvents = SimSet.Events;
    if (SimSet.GenerationMode == AParticleSimSettings::File)
    {
        //limit to max in file
    }
    if (numEvents == 0)
    {
        ErrorString = "Nothing to simulate!";
        return false;
    }

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

    qDebug() << "Particle simulation finished";
    return ErrorString.isEmpty();
}

#include "amaterialhub.h"
void AParticleSimManager::checkG4Settings()
{
    // TODO: no sensitive volumes but photon sim scheduled

    // TODO: optical grids will not be expanded

    AMaterialHub::getInstance().checkReadyForGeant4Sim(ErrorString);
}

#include "arandomhub.h"
#include "ajsontools.h"
bool AParticleSimManager::configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "g4ants3";

    const QString ExchangeDir = A3Global::getConstInstance().ExchangeDir;

    const QString LocalGdmlName = ExchangeDir + '/' +SimSet.RunSet.getGdmlFileName();
    QString err = Geometry.exportToGDML(LocalGdmlName);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }
    Request.CommonFiles.push_back(LocalGdmlName);

//  ...
//    MonitorFiles.clear();

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

/*
            QJsonObject json;
            A3Config::getInstance().writeToJson(json);
            WorkSet.writeToJson(json);
*/
            QString ConfigFN = QString("config-%0.json").arg(iProcess);
//            jstools::saveJsonToFile(json, ExchangeDir + '/' + ConfigFN);
            Worker.ConfigFile = ConfigFN;

            nc.Workers.push_back(Worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return true;
}

#include "ageoobject.h"
void AParticleSimManager::generateG4antsConfigCommon(const AParticleRunSettings  & RunSet, int ThreadIndex, QJsonObject & json)
{
    const AG4SimulationSettings & G4SimSet = SimSet.G4Set;

    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    json["PhysicsList"] = G4SimSet.PhysicsList;

    json["LogHistory"] = RunSet.SaveTrackingData;

/*
    QJsonArray Parr;
    const int numPart = MpCollection.countParticles();
    for (int iP=0; iP<numPart; iP++)
    {
        const AParticle * part = MpCollection.getParticle(iP);
        if (part->isIon())
        {
            QJsonArray ar;
            ar << part->ParticleName << part->ionZ << part->ionA;
            Parr << ar;
        }
        else Parr << part->ParticleName;
    }
    json["Particles"] = Parr;
*/

    const QStringList Materials = MatHub.getListOfMaterialNames();
    QJsonArray Marr;
    for (auto & mname : Materials ) Marr << mname;
    json["Materials"] = Marr;

    QJsonArray OverrideMats;
    for (int iMat = 0; iMat < MatHub.countMaterials(); iMat++)
    {
        const AMaterial * mat = MatHub[iMat];
        if (mat->bG4UseNistMaterial)
        {
            QJsonArray el; el << mat->name << mat->G4NistMaterial;
            OverrideMats << el;
        }
    }
    if (!OverrideMats.isEmpty()) json["MaterialsToRebuild"] = OverrideMats;

    json["ActivateThermalScattering"] = G4SimSet.UseTSphys;

    QJsonArray SVarr;
    for (auto & v : G4SimSet.SensitiveVolumes ) SVarr << v;
    json["SensitiveVolumes"] = SVarr;

    json["GDML"] = RunSet.getGdmlFileName();

    QJsonArray arSL;
    for (auto & key : G4SimSet.StepLimits.keys())
    {
        QJsonArray el;
        el << key << G4SimSet.StepLimits.value(key);
        arSL.push_back(el);
    }
    json["StepLimits"] = arSL;

    QJsonArray Carr;
    for (auto & c : G4SimSet.Commands ) Carr << c;
    json["Commands"] = Carr;

    json["GuiMode"] = false;

    json["Seed"] = RunSet.Seed;

    bool bG4Primaries = false;
    bool bBinaryPrimaries = false;
/*
    if (PartSimSet.GenerationMode == AParticleSimSettings::File)
    {
        bG4Primaries     = PartSimSet.FileGenSettings.isFormatG4();
        bBinaryPrimaries = PartSimSet.FileGenSettings.isFormatBinary();
    }
*/
    json["Primaries_G4ants"] = bG4Primaries;
    json["Primaries_Binary"] = bBinaryPrimaries;

    QString primFN = RunSet.getPrimariesFileName(ThreadIndex);
    json["File_Primaries"] = primFN;
//    removeOldFile(primFN, "primaries");

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

    json["BinaryOutput"] = !RunSet.AsciiOutput;
    json["Precision"]    = RunSet.AsciiPrecision;

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
    if (SimSet.RunSet.OutputDirectory.isEmpty())       addErrorLine("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory).exists()) addErrorLine("Output directory does not exist!");

    addErrorLine(A3Global::getInstance().checkExchangeDir());
}
