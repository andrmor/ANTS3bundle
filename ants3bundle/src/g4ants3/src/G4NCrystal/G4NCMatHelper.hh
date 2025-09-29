#ifndef G4NCrystal_MatHelper_hh
#define G4NCrystal_MatHelper_hh

#include "G4Material.hh"

namespace G4NCrystal {

  //Create NCrystal-enabled G4Material directly from an configuration string
  //(see NCMatCfg.hh for format). Note that for oriented crystals (single
  //crystals), any orientations specified will be interpreted in the local frame
  //of the G4LogicalVolume in which the material is installed).:
  G4Material * createMaterial( const char * cfgstr );
  G4Material * createMaterial( const G4String& cfgstr );

  //Set/disable debug output (off by default unless NCRYSTAL_DEBUG_G4MATERIALS
  //was set when the library was loaded):
  void enableCreateMaterialVerbosity(bool = true);

  //Low level ability to add properties to existing materials directly:
  bool hasNCrystalScatterProperty( const G4Material*);
  void addNCrystalScatterProperty( G4Material*, const char* cfgstr );
  void addNCrystalScatterProperty( G4Material*, const G4String& cfgstr );

}

#endif
