#ifndef SensitiveDetector_h
#define SensitiveDetector_h

#include "G4VSensitiveDetector.hh"
#include "json11.hh"

class G4Step;
class G4HCofThisEvent;

class SensitiveDetector : public G4VSensitiveDetector
{
public:
    SensitiveDetector(const G4String & name);
    virtual ~SensitiveDetector();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
};

class AHistogram1D;
class AHistogram2D;

class MonitorSensitiveDetector : public G4VSensitiveDetector
{
public:
    MonitorSensitiveDetector(const std::string & name, const std::string & particle, int index);
    virtual ~MonitorSensitiveDetector();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

    void readFromJson(const json11::Json & json);
    void writeToJson(json11::Json::object & json);

    std::string Name;
    std::string ParticleName;
    int         MonitorIndex;

    G4ParticleDefinition * pParticleDefinition = nullptr;

    bool bAcceptLower;
    bool bAcceptUpper;
    bool bAcceptDirect;
    bool bAcceptIndirect;
    bool bAcceptPrimary;
    bool bAcceptSecondary;
    bool bStopTracking;

    int     angleBins;
    double  angleFrom;
    double  angleTo;

    int     energyBins;
    double  energyFrom;
    double  energyTo;
    int     energyUnits; // 0,1,2,3 -> meV, eV, keV, MeV;

    int     timeBins;
    double  timeFrom;
    double  timeTo;

    int     xbins;
    int     ybins;
    double  size1;
    double  size2;

    //run-time
    AHistogram1D * hTime     = nullptr;
    AHistogram1D * hAngle    = nullptr;
    AHistogram1D * hEnergy   = nullptr;
    AHistogram2D * hPosition = nullptr;

protected:
    void writeHist1D(AHistogram1D *hist, json11::Json::object & json) const;
};

#endif // SensitiveDetector_h
