#ifndef G4NCrystal_Install_hh
#define G4NCrystal_Install_hh

namespace G4NCrystal {

  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // Call just after initialising the G4 run manager, in order to modify the  //
  // physics processes of neutrons and let any "NCrystal" properties          //
  // associated to the G4Materials take over the elastic hadronic physics     //
  // below 2eV (this is thus a run-time physics-list modification, and is an  //
  // alternative to the usual approach of hard-coded physics lists):          //
  //                                                                          //
  // NB: For this to work, your physics list must have installed exactly one  //
  //    active process derived from G4HadronElasticProcess for neutrons.      //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////

  void install();

  //Second version of install() which does nothing if no materials in the active
  //geometry has an "NCrystal" property (useful for framework implementers):

  void installOnDemand();

}

#endif
