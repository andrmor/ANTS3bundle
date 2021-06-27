#include "a3farmsi.h"
#include "a3global.h"

A3FarmSI::A3FarmSI(QObject * parent) : QObject(parent), FarmNodes(A3Global::getInstance().FarmNodes) {}

void A3FarmSI::clearNodes()
{
    FarmNodes.clear();
}

void A3FarmSI::addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor)
{
    //A3Global & GlobSet = A3Global::getInstance();

    A3FarmNodeRecord nn;
    nn.Name = Name;
    nn.Address = Address;
    nn.Port = Port;
    nn.Cores = Cores;
    nn.SpeedFactor = SpeedFactor;

    FarmNodes << nn;
}
