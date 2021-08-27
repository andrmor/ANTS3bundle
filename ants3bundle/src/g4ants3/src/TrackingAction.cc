#include "TrackingAction.hh"
#include "SessionManager.hh"

#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <QDebug>
TrackingAction::TrackingAction(){}

TrackingAction::~TrackingAction(){}

void TrackingAction::PreUserTrackingAction(const G4Track *track)
{
    SessionManager & SM = SessionManager::getInstance();
    if (SM.CollectHistory == SessionManager::NotCollecting) return;

    // format:
    // > TrackID ParentTrackID ParticleId X Y Z Time E iMat VolName VolIndex

    /*
    std::stringstream ss;
    ss.precision(SM.Precision);

    ss << '>';
    ss << track->GetTrackID() << ' ';
    ss << track->GetParentID() << ' ';
    ss << track->GetParticleDefinition()->GetParticleName() << ' ';
    const G4ThreeVector & pos = track->GetPosition();
    ss << pos[0] << ' ' << pos[1] << ' ' << pos[2] << ' ';
    ss << track->GetGlobalTime()/ns << ' ';
    ss << track->GetKineticEnergy()/keV << ' ';

    const int iMat = SM.findMaterial( track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName() ); //will terminate session if not found!
    ss << iMat << ' ';
    ss << track->GetVolume()->GetLogicalVolume()->GetName() << ' ';
    ss << track->GetVolume()->GetCopyNo() << ' ';

    SM.sendLineToTracksOutput(ss);
    */

    const int iMat = SM.findMaterial( track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName() ); //will terminate session if not found!

    SM.saveTrackStart(track->GetTrackID(), track->GetParentID(),
                      track->GetParticleDefinition()->GetParticleName(),
                      track->GetPosition(), track->GetGlobalTime()/ns, track->GetKineticEnergy()/keV,
                      iMat, track->GetVolume()->GetLogicalVolume()->GetName(), track->GetVolume()->GetCopyNo());
}

void TrackingAction::PostUserTrackingAction(const G4Track *)
{
    SessionManager & SM = SessionManager::getInstance();
    if (SM.CollectHistory == SessionManager::NotCollecting) return;

    if (SM.CollectHistory == SessionManager::OnlyTracks)
    {
        SM.TracksToBuild--;
        if (SM.TracksToBuild <= 0)
            SM.CollectHistory = SessionManager::NotCollecting;
    }
}
