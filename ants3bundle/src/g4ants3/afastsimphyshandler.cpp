#include "afastsimphyshandler.h"

#include "G4VModularPhysicsList.hh"
#include "G4FastSimulationPhysics.hh"

void AFastSimPhysHandler::createFastSimulationPhysics(G4VModularPhysicsList * physicsList)
{
    G4FastSimulationPhysics * fsm = new G4FastSimulationPhysics();
    //https://indico.cern.ch/event/789510/contributions/3297180/attachments/1817759/2973421/G4Tutorial_fastSim_vFin.pdf

    // see createPhantomRegion() for registration of the models
    if (true) fsm->ActivateFastSimulation("gamma");  // see registerAcollinearGammaModel()
    physicsList->RegisterPhysics(fsm);
}
