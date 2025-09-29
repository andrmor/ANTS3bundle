#ifndef G4NCrystal_Bias_hh
#define G4NCrystal_Bias_hh

#include "G4VBiasingOperator.hh"

class G4BOptnChangeCrossSection;

namespace G4NCrystal {


  class NCrystalBiasingOperator final : public G4VBiasingOperator {

    // Biasing operator that applies to thermal neutrons, in order for NCrystal
    // to provide scattering physics.
    //
    // In the case of low-energy neutrons and materials with NCrystal
    // properties, this biases the "hadElastic" process (from the ".._HP"
    // physics lists).

  public:
    NCrystalBiasingOperator();
    virtual ~NCrystalBiasingOperator();
    G4VBiasingOperation*
    ProposeOccurenceBiasingOperation(  const G4Track*,
                                       const G4BiasingProcessInterface* ) override;
    G4VBiasingOperation*
    ProposeFinalStateBiasingOperation( const G4Track*, const
                                       G4BiasingProcessInterface* ) override;
    G4VBiasingOperation*
    ProposeNonPhysicsBiasingOperation( const G4Track*,
                                       const G4BiasingProcessInterface* ) override;
  private:
    class NCrystalFinalStateBiasOp;
    class NCrystalProcess;
    bool applies( const G4Track*, const G4BiasingProcessInterface* ) const;

    NCrystalProcess* fNCrystalProcess = nullptr;
    G4BOptnChangeCrossSection* fCrossSectionBiasOp = nullptr;
    NCrystalFinalStateBiasOp* fNCrystalFinalStateBiasOp = nullptr;
  };

}

#endif
