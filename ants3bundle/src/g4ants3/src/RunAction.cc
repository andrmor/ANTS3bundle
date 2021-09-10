#include "RunAction.hh"
#include "SessionManager.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

RunAction::RunAction()
    : G4UserRunAction()
{ 
    // set printing event number per each 100 events
    //G4RunManager::GetRunManager()->SetPrintProgress(1000);
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    SessionManager & SM = SessionManager::getInstance();

    SM.resetPredictedTrackID();
    SM.writeNewEventMarker();

    //inform the runManager to save random number seed             *** need?
    //G4RunManager::GetRunManager()->SetRandomNumberStore(false);
}

void RunAction::EndOfRunAction(const G4Run* )
{
    SessionManager & SM = SessionManager::getInstance();
    SM.onEventFinished();
}
