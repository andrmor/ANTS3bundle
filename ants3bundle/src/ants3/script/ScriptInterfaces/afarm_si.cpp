#include "afarm_si.h"
#include "afarmhub.h"

AFarm_SI::AFarm_SI() :
    AScriptInterface(), FarmHub(AFarmHub::getInstance())
{
    Help["countLocalNodes"] = "Return the user-defined number of local cores.";
}

int AFarm_SI::countLocalCores()
{
    return FarmHub.LocalProcesses;
}

void AFarm_SI::clearNodes()
{
    FarmHub.clearNodes();
}

void AFarm_SI::addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor)
{
    bool ok = FarmHub.addNode(Name, Address, Port, Cores, SpeedFactor);
    if (!ok) abort("Failed to add farm node: IP and port already exist");
}
