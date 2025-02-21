#include "aparticlesimmanager.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "asourceparticlegenerator.h"
#include "afileparticlegenerator.h"
#include "ageometryhub.h"
#include "afarmnoderecord.h"
#include "a3workdistrconfig.h"
#include "adispatcherinterface.h"
#include "a3global.h"
#include "arandomhub.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>
#include <QFile>

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
    Generator_File    = new AFileParticleGenerator(SimSet.FileGenSettings);
}

AParticleSimManager::~AParticleSimManager()
{
    delete Generator_Sources;
    delete Generator_File;
}

#include "aconfig.h"
void AParticleSimManager::simulate(int numLocalProc)
{
    AErrorHub::clear();

    doPreSimChecks();
    if (AErrorHub::isError()) return;

    bool ok = configureParticleGun();
    if (!ok) return;

    const int numEvents = getNumberEvents();
    if (numEvents == 0)
    {
        AErrorHub::addError("Configuration reports zero events to simulate");
        return;
    }

    removeOutputFiles();  // note that output files in exchange dir will be deleted in adispatcherinterface

    // configure number of local/remote processes to run
    std::vector<AFarmNodeRecord> RunPlan;
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QString err = Dispatcher.fillRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        AErrorHub::addError(err.toLatin1().data());
        return;
    }

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;

    ok = configureSimulation(RunPlan, Request);
    if (!ok) return;

    qDebug() << "Running simulation...";
    QJsonObject Reply = Dispatcher.performTask(Request);

    processReply(Reply);

    if (!AErrorHub::isError())
    {
        mergeOutput(!SimSet.RunSet.AsciiOutput);
        if (SimSet.RunSet.SaveConfig) jstools::saveJsonToFile( AConfig::getConstInstance().JSON,
                                                               QString(SimSet.RunSet.OutputDirectory.data()) + "/" + QString(SimSet.RunSet.FileNameSaveConfig.data()) );
    }
}

void AParticleSimManager::abort()
{
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    Dispatcher.abortTask();
}

bool AParticleSimManager::isAborted() const
{
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    return Dispatcher.isAborted();
}

void AParticleSimManager::processReply(const QJsonObject & Reply)
{
    //qDebug() << "<<<<<<<--------------->>>>>>>>";
    //qDebug() << Reply;

    QString LastError;

    for (size_t iFile = 0; iFile < ReceiptFiles.size(); iFile++)
    {
        const QString & fn = ReceiptFiles[iFile];
        //qDebug() << fn;
        if (!QFile::exists(fn))
        {
            AErrorHub::addQError(QString("Receipt file was not found for worker index %0").arg(iFile)); // make more human-readble for large numbers
        }
        else
        {
            QJsonObject ReceiptJson;
            bool ok = jstools::loadJsonFromFile(ReceiptJson, fn);
            if (!ok)
            {
                AErrorHub::addQError(QString("Cannot load json from receipt file for worker index %0").arg(iFile));
                continue;
            }

            QString Error;
            bool bSuccess = false;
            ok = jstools::parseJson(ReceiptJson, "Success", bSuccess);
            if (!ok || !bSuccess)
            {
                jstools::parseJson(ReceiptJson, "Error", Error);
                if (Error.isEmpty()) AErrorHub::addQError("Unknown error!");
                else
                {
                    if (Error != LastError)
                    {
                        AErrorHub::addQError(Error);
                        LastError = Error;
                    }
                }
                continue;
            }
        }
    }
}

void addNames(const AParticleRunSettings & settings)
{
    const QString OutputDir(settings.OutputDirectory.data());

    std::vector<QString> fileNames;
    fileNames.push_back(OutputDir + '/' + settings.FileNameTrackingHistory.data());
    fileNames.push_back(OutputDir + '/' + settings.FileNameDeposition.data());
    fileNames.push_back(OutputDir + '/' + settings.SaveSettings.FileName.data());
    fileNames.push_back(OutputDir + '/' + settings.MonitorSettings.FileName.data());
    fileNames.push_back(OutputDir + '/' + settings.CalorimeterSettings.FileName.data());
    fileNames.push_back(OutputDir + '/' + settings.Receipt.data());
    fileNames.push_back(OutputDir + '/' + settings.FileNameSaveConfig.data());

    for (const QString & fn : fileNames) QFile::remove(fn);
}

void AParticleSimManager::removeOutputFiles()
{
    qDebug() << "Removing (if exist) output files with conflicting names";
    AParticleRunSettings defaultSettings;
    defaultSettings.OutputDirectory = SimSet.RunSet.OutputDirectory;

    addNames(SimSet.RunSet);
    addNames(defaultSettings); // to make sure the files with the default names are removed, in case some customization was done
}

void AParticleSimManager::doPreSimChecks()
{
    checkG4Settings();
    checkDirectories();
    Geometry.checkGeometryCompatibleWithGeant4();

    // !!!*** refactor
    QString err;
    err = Geometry.checkVolumesExist(SimSet.G4Set.SensitiveVolumes);
    if (!err.isEmpty()) AErrorHub::addError(err.toLatin1().data());
}

#include "amaterialhub.h"
void AParticleSimManager::checkG4Settings()
{
    // TODO: no sensitive volumes but photon sim scheduled

    // TODO: optical grids will not be expanded

    //reformat !!!***
    QString Error;
    AMaterialHub::getInstance().checkReadyForGeant4Sim(Error);
    if (!Error.isEmpty()) AErrorHub::addError(Error.toLatin1().data());
}

int AParticleSimManager::getNumberEvents() const
{
    int numEvents = SimSet.Events;
    if (SimSet.GenerationMode == AParticleSimSettings::File)
        numEvents = std::min(numEvents, SimSet.FileGenSettings.NumEvents);
    return numEvents;
}

bool AParticleSimManager::configureSimulation(const std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "g4ants3";

    const QString & ExchangeDir = A3Global::getConstInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    bool ok = configureGDML(Request, ExchangeDir); if (!ok) return false;
    configureMaterials();
    configureMonitors();
    configureCalorimeters();
    configureAnalyzers();
    configureScintillators();

    HistoryFileMerger.clear();
    DepositionFileMerger.clear();
    ParticlesFileMerger.clear();
    MonitorFiles.clear();
    CalorimeterFiles.clear();
    AnalyzerFiles.clear();
    ReceiptFiles.clear();

    ARandomHub & RandomHub = ARandomHub::getInstance();
    RandomHub.setSeed(SimSet.RunSet.Seed);

    int iEvent = 0;
    int iProcess = 0;
    for (const AFarmNodeRecord & r : RunPlan)   // per node server
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)            // per process at the node server
        {
            if (num == 0) break;

            A3NodeWorkerConfig Worker;
            AParticleSimSettings WorkSet;

            WorkSet = SimSet;
            WorkSet.RunSet.EventFrom = iEvent;
            WorkSet.RunSet.EventTo   = iEvent + num;
            iEvent += num;

            switch (SimSet.GenerationMode)
            {
            case AParticleSimSettings::Sources :
                // nothing to do
                break;
            case AParticleSimSettings::File :
                {
                    ParticleGun->setStartEvent(iEvent);
                    const QString fileName = QString("primaries-%0").arg(iProcess);
                    WorkSet.FileGenSettings.FileName = fileName.toLatin1().data();
                    //refactor to avoid scanning from the beginning every time !!!***
                    static_cast<AFileParticleGenerator*>(ParticleGun)->generateG4File(WorkSet.RunSet.EventFrom, WorkSet.RunSet.EventTo, (ExchangeDir + '/' + fileName).toLatin1().data());
                    Worker.InputFiles.push_back(fileName);
                }
                break;
            default:
                AErrorHub::addError("This generation mode is not yet implemented!");
                return false;
            }

            WorkSet.RunSet.Seed = INT_MAX * RandomHub.uniform();

            if (SimSet.RunSet.SaveTrackingHistory)
            {
                const QString fileName = QString("history-%0").arg(iProcess);
                WorkSet.RunSet.FileNameTrackingHistory = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                HistoryFileMerger.add(ExchangeDir + '/' + fileName);
            }

            if (SimSet.RunSet.SaveDeposition)
            {
                const QString fileName = QString("depo-%0").arg(iProcess);
                WorkSet.RunSet.FileNameDeposition = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                DepositionFileMerger.add(ExchangeDir + '/' + fileName);
            }

            if (SimSet.RunSet.SaveSettings.Enabled)
            {
                const QString fileName = QString("particles-%0").arg(iProcess);
                WorkSet.RunSet.SaveSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                ParticlesFileMerger.add(ExchangeDir + '/' + fileName);
            }

            if (SimSet.RunSet.MonitorSettings.Enabled)
            {
                const QString fileName = QString("particleMonitors-%0").arg(iProcess);
                WorkSet.RunSet.MonitorSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                MonitorFiles.push_back(ExchangeDir + '/' + fileName);
            }

            if (SimSet.RunSet.CalorimeterSettings.Enabled)
            {
                const QString fileName = QString("calorimters-%0").arg(iProcess);
                WorkSet.RunSet.CalorimeterSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                CalorimeterFiles.push_back(ExchangeDir + '/' + fileName);
            }

            if (SimSet.RunSet.AnalyzerSettings.Enabled)
            {
                const QString fileName = QString("particleAnalyzers-%0").arg(iProcess);
                WorkSet.RunSet.AnalyzerSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                AnalyzerFiles.push_back(ExchangeDir + '/' + fileName);
            }

            {
                const QString fileName = QString("receipt-%0.txt").arg(iProcess);
                WorkSet.RunSet.Receipt = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                ReceiptFiles.push_back(ExchangeDir + '/' + fileName);
            }

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

bool AParticleSimManager::configureParticleGun()
{
    ParticleGun = nullptr;

    switch (SimSet.GenerationMode)
    {
    case AParticleSimSettings::Sources :
        ParticleGun = Generator_Sources;
        break;
    case AParticleSimSettings::File :
        ParticleGun = Generator_File;
        if (!SimSet.FileGenSettings.isValidated())
        {
            Generator_File->checkFile(false);
            if (AErrorHub::isError()) return false;
        }
        break;
    case AParticleSimSettings::Script :
        //break;
    default:;
    }

    if (!ParticleGun)
    {
        AErrorHub::addError("Unknown or not implemented particle generation mode");
        return false;
    }

    return ParticleGun->init();
}

bool AParticleSimManager::configureGDML(A3WorkDistrConfig & Request, const QString & ExchangeDir)
{
    SimSet.RunSet.GDML = "detector.gdml";

    const QString LocalGdmlName = ExchangeDir + "/" + SimSet.RunSet.GDML.data();
    Request.CommonFiles.push_back(LocalGdmlName);
    //QString err = Geometry.exportToGDML(LocalGdmlName);
    QString err = AGeometryHub::getInstance().exportGeometry(LocalGdmlName);

    if (err.isEmpty()) return true;
    else
    {
        AErrorHub::addQError(err);
        return false;
    }
}

void AParticleSimManager::configureMaterials()
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    SimSet.RunSet.Materials             = MatHub.getMaterialNames();
    SimSet.RunSet.MaterialsFromNist     = MatHub.getMaterialsFromNist();
    SimSet.RunSet.MaterialsMeanExEnergy = MatHub.getMaterialsMeanExEnergy();
}

void AParticleSimManager::configureMonitors()
{
    SimSet.RunSet.MonitorSettings.initFromHub();
}

void AParticleSimManager::configureCalorimeters()
{
    SimSet.RunSet.CalorimeterSettings.initFromHub();
}

void AParticleSimManager::configureAnalyzers()
{
    SimSet.RunSet.AnalyzerSettings.initFromHub();
}

void AParticleSimManager::configureScintillators()
{
    SimSet.G4Set.ScintSensitiveVolumes.clear();
    if (SimSet.G4Set.AddScintillatorsToSensitiveVolumes)
    {
        std::vector<QString> vol;
        Geometry.getScintillatorVolumeUniqueNames(vol);

        for (const QString & name : vol)
            SimSet.G4Set.ScintSensitiveVolumes.emplace_back(name.toLatin1().data());
    }
}

// ---

#include "a3global.h"
#include <QDir>
void AParticleSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.empty())                AErrorHub::addError("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory.data()).exists()) AErrorHub::addError("Output directory does not exist!");

    A3Global::getInstance().checkExchangeDir();
}

#include "amonitorhub.h"
#include "acalorimeterhub.h"
#include "aparticleanalyzerhub.h"
void AParticleSimManager::mergeOutput(bool binary)
{
    qDebug() << "Merging output files...";

    const QString & OutputDir(SimSet.RunSet.OutputDirectory.data());

    if (SimSet.RunSet.SaveTrackingHistory)
        HistoryFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameTrackingHistory.data(), binary);

    if (SimSet.RunSet.SaveDeposition)
        DepositionFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameDeposition.data(), binary);

    if (SimSet.RunSet.SaveSettings.Enabled)
        ParticlesFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.SaveSettings.FileName.data(), binary);

    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    //MonitorHub.clearData(AMonitorHub::Particle);
    if (SimSet.RunSet.MonitorSettings.Enabled)
        MonitorHub.mergeParticleMonitorFiles(MonitorFiles, OutputDir + '/' + SimSet.RunSet.MonitorSettings.FileName.data());

    ACalorimeterHub & CalHub = ACalorimeterHub::getInstance();
    if (SimSet.RunSet.CalorimeterSettings.Enabled)
        CalHub.mergeCalorimeterFiles(CalorimeterFiles, OutputDir + '/' + SimSet.RunSet.CalorimeterSettings.FileName.data());

    AParticleAnalyzerHub & AnHub = AParticleAnalyzerHub::getInstance();
    if (SimSet.RunSet.AnalyzerSettings.Enabled)
    {
        AnHub.loadAnalyzerFiles(AnalyzerFiles);
        AnHub.saveAnalyzerData(OutputDir + '/' + SimSet.RunSet.AnalyzerSettings.FileName.data());
    }
}
