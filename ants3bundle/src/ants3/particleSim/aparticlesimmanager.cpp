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

    bool ok = configureGDML(Request, ExchangeDir);
    if (!ok) return false;

    ok = configureMonitors(Request, ExchangeDir); // !!!***
    if (!ok) return false;

    configureMaterials();

    HistoryFileMerger.clear();
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
                    WorkSet.FileGenSettings.setFileName(fileName.toLatin1().data());
                    qDebug() << "-----------------" << iProcess << fileName;
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

            if (SimSet.RunSet.SaveSettings.Enabled)
            {
                const QString fileName = QString("particles-%0").arg(iProcess);
                WorkSet.RunSet.SaveSettings.FileName = fileName.toLatin1().data();
                Worker.OutputFiles.push_back(fileName);
                ParticlesFileMerger.add(ExchangeDir + '/' + fileName);
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
