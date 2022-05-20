#include "aphotonsim_si.h"
#include "aphotonsimmanager.h"
#include "aerrorhub.h"

#include <QDebug>

APhotonSim_SI::APhotonSim_SI() :
    AScriptInterface(), SimMan(APhotonSimManager::getInstance()) {}

APhotonSim_SI::~APhotonSim_SI()
{
    qDebug() << "Dest for APhotonSim_SI";
}

QString APhotonSim_SI::simulate(bool updateGui)
{
    bool ok = SimMan.simulate(-1);
    if (ok)
    {
        if (updateGui) emit SimMan.requestUpdateResultsGUI();
        return "Done!";
    }
    else
    {
        return AErrorHub::getQError();
    }
}
