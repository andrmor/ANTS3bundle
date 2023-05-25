#include "afarmhub.h"
#include "afarmnoderecord.h"
#include "adispatcherinterface.h"
#include "a3workdistrconfig.h"
#include "ajsontools.h"

AFarmHub &AFarmHub::getInstance()
{
    static AFarmHub instance;
    return instance;
}

const AFarmHub &AFarmHub::getConstInstance()
{
    return getInstance();
}

void AFarmHub::clearNodes()
{
    for (auto * n : FarmNodes) delete n;
    FarmNodes.clear();
}

void AFarmHub::addNewNode()
{
    AFarmNodeRecord * node = new AFarmNodeRecord();

    while (isIPandPortAlreadyExist(node->Address, node->Port))
        node->Port++;

    FarmNodes.push_back(node);
}

bool AFarmHub::addNode(const QString & name, const QString & ip, int port, int maxProcesses, double speedFactor)
{
    if (isIPandPortAlreadyExist(ip, port)) return false;

    AFarmNodeRecord * node = new AFarmNodeRecord();
    node->Name = name;
    node->Address = ip;
    node->Port = port;
    node->Processes = maxProcesses;
    node->SpeedFactor = speedFactor;
    FarmNodes.push_back(node);

    return true;
}

bool AFarmHub::removeNode(int index)
{
    if (index < 0 || index >= (int)FarmNodes.size()) return false;

    delete FarmNodes[index];
    FarmNodes.erase(FarmNodes.begin() + index);
    return true;
}

void AFarmHub::checkFarmStatus()
{
    ADispatcherInterface & Disp = ADispatcherInterface::getInstance();

    A3WorkDistrConfig Request;
    Request.Command = "check";

    for (const AFarmNodeRecord * FarmNode : FarmNodes)
    {
        A3WorkNodeConfig nc;
        nc.Address = FarmNode->Address;
        nc.Port    = FarmNode->Port;
        Request.Nodes.push_back(nc);
    }

    QJsonObject Reply = Disp.performTask(Request);

    QJsonArray ar = Reply["NodeStatus"].toArray();

    int iN = 0;
    for (AFarmNodeRecord * FarmNode : FarmNodes)
    {
        if (iN < ar.size())
        {
            int proc = ar[iN].toInt();
            if (proc == -1)
            {
                FarmNode->Status    = AFarmNodeRecord::NotResponding;
                FarmNode->Processes = 0;
                FarmNode->Enabled   = false;
            }
            else
            {
                FarmNode->Status    = AFarmNodeRecord::Available;
                FarmNode->Processes = proc;
            }
        }
        iN++;
    }
}

bool AFarmHub::isIPandPortAlreadyExist(const QString &address, int port) const
{
    for (const AFarmNodeRecord * fn : FarmNodes)
    {
        if (fn->Address != address) continue;
        if (fn->Port == port) return true;
    }

    return false;
}

void AFarmHub::writeToJson(QJsonObject & json) const
{
    json["UseLocal"]       = UseLocal;
    json["LocalProcesses"] = LocalProcesses;
    json["UseFarm"]        = UseFarm;
    json["TimeoutMs"]      = TimeoutMs;

    QJsonArray ar;
    qDebug() << "-----------------"<<FarmNodes.size();
    for (AFarmNodeRecord * node : FarmNodes)
    {
        QJsonObject js;
        node->writeToJson(js);
        ar.push_back(js);
    }
    json["FarmNodes"] = ar;
}

void AFarmHub::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "UseLocal",       UseLocal);
    jstools::parseJson(json, "LocalProcesses", LocalProcesses);
    jstools::parseJson(json, "UseFarm",        UseFarm);
    jstools::parseJson(json, "TimeoutMs",      TimeoutMs);

    clearNodes();
    QJsonArray ar;
    jstools::parseJson(json, "FarmNodes", ar);
    for (int iNode = 0; iNode < ar.size(); iNode++)
    {
        QJsonObject js = ar[iNode].toObject();
        AFarmNodeRecord * node = new AFarmNodeRecord();
        node->readFromJson(js);
        FarmNodes.push_back(node);
    }
}
