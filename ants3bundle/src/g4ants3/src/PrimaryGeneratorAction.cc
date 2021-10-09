#include "PrimaryGeneratorAction.hh"
#include "SessionManager.hh"
#include "aparticlegun.h"
#include "aparticlerecord.h"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4IonTable.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    GeantParticleGun = new G4ParticleGun(1);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete GeantParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event * anEvent)
{
    SessionManager & SM = SessionManager::getInstance();

    auto Handler = [this, anEvent](const AParticleRecord & particle) mutable
    {
        //qDebug() << particle.r[0]<<particle.r[1]<< particle.r[2] << "  V: " << particle.v[0]<< particle.v[1]<< particle.v[2];
        GeantParticleGun->SetParticleDefinition(particle.particle);
        GeantParticleGun->SetParticlePosition({particle.r[0], particle.r[1], particle.r[2]}); //position in millimeters - no need units
        GeantParticleGun->SetParticleMomentumDirection({particle.v[0], particle.v[1], particle.v[2]});
        GeantParticleGun->SetParticleEnergy(particle.energy * keV);
        GeantParticleGun->SetParticleTime(particle.time); //in ns - no need units

        GeantParticleGun->GeneratePrimaryVertex(anEvent);
        incrementPredictedTrackID();
    };

    SM.ParticleGenerator->generateEvent(Handler, SM.CurrentEvent);
}
