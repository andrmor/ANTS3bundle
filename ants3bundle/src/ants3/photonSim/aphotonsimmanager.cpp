#include "aphotonsimmanager.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "aconfig.h"
#include "a3global.h"
#include "adispatcherinterface.h"
#include "a3workdistrconfig.h"
#include "astatisticshub.h"
#include "afarmnoderecord.h"
#include "ajsontools.h"
#include "adepositionfilehandler.h"
#include "aphotonbombfilehandler.h"
#include "aphotonfilehandler.h"
#include "aerrorhub.h"
#include "anoderecord.h"
#include "adeporecord.h"
#include "aphoton.h"

#include <QDir>

#include <memory>

APhotonSimManager & APhotonSimManager::getInstance()
{
    static APhotonSimManager instance;
    return instance;
}

const APhotonSimManager & APhotonSimManager::getConstInstance()
{
    return getInstance();
}

void APhotonSimManager::setSeed(double seed)
{
    SimSet.RunSet.Seed = seed;
}

APhotonSimManager::APhotonSimManager() :
    SimSet(APhotonSimHub::getInstance().Settings) {}

bool APhotonSimManager::simulate(int numLocalProc)
{
    qDebug() << "Photon sim triggered";
    AErrorHub::clear();

    bool ok = checkDirectories();
    if (!ok) return false;

    ok = updateRuntimeForFunctionalModels();
    if (!ok) return false;

    removeOutputFiles();  // note that output files in exchange dir will be deleted in adispatcherinterface
    int numEvents = 0;
    switch (SimSet.SimType)
    {
    case EPhotSimType::PhotonBombs :
        switch (SimSet.BombSet.GenerationMode)
        {
        case EBombGen::Single :
            numEvents = 1;
            break;
        case EBombGen::Grid :
            numEvents = SimSet.BombSet.GridSettings.getNumEvents();
            break;
        case EBombGen::Flood :
            numEvents = SimSet.BombSet.FloodSettings.Number;
            break;
        case EBombGen::File :
            {
                if (!SimSet.BombSet.BombFileSettings.isValidated())
                {
                    APhotonBombFileHandler bh(SimSet.BombSet.BombFileSettings);
                    bool ok = bh.checkFile();
                    if (!ok) return false;
                }
                numEvents = SimSet.BombSet.BombFileSettings.NumEvents;
                // !!!*** add possibility to limit to a given number of events!
            }
            break;
        default:
            AErrorHub::addError("This bomb generation mode is not implemented yet!");
            return false;
        }
        break;
    case EPhotSimType::FromEnergyDepo :
        {
            if (!SimSet.DepoSet.isValidated())
            {
                ADepositionFileHandler fh(SimSet.DepoSet);
                bool ok = fh.checkFile();
                if (!ok) return false;
            }
            numEvents = SimSet.DepoSet.NumEvents;
            // !!!*** add possibility to limit to a given number of events!
            //qDebug() << "---From depo, num events:" << numEvents << AErrorHub::isError();
        }
        break;
    case EPhotSimType::IndividualPhotons :
        {
            if (!SimSet.PhotFileSet.isValidated())
            {
                APhotonFileHandler fh(SimSet.PhotFileSet);
                bool ok = fh.checkFile();
                if (!ok) return false;
            }
            numEvents = SimSet.PhotFileSet.NumEvents;
            // !!!*** add possibility to limit to a given number of events!
        }
        break;
    case EPhotSimType::FromLRFs :
        {
            // TODO direct calculation here! !!!***
            AErrorHub::addError("This simulation mode is not implemented yet!");
            return false;
        }
        break;
    default:
        AErrorHub::addError("This simulation mode is not implemented yet!");
        return false;
    }

    if (numEvents == 0)
    {
        AErrorHub::addError("Nothing to simulate (no events)");
        return false;
    }

    // configure number of local/remote processes to run
    std::vector<AFarmNodeRecord> RunPlan;
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QString err = Dispatcher.fillRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty()) return false;

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;
    ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    qDebug() << "Running simulation...";
    QJsonObject Reply = Dispatcher.performTask(Request);

    qDebug() << "\n\n---------------------";
    qDebug() << Reply;
    qDebug() << "---------------------\n\n";

    processReply(Reply);

    qDebug() << "Photon simulation finished";

    if (AErrorHub::isError()) return false;

    mergeOutput();
    return !AErrorHub::isError();
}

bool APhotonSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.isEmpty())       AErrorHub::addError("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory).exists()) AErrorHub::addError("Output directory does not exist!");

    A3Global::getInstance().checkExchangeDir();

    return !AErrorHub::isError();
}

#include "aphotonfunctionalhub.h"
bool APhotonSimManager::updateRuntimeForFunctionalModels()
{
    APhotonFunctionalHub & PhTunHub = APhotonFunctionalHub::getInstance();
    return PhTunHub.updateRuntimeProperties();
}

void APhotonSimManager::processReply(const QJsonObject & json)
{
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

/*
    qDebug() << "Reply message:" << json;
    QString Err;
    jstools::parseJson(json, "Error", Err);
    if (!Err.isEmpty())
    {
        AErrorHub::addQError(Err);
        return;
    }

    bool bSuccess = false;
    jstools::parseJson(json, "Success", bSuccess);
    if (bSuccess) return; // success

    AErrorHub::addError("Bad reply of the dispatcher: Fail status but no error text");
*/
}

void removePhotonOutputFiles(const APhotSimRunSettings & settings)
{
    const QString & OutputDir = settings.OutputDirectory;

    std::vector<QString> fileNames;
    fileNames.push_back(OutputDir + '/' + settings.FileNameSensorSignals);
    fileNames.push_back(OutputDir + '/' + settings.FileNameTracks);
    fileNames.push_back(OutputDir + '/' + settings.PhotonLogSet.FileName);
    fileNames.push_back(OutputDir + '/' + settings.FileNamePhotonBombs);
    fileNames.push_back(OutputDir + '/' + settings.FileNameStatistics);
    fileNames.push_back(OutputDir + '/' + settings.FileNameMonitors);
    fileNames.push_back(OutputDir + '/' + settings.FileNameReceipt);

    for (const QString & fn : fileNames) QFile::remove(fn);
}

void APhotonSimManager::removeOutputFiles()
{
    qDebug() << "Removing (if exist) output files with conflicting names";
    APhotSimRunSettings defaultSettings;
    defaultSettings.OutputDirectory = SimSet.RunSet.OutputDirectory;

    removePhotonOutputFiles(SimSet.RunSet);
    removePhotonOutputFiles(defaultSettings); // to make sure the files with the default names are removed, in case some customization was done
}

#include "amonitorhub.h"
#include "aerrorhub.h"
void APhotonSimManager::mergeOutput()
{
    qDebug() << "Merging output files...";

    const QString & OutputDir = SimSet.RunSet.OutputDirectory;
    if (SimSet.RunSet.SaveSensorSignals) SignalFileMerger   .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameSensorSignals);
    if (SimSet.RunSet.SaveSensorLog)     SensorLogFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameSensorLog);
    if (SimSet.RunSet.PhotonLogSet.Save) PhotonLogFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.PhotonLogSet.FileName);
    if (SimSet.RunSet.SaveTracks)        TrackFileMerger    .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameTracks);
    if (SimSet.RunSet.SavePhotonBombs)   BombFileMerger     .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNamePhotonBombs);

    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    Stat.clear();
    if (SimSet.RunSet.SaveStatistics)
    {
        for (const QString & FN : StatisticsFiles)
        {
            QJsonObject json;
            bool ok = jstools::loadJsonFromFile(json, FN);
            if (ok)
            {
                APhotonStatistics thisStat;
                thisStat.readFromJson(json);
                Stat.append(thisStat);
            }
        }
        QJsonObject json;
        Stat.writeToJson(json);
        jstools::saveJsonToFile(json, OutputDir + '/' + SimSet.RunSet.FileNameStatistics);
    }

    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    MonitorHub.clearData(AMonitorHub::Photon);
    if (SimSet.RunSet.SaveMonitors)
        MonitorHub.mergePhotonMonitorFiles(MonitorFiles, OutputDir + '/' + SimSet.RunSet.FileNameMonitors);

    qDebug() << "Done!";
}

void  APhotonSimManager::clearFileMergers()
{
    SignalFileMerger.clear();
    SensorLogFileMerger.clear();
    PhotonLogFileMerger.clear();
    TrackFileMerger.clear();
    BombFileMerger.clear();
    StatisticsFiles.clear();
    MonitorFiles.clear();
    ReceiptFiles.clear();
}

std::vector<int> computeNumberSplit(int numberToSplit, size_t nodes)
{
    std::vector<int> res(nodes, 0);

    int perOne = std::ceil(1.0 * numberToSplit / nodes);
    if (perOne < 1) perOne = 1;
    if (perOne > numberToSplit) perOne = numberToSplit;

    int added = 0;
    for (size_t i = 0; i < nodes; i++)
    {
        res[i] = perOne;
        added += perOne;
        if (added >= numberToSplit) break;
    }

    return res;
}

bool APhotonSimManager::configureSimulation(const std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    qDebug() << "Configuring simulation...";
    Request.Command = "lsim"; // name of the corresponding executable
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    clearFileMergers();

    std::unique_ptr<AFileHandlerBase> InFileHandler;
    AFileHandlerBase * ptr = makeInputFileHandler();
    if (ptr)
    {
        InFileHandler = std::unique_ptr<AFileHandlerBase>(ptr);
        if (!InFileHandler->init()) return false;
    }

    ARandomHub & RandomHub = ARandomHub::getInstance();
    RandomHub.setSeed(SimSet.RunSet.Seed);

    int iEvent = 0;
    int iProcess = 0;

    size_t numProcesses = 0;
    for (const AFarmNodeRecord & r : RunPlan) numProcesses += r.Split.size();
    std::vector<int> tracksToAdd = computeNumberSplit(SimSet.RunSet.MaxTracks, numProcesses);
    std::vector<int> logsToAdd   = computeNumberSplit(SimSet.RunSet.PhotonLogSet.MaxNumber, numProcesses);

    for (const AFarmNodeRecord & r : RunPlan)   // per node server
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)            // per process at the node server
        {
            if (num == 0) break;

            A3NodeWorkerConfig Worker;
            APhotonSimSettings WorkSet = SimSet;
            WorkSet.RunSet.Seed      = INT_MAX * RandomHub.uniform();
            WorkSet.RunSet.EventFrom = iEvent;
            WorkSet.RunSet.EventTo   = iEvent + num;
            iEvent += num;

            switch (SimSet.SimType)
            {
            case EPhotSimType::PhotonBombs :
                switch (SimSet.BombSet.GenerationMode)
                {
                case EBombGen::Single :
                    // TODO: for single bomb, try to split between processes by photons to! But needs custom merger!
                    break;
                case EBombGen::Grid :
                    break;
                case EBombGen::Flood :
                    break;
                case EBombGen::File :
                    WorkSet.BombSet.BombFileSettings.NumEvents = num;
                    WorkSet.BombSet.BombFileSettings.FileName = QString("inBombs-%0").arg(iProcess);
                    QString localFileName = ExchangeDir + '/' + WorkSet.BombSet.BombFileSettings.FileName;
                    ANodeRecord buffer;
                    bool ok = InFileHandler->copyToFileBuffered(WorkSet.RunSet.EventFrom, WorkSet.RunSet.EventTo, localFileName, buffer);
                    if (!ok) return false;
                    WorkSet.BombSet.BombFileSettings.LastModified = QFileInfo(localFileName).lastModified();
                    Worker.InputFiles.push_back(localFileName);
                    break;
                }
                break;
            case EPhotSimType::FromEnergyDepo :
                {
                    WorkSet.DepoSet.NumEvents = num;
                    WorkSet.DepoSet.FileName = QString("inDepo-%0").arg(iProcess);
                    QString localFileName = ExchangeDir + '/' + WorkSet.DepoSet.FileName;
                    ADepoRecord buffer;
                    bool ok = InFileHandler->copyToFileBuffered(WorkSet.RunSet.EventFrom, WorkSet.RunSet.EventTo, localFileName, buffer);
                    if (!ok) return false;
                    WorkSet.DepoSet.LastModified = QFileInfo(localFileName).lastModified();
                    Worker.InputFiles.push_back(localFileName);
                }
                break;
            case EPhotSimType::IndividualPhotons :
                {
                    WorkSet.PhotFileSet.NumEvents = num;
                    WorkSet.PhotFileSet.FileName = QString("inPhotons-%0").arg(iProcess);
                    QString localFileName = ExchangeDir + '/' + WorkSet.PhotFileSet.FileName;
                    APhoton buffer;
                    bool ok = InFileHandler->copyToFileBuffered(WorkSet.RunSet.EventFrom, WorkSet.RunSet.EventTo, localFileName, buffer);
                    if (!ok) return false;
                    WorkSet.PhotFileSet.LastModified = QFileInfo(localFileName).lastModified();
                    Worker.InputFiles.push_back(localFileName);
                }
                break;
            case EPhotSimType::FromLRFs :
                AErrorHub::addError("\"From LRFs\" sim mode is not handled this way!");
                return false;
            }

            // max tracks and logs per node
            WorkSet.RunSet.MaxTracks = tracksToAdd[iProcess];
            WorkSet.RunSet.PhotonLogSet.MaxNumber = logsToAdd[iProcess];

            configureOutputFiles(Worker, WorkSet, iProcess);
            makeWorkerConfigFile(Worker, WorkSet, iProcess);
            nc.Workers.push_back(Worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return !AErrorHub::isError();
}

AFileHandlerBase * APhotonSimManager::makeInputFileHandler()
{
    if (SimSet.SimType == EPhotSimType::PhotonBombs && SimSet.BombSet.GenerationMode == EBombGen::File)
        return new APhotonBombFileHandler(SimSet.BombSet.BombFileSettings);

    if (SimSet.SimType == EPhotSimType::FromEnergyDepo)
        return new ADepositionFileHandler(SimSet.DepoSet);

    if (SimSet.SimType == EPhotSimType::IndividualPhotons)
        return new APhotonFileHandler(SimSet.PhotFileSet);

    return nullptr;
}

void APhotonSimManager::configureOutputFiles(A3NodeWorkerConfig & Worker, APhotonSimSettings & WorkSet, int iProcess)
{
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;

    if (SimSet.RunSet.SaveSensorSignals)
    {
        WorkSet.RunSet.FileNameSensorSignals = QString("signals-%0").arg(iProcess);
        Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameSensorSignals);
        SignalFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameSensorSignals);
    }
    if (SimSet.RunSet.SaveSensorLog)
    {
        WorkSet.RunSet.FileNameSensorLog = QString("sensorlog-%0").arg(iProcess);
        Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameSensorLog);
        SensorLogFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameSensorLog);
    }
    if (SimSet.RunSet.PhotonLogSet.Save)
    {
        WorkSet.RunSet.PhotonLogSet.FileName = QString("photonlog-%0").arg(iProcess);
        Worker.OutputFiles.push_back(WorkSet.RunSet.PhotonLogSet.FileName);
        PhotonLogFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.PhotonLogSet.FileName);
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

    {
        WorkSet.RunSet.FileNameReceipt = QString("receiptOpt-%0.txt").arg(iProcess);
        Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameReceipt);
        ReceiptFiles.push_back(ExchangeDir + '/' + WorkSet.RunSet.FileNameReceipt);
    }
}

void APhotonSimManager::makeWorkerConfigFile(A3NodeWorkerConfig & Worker, APhotonSimSettings & WorkSet, int iProcess)
{
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;

    QJsonObject json;
    AConfig::getInstance().writeToJson(json, true);
    WorkSet.writeToJson(json, true);
    QString ConfigFN = QString("config-%0.json").arg(iProcess);
    jstools::saveJsonToFile(json, ExchangeDir + '/' + ConfigFN);
    Worker.ConfigFile = ConfigFN;
}
