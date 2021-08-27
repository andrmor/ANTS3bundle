#include "StackingAction.hh"
#include "G4Track.hh"
#include "SessionManager.hh"
#include "G4StackManager.hh"

#include <iostream>

StackingAction::StackingAction() {}

StackingAction::~StackingAction() {}

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track * /*track*/)
{
    //SessionManager & SM = SessionManager::getInstance();

    // WARNING!
    //deleting the tracks or assigning to special stacks will upset the track ID prediction system:
    //see SM.NextTrackID

    //std::stringstream ss;
    //ss << "  track created: " << track->GetTrackID() << "  before add total # of tracks: " <<stackManager->GetNTotalTrack();
    //SM.sendLineToTracksOutput(ss);

    return fWaiting;
}
