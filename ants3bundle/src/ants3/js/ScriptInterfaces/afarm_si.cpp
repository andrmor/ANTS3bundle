#include "afarm_si.h"
#include "afarmhub.h"

AFarm_SI::AFarm_SI(QObject * parent) :
    AScriptInterface(),
    FarmHub(AFarmHub::getInstance()) {}

void AFarm_SI::clearNodes()
{
    FarmHub.clearNodes();
}

void AFarm_SI::addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor)
{
    bool ok = FarmHub.addNode(Name, Address, Port, Cores, SpeedFactor);

    if (!ok)
    {
        // abort! !!!***
    }
}
