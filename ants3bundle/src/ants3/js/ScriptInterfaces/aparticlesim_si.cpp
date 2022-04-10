#include "aparticlesim_si.h"
#include "aparticlesimmanager.h"
#include "aerrorhub.h"

AParticleSim_SI::AParticleSim_SI() :
    SimMan(AParticleSimManager::getInstance()) {}

void AParticleSim_SI::simulate(bool updateGui)
{
    AErrorHub::clear();

    SimMan.simulate();

    QString err = AErrorHub::getQError();
    if (err.isEmpty())
    {
        if (updateGui) SimMan.requestUpdateResultsGUI();;
    }
    else
    {
        abort(err);
    }
}
