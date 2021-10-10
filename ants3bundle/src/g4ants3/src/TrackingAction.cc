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

    const int iMat = SM.findMaterial( track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName() ); //will terminate session if not found!
    SM.saveTrackStart(track->GetTrackID(), track->GetParentID(),
                      track->GetParticleDefinition()->GetParticleName(),
                      track->GetPosition(), track->GetGlobalTime()/ns, track->GetKineticEnergy()/keV,
                      iMat, track->GetVolume()->GetLogicalVolume()->GetName(), track->GetVolume()->GetCopyNo());
}

/*
void TrackingAction::PostUserTrackingAction(const G4Track *)
{
    SessionManager & SM = SessionManager::getInstance();
}
*/
