#include "afarmhub.h"
#include "a3farmnoderecord.h"

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
    A3FarmNodeRecord * node = new A3FarmNodeRecord();

    while (isIPandPortAlreadyExist(node->Address, node->Port))
        node->Port++;

    FarmNodes.push_back(node);
}

bool AFarmHub::addNode(const QString & name, const QString & ip, int port, int maxProcesses, double speedFactor)
{
    if (isIPandPortAlreadyExist(ip, port)) return false;

    A3FarmNodeRecord * node = new A3FarmNodeRecord();
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

#include "adispatcherinterface.h"
#include "a3workdistrconfig.h"
#include <QJsonArray>
void AFarmHub::checkFarmStatus()
{
    ADispatcherInterface & Disp = ADispatcherInterface::getInstance();

    A3WorkDistrConfig Request;
    Request.Command = "check";

    for (const A3FarmNodeRecord * FarmNode : FarmNodes)
    {
        A3WorkNodeConfig nc;
        nc.Address = FarmNode->Address;
        nc.Port    = FarmNode->Port;
        Request.Nodes.push_back(nc);
    }

    QJsonObject Reply = Disp.performTask(Request);

    QJsonArray ar = Reply["NodeStatus"].toArray();

    int iN = 0;
    for (A3FarmNodeRecord * FarmNode : FarmNodes)
    {
        if (iN < ar.size())
        {
            int proc = ar[iN].toInt();
            if (proc == -1)
            {
                FarmNode->Status    = A3FarmNodeRecord::NotResponding;
                FarmNode->Processes = 0;
                FarmNode->Enabled   = false;
            }
            else
            {
                FarmNode->Status    = A3FarmNodeRecord::Available;
                FarmNode->Processes = proc;
            }
        }
        iN++;
    }
}

bool AFarmHub::isIPandPortAlreadyExist(const QString &address, int port) const
{
    for (const A3FarmNodeRecord * fn : FarmNodes)
    {
        if (fn->Address != address) continue;
        if (fn->Port == port) return true;
    }

    return false;
}
