#include "a3farmsi.h"
#include "afarmhub.h"

A3FarmSI::A3FarmSI(QObject * parent) :
    QObject(parent),
    FarmHub(AFarmHub::getInstance()) {}

void A3FarmSI::clearNodes()
{
    FarmHub.clearNodes();
}

void A3FarmSI::addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor)
{
    bool ok = FarmHub.addNode(Name, Address, Port, Cores, SpeedFactor);

    if (!ok)
    {
        // abort! !!!***
    }
}
