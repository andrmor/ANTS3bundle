#include "EventAction.hh"
#include "SessionManager.hh"
#include "SensitiveDetector.hh"

EventAction::EventAction()
: G4UserEventAction() {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();
    SM.writeNewEventMarker();

    for (CalorimeterSensitiveDetector * cal : SM.Calorimeters) cal->SumDepoOverEvent = 0;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();
    SM.onEventFinished();
    SM.resetPredictedTrackID();

    for (CalorimeterSensitiveDetector * cal : SM.Calorimeters)
        if (cal->EventDepoData) cal->EventDepoData->fill(cal->SumDepoOverEvent);
}
