#include "aphotonsimmanager.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "a3config.h"
#include "a3global.h"
#include "a3dispinterface.h"
#include "a3workdistrconfig.h"
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

bool APhotonSimManager::simulate(int numLocalProc)
{
    qDebug() << "Photon sim triggered";
    ErrorString.clear();

    const APhotonSimSettings & SimSet = APhotonSimHub::getInstance().Settings;

    if (SimSet.RunSet.OutputDirectory.isEmpty())
    {
        ErrorString = "Output directory is not set!";
        return false;
    }
    if (!QDir(SimSet.RunSet.OutputDirectory).exists())
    {
        ErrorString = "Output directory does not exist!";
        return false;
    }

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
            // direct calculation here!
            // ...
            //return true;
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

    // configure number of lical/remote processes to run
    //A3Global & GlobSet = A3Global::getInstance();
    if (numLocalProc < 0) numLocalProc = 4;

    std::vector<A3FarmNodeRecord> RunPlan;
    A3DispInterface & Dispatcher = A3DispInterface::getInstance();
    QString err = Dispatcher.prepareRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }
    qDebug() << "Obtained run plan over local/farm nodes:";
    for (A3FarmNodeRecord & r : RunPlan) qDebug() << "--->" << r.Address << r.Split;

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;
    bool ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    QString Reply = Dispatcher.performTask(Request);
    qDebug() << "Reply message:" << Reply;

    qDebug() << "Merging output files...";
    const QString & OutputDir = SimSet.RunSet.OutputDirectory;
    if (SimSet.RunSet.SaveSensorSignals) ErrorString += SignalFileMerger.mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameSensorSignals);
    if (SimSet.RunSet.SaveTracks)        ErrorString += TrackFileMerger .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNameTracks);
    if (SimSet.RunSet.SavePhotonBombs)   ErrorString += BombFileMerger  .mergeToFile(OutputDir + '/' + SimSet.RunSet.FileNamePhotonBombs);
    if (SimSet.RunSet.SaveStatistics)
    {
        // custom procedure !!!***
        //ErrorString (OutputDir + '/' + SimSet.RunSet.FileNameStatistics);
    }

    qDebug() << "Photon simulation finished";
    return ErrorString.isEmpty();
}

bool APhotonSimManager::configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    // !!!*** enforce output and exchange directories exist!

    Request.Command = "lsim"; // name of the corresponding executable

    const APhotonSimSettings & SimSet = APhotonSimHub::getInstance().Settings;
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    SignalFileMerger.clear();
    TrackFileMerger.clear();
    BombFileMerger.clear();
    StatisticsFileMerger.clear();

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
                WorkSet.RunSet.FileNameTracks       = QString("tracks-%0") .arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameTracks);
                TrackFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameTracks);
            }
            if (SimSet.RunSet.SavePhotonBombs)
            {
                WorkSet.RunSet.FileNamePhotonBombs  = QString("bombs-%0") .arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNamePhotonBombs);
                BombFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNamePhotonBombs);
            }
            if (SimSet.RunSet.SaveStatistics)
            {
                WorkSet.RunSet.FileNameStatistics   = QString("stats-%0") .arg(iProcess);
                Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameStatistics);
                StatisticsFileMerger.add(ExchangeDir + '/' + WorkSet.RunSet.FileNameStatistics);
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
