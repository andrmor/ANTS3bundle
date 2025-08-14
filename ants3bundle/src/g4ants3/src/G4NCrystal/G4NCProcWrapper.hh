#ifndef G4NCrystal_ProcWrapper_hh
#define G4NCrystal_ProcWrapper_hh

#include "G4VDiscreteProcess.hh"
#include "G4ParticleChange.hh"

class G4HadronElasticProcess;
class G4Material;

namespace G4NCrystal {

  class Manager;

  class ProcWrapper : public G4VDiscreteProcess
  {
    // Wrapper process used by G4NCInstall to dynamically support NCrystal
    // physics with any physics model.

  public:
    ProcWrapper(G4HadronElasticProcess * procToWrap,
                const G4String& processName = "");
    virtual ~ProcWrapper();

    //Intercepted methods in which NCrystal scatter physics is supplied when
    //appropriate, passing the call through to the wrapped process when not:
    G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step& ) final;
    G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*) final;

    void BuildPhysicsTable(const G4ParticleDefinition&) final;
    G4bool IsApplicable(const G4ParticleDefinition& pd) final;

  private:
    G4ParticleChange m_particleChange;
    G4HadronElasticProcess * m_wrappedProc;
    Manager * m_mgr;
  };

}

#endif
