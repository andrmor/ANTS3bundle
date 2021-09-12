#include "EventAction.hh"
#include "SessionManager.hh"

EventAction::EventAction()
: G4UserEventAction() {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();
    SM.writeNewEventMarker();
}

void EventAction::EndOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();
    SM.onEventFinished();
    SM.resetPredictedTrackID();
}
