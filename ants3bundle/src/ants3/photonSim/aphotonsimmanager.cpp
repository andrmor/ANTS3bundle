#include "aphotonsimmanager.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "a3config.h"
#include "a3global.h"
#include "adispatcherinterface.h"
#include "a3workdistrconfig.h"
#include "astatisticshub.h"
#include "a3farmnoderecord.h"
#include "ajsontools.h"

#include <QDir>

APhotonSimManager & APhotonSimManager::getInstance()
{
    static APhotonSimManager instance;
    return instance;
}

const APhotonSimManager & APhotonSimManager::getConstInstance()
{
    return getInstance();
}

APhotonSimManager::APhotonSimManager() :
    SimSet(APhotonSimHub::getInstance().Settings)
{}

bool APhotonSimManager::simulate(int numLocalProc)
{
    qDebug() << "Photon sim triggered";
    ErrorString.clear();

    bool ok = checkDirectories();
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
        case EBombGen::Flood :
            numEvents = SimSet.BombSet.FloodSettings.Number;
            break;
        default:
            ErrorString = "This bomb generation mode is not implemented yet!";
            return false;
        }
        break;
    case EPhotSimType::FromLRFs :
        {
            // TODO direct calculation here! !!!***
            ErrorString = "This simulation mode is not implemented yet!";
            return false;
        }
    default:
        ErrorString = "This simulation mode is not implemented yet!";
        return false;
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
    ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    qDebug() << "Running simulation...";
    QJsonObject Reply = Dispatcher.performTask(Request);

    processReply(Reply);

    if (ErrorString.isEmpty()) mergeOutput();

    qDebug() << "Photon simulation finished";
    return ErrorString.isEmpty();
}

bool APhotonSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.isEmpty())       addErrorLine("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory).exists()) addErrorLine("Output directory does not exist!");

    addErrorLine(A3Global::getInstance().checkExchangeDir());

    return ErrorString.isEmpty();
}

void APhotonSimManager::processReply(const QJsonObject & json)
{
     qDebug() << "Reply message:" << json;

    jstools::parseJson(json, "Error", ErrorString);
    if (!ErrorString.isEmpty()) return; // error

    bool bSuccess = false;
    jstools::parseJson(json, "Success", bSuccess);
    if (bSuccess) return; // success

    ErrorString = "Unknown error";
}

void APhotonSimManager::removeOutputFiles()
{
    qDebug() << "Removing (if exist) files with the names listed in output files";

    const QString & OutputDir = SimSet.RunSet.OutputDirectory;
    std::vector<QString> fileNames;

    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNameSensorSignals);
    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNameTracks);
    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNamePhotonBombs);
    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNameStatistics);
    fileNames.push_back(OutputDir + '/' + SimSet.RunSet.FileNameMonitors);

    for (const QString & fn : fileNames)
        QFile::remove(fn);
}

#include "amonitorhub.h"
void APhotonSimManager::mergeOutput()
{
    qDebug() << "Merging output files...";

    const QString & OutputDir = SimSet.RunSet.OutputDirectory;
    if (SimSet.RunSet.SaveSensorSignals) ErrorString += SignalFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameSensorSignals);
    if (SimSet.RunSet.SaveTracks)        ErrorString += TrackFileMerger .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameTracks);
    if (SimSet.RunSet.SavePhotonBombs)   ErrorString += BombFileMerger  .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNamePhotonBombs);

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
}

void APhotonSimManager::addErrorLine(const QString & error)
{
    if (error.isEmpty()) return;

    if (ErrorString.isEmpty()) ErrorString = error;
    else                       ErrorString += QString("\n%0").arg(error);
}

bool APhotonSimManager::configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    qDebug() << "Configuring simulation...";

    Request.Command = "lsim"; // name of the corresponding executable

    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    SignalFileMerger.clear();
    TrackFileMerger.clear();
    BombFileMerger.clear();
    StatisticsFiles.clear();
    MonitorFiles.clear();

    ARandomHub & RandomHub = ARandomHub::getInstance();
    RandomHub.setSeed(SimSet.RunSet.Seed);

    int iEvent = 0;
    int iProcess = 0;
    //OutputFiles.clear();
    for (A3FarmNodeRecord & r : RunPlan)   // per node server
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)            // per process at the node server
        {
            if (num == 0) break;

            A3NodeWorkerConfig Worker;
            APhotonSimSettings WorkSet;

            switch (SimSet.SimType)
            {
            case EPhotSimType::PhotonBombs :
                switch (SimSet.BombSet.GenerationMode)
                {
                case EBombGen::Single :
                    // TODO: for single bomb, can split between processes by photons to! Try to implement after making file merge, since should be single event in this case
                    WorkSet = SimSet;
                    WorkSet.RunSet.EventFrom = iEvent;
                    WorkSet.RunSet.EventTo   = iEvent + 1;
                    iEvent++;
                    break;
                case EBombGen::Flood :
                    WorkSet = SimSet;
                    WorkSet.RunSet.EventFrom = iEvent;
                    WorkSet.RunSet.EventTo   = iEvent + num;
                    iEvent += num;
                    break;
                default:
                    ErrorString = "Not yet implemented!";
                    return false;
                }
                break;
            default:
                ErrorString = "Not yet implemented!";
                return false;
            }

            WorkSet.RunSet.Seed = INT_MAX * RandomHub.uniform();

            // !!!*** refactor to a method
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


            QJsonObject json;
            A3Config::getInstance().writeToJson(json);
            WorkSet.writeToJson(json);
            QString ConfigFN = QString("config-%0.json").arg(iProcess);
            jstools::saveJsonToFile(json, ExchangeDir + '/' + ConfigFN);
            Worker.ConfigFile = ConfigFN;

/*
            QString inputFN =  QString("data-%0.txt").arg(iProcess);
            ftools::saveTextToFile(text, ExchangeDir + '/' + inputFN);
            Worker.InputFiles.push_back(inputFN);
            OutputFiles.push_back(ExchangeDir + '/' + outputFN);
*/

            nc.Workers.push_back(Worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return true;
}
