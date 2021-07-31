#include "a3particlesimmanager.h"
#include "a3dispinterface.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "a3workdistrconfig.h"
#include "a3config.h"
#include "a3global.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>

A3ParticleSimManager::A3ParticleSimManager(QObject * parent) :
    QObject(parent), Dispatch(A3DispInterface::getInstance()) {}

A3ParticleSimManager::~A3ParticleSimManager()
{
    qDebug() << "Destr for ParticleSimManager";
}

bool A3ParticleSimManager::simulate(int numLocalProc)
{
    qDebug() << "Particle sim triggered";
    ErrorString.clear();
    if (numLocalProc < 0) numLocalProc = 4;

    A3Config & Config = A3Config::getInstance();
    QStringList Events = Config.lines.split('\n', Qt::SkipEmptyParts);
    int numEvents = Events.size();
    if (numEvents == 0)
    {
        ErrorString = "No input data!";
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
    bool ok = configureParticleSimulation(RunPlan, Request);
    if (!ok) return false;

    QString Reply = Dispatch.performTask(Request);
    qDebug() << "Reply message:" << Reply;

    qDebug() << "Merging files...";
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    ftools::mergeTextFiles(OutputFiles, ExchangeDir + '/' + ResultsFileName);

    qDebug() << "Particle simulation finished";
    emit simFinished();
    return true;
}

bool A3ParticleSimManager::configureParticleSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "psim"; // name of the corresponding executable

    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;
    A3Config & Config = A3Config::getInstance();
    QStringList Events = Config.lines.split('\n', Qt::SkipEmptyParts);

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

            nc.Workers.push_back(worker);
            iProcess++;
        }
        Request.Nodes.push_back(nc);
    }

    return true;
}
