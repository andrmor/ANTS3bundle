#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event * );

    void resetPredictedTrackID()     {NextTrackID = 1;}
    void incrementPredictedTrackID() {NextTrackID++;}
    int  getPredictedTrackID() const {return NextTrackID;}

private:
    G4ParticleGun * GeantParticleGun = nullptr;
    int NextTrackID = 1;
};

#endif // PrimaryGeneratorAction_h
