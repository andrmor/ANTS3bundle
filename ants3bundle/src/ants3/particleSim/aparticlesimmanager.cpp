#include "aparticlesimmanager.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "asourceparticlegenerator.h"
#include "afileparticlegenerator.h"
#include "ageometryhub.h"
#include "a3farmnoderecord.h"
#include "a3workdistrconfig.h"
#include "adispatcherinterface.h"
#include "a3global.h"
#include "arandomhub.h"
#include "ajsontools.h"
#include "aerrorhub.h"

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
    Generator_File    = new AFileParticleGenerator(SimSet.FileGenSettings);
}

AParticleSimManager::~AParticleSimManager()
{
    delete Generator_Sources;
    delete Generator_File;
}

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
    std::vector<A3FarmNodeRecord> RunPlan;
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

//    processReply(Reply);

    if (!AErrorHub::isError()) mergeOutput();
}

#include <QFile>
void AParticleSimManager::removeOutputFiles()
{
    qDebug() << "Removing (if exist) files with the names listed in output files";

    const QString OutputDir(SimSet.RunSet.OutputDirectory.data());
    std::vector<QString> fileNames;

    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNameTrackingHistory.data());
    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.SaveSettings.FileName.data());

    for (const QString & fn : fileNames)
        QFile::remove(fn);
}

void AParticleSimManager::doPreSimChecks()
{
    checkG4Settings();
    checkDirectories();

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

bool AParticleSimManager::configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "g4ants3";

    const QString & ExchangeDir = A3Global::getConstInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    bool ok = configureGDML(Request, ExchangeDir); if (!ok) return false;
    configureMaterials();
    configureMonitors();

    HistoryFileMerger.clear();
    DepositionFileMerger.clear();
    ParticlesFileMerger.clear();

    ARandomHub & RandomHub = ARandomHub::getInstance();
    RandomHub.setSeed(SimSet.RunSet.Seed);

    int iEvent = 0;
    int iProcess = 0;
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
                const QString fileName = QString("monitors-%0").arg(iProcess);
                WorkSet.RunSet.MonitorSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                //special rules for merging!
            }

            WorkSet.RunSet.Receipt = "receipt-" + std::to_string(iProcess) + ".txt";

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
    QString err = Geometry.exportToGDML(LocalGdmlName);

    // refactor !!!***
    if (err.isEmpty()) return true;
    else
    {
        AErrorHub::addError(err.toLatin1().data());
        return false;
    }
}

void AParticleSimManager::configureMaterials()
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    SimSet.RunSet.Materials         = MatHub.getMaterialNames();
    SimSet.RunSet.MaterialsFromNist = MatHub.getMaterialsFromNist();
}

void AParticleSimManager::configureMonitors()
{
    SimSet.RunSet.MonitorSettings.initFromHub();
}

// ---

#include "a3global.h"
#include <QDir>
void AParticleSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.empty())                AErrorHub::addError("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory.data()).exists()) AErrorHub::addError("Output directory does not exist!");

    AErrorHub::addError(A3Global::getInstance().checkExchangeDir());
}

//#include "amonitorhub.h"
void AParticleSimManager::mergeOutput()
{
    qDebug() << "Merging output files...";

    const QString & OutputDir(SimSet.RunSet.OutputDirectory.data());

    if (SimSet.RunSet.SaveTrackingHistory)
        HistoryFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameTrackingHistory.data());

    if (SimSet.RunSet.SaveDeposition)
        DepositionFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameDeposition.data());

    if (SimSet.RunSet.SaveSettings.Enabled)
        ParticlesFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.SaveSettings.FileName.data());

/*
    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    MonitorHub.clearData();
    if (SimSet.RunSet.SaveMonitors)
    {
        for (const QString & FN : MonitorFiles)
        {
            QJsonObject js;
            bool ok = jstools::loadJsonFromFile(js, FN);
            if (ok) MonitorHub.appendDataFromJson(js);
        }
        QJsonObject json;
        MonitorHub.writeDataToJson(json);
        jstools::saveJsonToFile(json, OutputDir + '/' + SimSet.RunSet.FileNameMonitors);
    }
*/
}

#include "atrackingdataimporter.h"
#include "aeventtrackingrecord.h"
//#include "atrackrecords.h"
//#include "atrackbuildoptions.h"
#include "TGeoTrack.h"
#include "TGeoManager.h"
#include "aparticletrackvisuals.h"

namespace
{
    void addTrack(const AParticleTrackingRecord * r,
                  bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
                  int & iTrack, int MaxTracks)
    {
        if (iTrack >= MaxTracks) return;

        const bool DoSkipPrim = SkipPrimaries || (SkipPrimNoInter && r->isNoInteractions());

        if (!DoSkipPrim)
        {
            TGeoTrack * track = new TGeoTrack(1, 22);

            track->SetLineColor(7);
            track->SetLineWidth(2);
            track->SetLineStyle(1);
//        AParticleTrackVisuals::getInstance().applyToParticleTrack(track, r->ParticleName);

            const std::vector<ATrackingStepData *> & Steps = r->getSteps();
            for (const ATrackingStepData * step : Steps)
            {
                if (step->Process != "T")
                    track->AddPoint(step->Position[0], step->Position[1], step->Position[2], step->Time);
            }
            gGeoManager->AddTrack(track);
            iTrack++;
        }

        if (!SkipSecondaries)
        {
            const std::vector<AParticleTrackingRecord *> & Secondaries = r->getSecondaries();
            for (AParticleTrackingRecord * sec : Secondaries)
                addTrack(sec, false, false, SkipSecondaries,  // already no primaries from this level down
                         iTrack, MaxTracks);
        }
    }
}

QString AParticleSimManager::buildTracks(const QString & fileName, const QStringList & LimitToParticles, const QStringList & ExcludeParticles,
                                         bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
                                         const int MaxTracks, int LimitToEvent)
{
    // binary or ascii !!!***
    bool bBinary = false;

    gGeoManager->ClearTracks();

    ATrackingDataImporter tdi(fileName, bBinary); // !!!*** make it persistent

    AEventTrackingRecord * record = AEventTrackingRecord::create();
    int iEvent = 0;
    int iTrack = 0;
    while (iTrack < MaxTracks)
    {
        bool ok = tdi.extractEvent(iEvent, record);
        iEvent++;

        if (!ok)
        {
            if (tdi.isEndReached()) return "";
            return tdi.ErrorString;
        }

        const std::vector<AParticleTrackingRecord *> Prims = record->getPrimaryParticleRecords();
        for (const AParticleTrackingRecord * r : Prims)
            addTrack(r,
                     SkipPrimaries, SkipPrimNoInter, SkipSecondaries,
                     iTrack, MaxTracks);
    }

    return "";
}

QString AParticleSimManager::fillTrackingRecord(const QString & fileName, int iEvent, AEventTrackingRecord * record)
{
    // binary or ascii !!!***
    bool bBinary = false;

    ATrackingDataImporter tdi(fileName, bBinary); // !!!*** make it persistent

    bool ok = tdi.extractEvent(iEvent, record);
    if (!ok) return tdi.ErrorString;
    return "";
}
