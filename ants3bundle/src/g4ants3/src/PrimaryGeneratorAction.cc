#include "PrimaryGeneratorAction.hh"
#include "SessionManager.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    G4int nofParticles = 1;
    fParticleGun = new G4ParticleGun(nofParticles);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

#include "G4IonTable.hh"
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    SessionManager & SM = SessionManager::getInstance();
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
}
