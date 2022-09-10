#ifndef DetectorConstruction_h
#define DetectorConstruction_h

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction(G4VPhysicalVolume *setWorld = 0);
    virtual ~DetectorConstruction();

public:
    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

private:
    G4VPhysicalVolume *fWorld;

    bool isAccordingTo(const std::string & name, const std::string & wildcard) const;
    void setStepLimiters();
    void removeVolumeNameDecorator(std::string & name); // should be synchronized with AGeometryHub: static constexpr char IndexSeparator[] = "_-_";
};

#endif // DetectorConstruction_h
