#include "aphotonsimmanager.h"
#include "a3workdistrconfig.h"
#include "a3dispinterface.h"
#include "aphotonsimhub.h"
#include "a3global.h"
#include "ajsontools.h"
#include "afiletools.h"

#include <QDebug>

APhotonSimManager::APhotonSimManager(A3DispInterface & Dispatch, QObject * parent) :
    QObject(parent), Dispatch(Dispatch)
{

}

APhotonSimManager::~APhotonSimManager()
{

}

bool APhotonSimManager::simulate(int numLocalProc)
{
    qDebug() << "Photon sim triggered";
    ErrorString.clear();
    //A3Global & GlobSet = A3Global::getInstance();
    if (numLocalProc < 0) numLocalProc = 4;

    int numEvents = 0;
    APhotonSimHub & SimSet = APhotonSimHub::getInstance();
    switch (SimSet.Settings.SimType)
    {
    case EPhotSimType::PhotonBombs :
        numEvents = 1;
        break;
    default:
        ErrorString = "Not yet implemented!";
        return false;
    }

    if (numEvents == 0)
    {
        ErrorString = "Nothing to simulate!";
        return false;
    }

    std::vector<A3FarmNodeRecord> RunPlan;
    QString err = Dispatch.prepareRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }
    qDebug() << "Obtained run plan over local/farm nodes:";
    for (A3FarmNodeRecord & r : RunPlan) qDebug() << r.Address << r.Split;

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;
    bool ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    QString Reply = Dispatch.performTask(Request);
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

    APhotonSimHub & SimSet = APhotonSimHub::getInstance();
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    int iEvent = 0;
    int iProcess = 0;
    OutputFiles.clear();
    for (A3FarmNodeRecord & r : RunPlan)
    {
        A3WorkNodeConfig nc;
        nc.Address = r.Address;
        nc.Port    = r.Port;

        for (int num : r.Split)
        {
            if (num == 0) break;

            A3NodeWorkerConfig worker;

            switch (SimSet.Settings.SimType)
            {
            case EPhotSimType::PhotonBombs :
                //APhotonSimSettings LocalSet = SimSet.Settings;
                break;
            default:
                qDebug() << "Not yet implemented!";
                return false;
            }
/*
            QString text;
            for (int i=0; i<num; i++)
                text += Events[iEvent++] + '\n';

            QString inputFN =  QString("data-%0.txt").arg(iProcess);
            ftools::saveTextToFile(text, ExchangeDir + '/' + inputFN);
            worker.InputFiles.push_back(inputFN);

            QJsonObject json;
            json["ID"]     = iProcess;
            json["Input"]  = inputFN;
            QString outputFN = QString("output-%0.txt").arg(iProcess);
            json["Output"] = outputFN;
            json["From"]   = Config.from;
            json["To"]     = Config.to;
            worker.OutputFiles.push_back(outputFN);
            OutputFiles.push_back(ExchangeDir + '/' + outputFN);

            QString configFN = QString("config-%0.json").arg(iProcess);
            jstools::saveJsonToFile(json, ExchangeDir + '/' + configFN);
            worker.ConfigFile = configFN;
*/

            nc.Workers.push_back(worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return true;
}
