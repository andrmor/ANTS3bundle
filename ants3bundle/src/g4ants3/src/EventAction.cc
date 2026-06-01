#include "EventAction.hh"
#include "SessionManager.hh"
#include "SensitiveDetector.hh"

EventAction::EventAction()
: G4UserEventAction() {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();

    SM.tmpOutStream.str(std::string());
    SM.tmpOutStream.clear();
    SM.bSaveTmpStream = false;

    //SM.writeNewEventMarker();

    for (CalorimeterSensitiveDetectorWrapper * cal : SM.Calorimeters) cal->SumDepoOverEvent = 0;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    SessionManager & SM = SessionManager::getInstance();
    SM.onEventFinished();
    SM.resetPredictedTrackID();

    if (SM.bSaveTmpStream)
    {
        std::string EventId = "#" + std::to_string(SM.tmpId);
        SM.tmpId++;
        *SM.outStreamHistory << EventId.data() << '\n';
        *SM.outStreamHistory << SM.tmpOutStream.rdbuf(); //<< '\n';
    }

    for (CalorimeterSensitiveDetectorWrapper * cal : SM.Calorimeters)
        if (cal->EventDepoData && cal->SumDepoOverEvent > 0) cal->EventDepoData->fill(cal->SumDepoOverEvent);
}
