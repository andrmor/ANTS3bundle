#ifndef acollineargammamodel_h
#define acollineargammamodel_h

#include "G4VFastSimulationModel.hh"
#include "G4ThreeVector.hh"

class AcollinearGammaModel : public G4VFastSimulationModel
{
public:
    AcollinearGammaModel(const G4String & name, G4Region * region, double angleDeg);

    G4bool IsApplicable(const G4ParticleDefinition & particle) override;

    G4bool ModelTrigger(const G4FastTrack & fastTrack) override;

    void DoIt(const G4FastTrack & fastTrack, G4FastStep & step) override;

protected:
    double Sigma;

    //int PrevID       = -1;
    int PrevParentID = -1;

    double PrevTime = 0;
    G4ThreeVector PrevPos, PrevDir;
};

class AcollinearGammaModel2D : public AcollinearGammaModel
{
public:
    AcollinearGammaModel2D(const G4String & name, G4Region * region, double angleDeg);

    void DoIt(const G4FastTrack & fastTrack, G4FastStep & step) override;
};

#endif // acollineargammamodel_h
