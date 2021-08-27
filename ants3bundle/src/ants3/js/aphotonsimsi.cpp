#include "aphotonsimsi.h"
#include "aphotonsimmanager.h"

APhotonSimSI::APhotonSimSI(QObject *parent) :
    QObject(parent), SimMan(APhotonSimManager::getInstance()) {}

QString APhotonSimSI::simulate(bool updateGui)
{
    bool ok = SimMan.simulate(-1);
    if (ok)
    {
        if (updateGui) emit SimMan.requestUpdateGUI();
        return "Done!";
    }
    else
    {
        return SimMan.ErrorString;
    }
}
