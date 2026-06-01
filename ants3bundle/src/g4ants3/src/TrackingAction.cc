#include "TrackingAction.hh"
#include "SessionManager.hh"

#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4Gamma.hh"

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

    if (track->GetParticleDefinition() != G4Gamma::Definition()) return;
    const double energy = track->GetKineticEnergy() / keV;
    //if (energy < 510.0 || energy > 512.0) return;
    if (energy < 1519.0 || energy > 1521.0) return;

    //qDebug() << "Pass!";
    SM.bSaveTmpStream = true;
}

/*
void TrackingAction::PostUserTrackingAction(const G4Track *)
{
    SessionManager & SM = SessionManager::getInstance();
}
*/
