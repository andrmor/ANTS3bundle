#include "PrimaryGeneratorAction.hh"
#include "SessionManager.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    GeantParticleGun = new G4ParticleGun(1);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete GeantParticleGun;
}

#include "G4IonTable.hh"
#include "aparticlegun.h"
#include "aparticlerecord.h"
void PrimaryGeneratorAction::GeneratePrimaries(G4Event * anEvent)
{
    SessionManager & SM = SessionManager::getInstance();

    /*
    const std::vector<ParticleRecord> & GeneratedPrimaries = SM.getNextEventPrimaries();

    for (const ParticleRecord & r : GeneratedPrimaries)
    {
        //std::cout << r.Particle->GetParticleName() <<" Pos:"<<r.Position[0]<<" "<<r.Position[1]<<" "<<r.Position[2] <<" Dir:"<<
        //             r.Direction[0]<<" "<<r.Direction[1]<<" "<<r.Direction[2]<<" E:"<< r.Energy <<" T:"<< r.Time << std::endl;

        fParticleGun->SetParticleDefinition(r.Particle);
        fParticleGun->SetParticlePosition(r.Position); //position in millimeters - no need units
        fParticleGun->SetParticleMomentumDirection(r.Direction);
        fParticleGun->SetParticleEnergy(r.Energy*keV);
        fParticleGun->SetParticleTime(r.Time); //in ns - no need units

        fParticleGun->GeneratePrimaryVertex(anEvent);

        SM.incrementPredictedTrackID();
    }
    */

    auto Handler = [this, anEvent](const AParticleRecord & particle) mutable
    {
        //qDebug() << particle.r[0]<<particle.r[1]<< particle.r[2] << "  V: " << particle.v[0]<< particle.v[1]<< particle.v[2];
        GeantParticleGun->SetParticleDefinition(particle.particle);
        GeantParticleGun->SetParticlePosition({particle.r[0], particle.r[1], particle.r[2]}); //position in millimeters - no need units
        GeantParticleGun->SetParticleMomentumDirection({particle.v[0], particle.v[1], particle.v[2]});
        GeantParticleGun->SetParticleEnergy(particle.energy * keV);
        GeantParticleGun->SetParticleTime(particle.time); //in ns - no need units

        GeantParticleGun->GeneratePrimaryVertex(anEvent);
    };

    SM.ParticleGenerator->generateEvent(Handler, SM.CurrentEvent);
}
