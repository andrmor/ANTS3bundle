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

    // TODO check it is has unique IP:Port   !!!***

    FarmNodes.push_back(node);
}

bool AFarmHub::addNode(const QString & name, const QString & ip, int port, int maxProcesses, double speedFactor)
{
    A3FarmNodeRecord * node = new A3FarmNodeRecord();

    node->Name = name;
    node->Address = ip;
    node->Port = port;
    node->Processes = maxProcesses;
    node->SpeedFactor = speedFactor;

    // TODO check it is has unique IP:Port   !!!***

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


