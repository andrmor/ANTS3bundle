#include "aphotonsimmanager.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "a3config.h"
#include "a3global.h"
#include "a3dispinterface.h"
#include "a3workdistrconfig.h"
#include "ajsontools.h"

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

    int numEvents = 0;
    const APhotonSimSettings & SimSet = APhotonSimHub::getInstance().Settings;
    switch (SimSet.SimType)
    {
    case EPhotSimType::PhotonBombs :
        switch (SimSet.BombSet.GenerationMode)
        {
        case EBombGen::Single :
            // TODO: for single bomb, can split between processes by photons to! Try to implement after making file merge, since should be single event in this case
            numEvents = 1;
            break;
        default:
            ErrorString = "Not yet implemented!";
            return false;
        }
        break;
    case EPhotSimType::FromLRFs :
        {
            // direct calculation here!
            // ...
            emit simFinished();
            return true;
        }
    default:
        ErrorString = "Not yet implemented!";
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

    qDebug() << "Merging files...";
    /*
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    ftools::mergeTextFiles(OutputFiles, ExchangeDir + '/' + ResultsFileName);
    */

    qDebug() << "Photon simulation finished";
    emit simFinished();
    return true;
}

bool APhotonSimManager::configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "photonsim"; // name of the corresponding executable

    const APhotonSimSettings & SimSet = APhotonSimHub::getInstance().Settings;
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    int iEvent = 0;
    int iProcess = 0;
    //OutputFiles.clear();
    for (A3FarmNodeRecord & r : RunPlan)
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)
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
                    WorkSet.RunSet.EventFrom = 0;
                    WorkSet.RunSet.EventTo   = 1;
                    break;
                default:
                    ErrorString = "Not yet implemented!";
                    return false;
                }
                break;
            default:
                qDebug() << "Not yet implemented!";
                return false;
            }

            // standard output file names TODO: selective using ouput control!
            WorkSet.RunSet.FileNameSensorSignals = QString("signals-%0").arg(iProcess);
            Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameSensorSignals);
            WorkSet.RunSet.FileNameTracks       = QString("tracks-%0") .arg(iProcess);
            Worker.OutputFiles.push_back(WorkSet.RunSet.FileNameTracks);

            // config
            QString ConfigFN = QString("config-%0.json").arg(iProcess);
            QJsonObject jsSim;
            WorkSet.writeToJson(jsSim);
            QJsonObject json;
            A3Config::getInstance().formConfigForPhotonSimulation(jsSim, json);
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
