#include "ageant4inspectormanager.h"
#include "adispatcherinterface.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "a3workdistrconfig.h"
#include "a3global.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>

AGeant4InspectorManager::AGeant4InspectorManager() :
    QObject(nullptr), Dispatch(ADispatcherInterface::getInstance()) {}

AGeant4InspectorManager & AGeant4InspectorManager::getInstance()
{
    static AGeant4InspectorManager instance;
    return instance;
}

bool AGeant4InspectorManager::inspectMaterial(const QString & matName, AG4MaterialRecord & reply)
{
    bAborted = false;
    ErrorString.clear();

    int numEvents = 1;
    int numLocalProc = 1.0;

    std::vector<AFarmNodeRecord> RunPlan;
    QString err = Dispatch.fillRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }
    qDebug() << "Obtained run plan over local/farm nodes:";
    for (AFarmNodeRecord & r : RunPlan) qDebug() << r.Address << r.Split;

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;
    bool ok = configureForInspectMaterial(matName, RunPlan, Request);
    if (!ok) return false;

    QJsonObject Reply = Dispatch.performTask(Request);
    qDebug() << "Reply:" << Reply;

    if (bAborted)
    {
        emit finished(false);
        return false;
    }

    qDebug() << "Processing reply...";
    QJsonObject json;
    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    ok = jstools::loadJsonFromFile(json, ExchangeDir + '/' + ResultsFileName);
    if (!ok) ErrorString = "Failed to find reply file with the results";
    else
    {
        bool bSuccess = false;
        ok = jstools::parseJson(json, "Success", bSuccess);
        if (!ok || !bSuccess)
        {
            ErrorString = "Failed to decode the reply";
            jstools::parseJson(json, "Error", ErrorString);
        }
        else
        {
            QJsonObject js;
            jstools::parseJson(json, "Response", js);

            jstools::parseJson(js, "Density", reply.Density);
            jstools::parseJson(js, "Formula", reply.Formula);

        }
    }

    emit finished(true);
    return true;
}

void AGeant4InspectorManager::abort()
{
    Dispatch.abortTask();
    bAborted = true;
}

bool AGeant4InspectorManager::configureForInspectMaterial(const QString & matName, std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "g4inspector";

    const QString & ExchangeDir = A3Global::getInstance().ExchangeDir;
    Request.ExchangeDir = ExchangeDir;

    AFarmNodeRecord & r = RunPlan.front();

    A3WorkNodeConfig nc;
    nc.Address = r.Address;
    nc.Port    = r.Port;

    A3NodeWorkerConfig worker;

    QJsonObject jsonIn;
    jsonIn["Request"] = "MaterialComposition";
    jsonIn["MaterialName"] = matName;
    jstools::saveJsonToFile(jsonIn, ExchangeDir + "/" + RequestFileName);

    worker.ConfigFile = RequestFileName;

    worker.OutputFiles.push_back(ResultsFileName);

    nc.Workers.push_back(worker);

    Request.Nodes.push_back(nc);

    return true;
}
