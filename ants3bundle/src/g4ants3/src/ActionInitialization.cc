#include "ActionInitialization.hh"
#include "SessionManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"
//#include "StackingAction.hh"

ActionInitialization::ActionInitialization()
    : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction());

    //SetUserAction(new RunAction);  // rebased all to Action!

    SetUserAction(new EventAction);

    //SetUserAction(new StackingAction);

    SessionManager & SM = SessionManager::getInstance();
    if (SM.CollectHistory != SessionManager::NotCollecting || SM.bMonitorsRequireSteppingAction || SM.bExitParticles)
        SetUserAction(new SteppingAction);

    if (SM.CollectHistory != SessionManager::NotCollecting)
        SetUserAction(new TrackingAction);
}
