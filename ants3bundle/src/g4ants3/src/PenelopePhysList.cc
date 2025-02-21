#include "PenelopePhysList.hh"

#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
//#include "G4EmStandardPhysics_option4.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4IonElasticPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"

#include "G4HadronPhysicsQGSP_BIC_HP.hh"

PenelopePhysList::PenelopePhysList(G4int ver)
{
    G4cout << "\n\n!!!!!-----   Using Custom physics list: QGSP_BIC_HP with G4EmStandardPhysics_option4 replaced with G4EmPenelopePhysics\n\n"<<G4endl;

    defaultCutValue = 0.7*CLHEP::mm;
    SetCutValue(0, "proton");
    SetVerboseLevel(ver);

    // EM Physics
    RegisterPhysics( new G4EmPenelopePhysics(ver) );

    // Synchroton Radiation & GN Physics
    RegisterPhysics( new G4EmExtraPhysics(ver) );

    // Decays
    RegisterPhysics( new G4DecayPhysics(ver) );
    RegisterPhysics( new G4RadioactiveDecayPhysics(ver) );

    // Hadron Elastic scattering
    RegisterPhysics( new G4HadronElasticPhysicsHP(ver) );

    // Hadron Physics
    RegisterPhysics(  new G4HadronPhysicsQGSP_BIC_HP(ver));

    // Stopping Physics
    RegisterPhysics( new G4StoppingPhysics(ver) );

    // Ion Physics
    RegisterPhysics( new G4IonElasticPhysics(ver) );
    RegisterPhysics( new G4IonPhysics(ver));

}
