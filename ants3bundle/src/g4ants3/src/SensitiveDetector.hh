#ifndef SensitiveDetector_h
#define SensitiveDetector_h

#include "G4VSensitiveDetector.hh"
#include "json11.hh"

class G4Step;
class G4HCofThisEvent;
class AHistogram3Dfixed;

namespace SensitiveDetectorTools
{
    void stopAndKill(G4Step * step);
};

class DepositionSensitiveDetector : public G4VSensitiveDetector
{
public:
    DepositionSensitiveDetector(const G4String & name);
    ~DepositionSensitiveDetector();

    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
};

class MonitorSensitiveDetector : public G4VSensitiveDetector
{
public:
    MonitorSensitiveDetector(const std::string & name);

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history) override;
};

class AHistogram1D;
class AHistogram2D;

class MonitorSensitiveDetectorWrapper
{
public:
    MonitorSensitiveDetectorWrapper(const std::string & name, const std::string & particle, int index);

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history);

    bool readFromJson(const json11::Json & json);
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
//    int     energyUnits; // 0,1,2,3 -> meV, eV, keV, MeV;
    std::string EnergyUnits; // meV, eV, keV, MeV
    double  EnergyFactor = 1.0;

    int     timeBins;
    double  timeFrom;
    double  timeTo;
    std::string TimeUnits; // ns, us, ms, s
    double  TimeFactor = 1.0;

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

class CalorimeterSensitiveDetector : public G4VSensitiveDetector
{
public:
    CalorimeterSensitiveDetector(const std::string & name);

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history) override;
};

// !!!*** optimisation is possible: do not compute local coordinates in DepoOverEvent mode
#include "acalsettings.h"
class CalorimeterSensitiveDetectorWrapper
{
public:
    CalorimeterSensitiveDetectorWrapper(const std::string & name, ACalorimeterProperties & properties, int index);
    ~CalorimeterSensitiveDetectorWrapper();

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history);

    void registerHit(double depo, const G4ThreeVector & local, G4Step * step); // called by both ProcessHits and externally from DelegatingCalorimeterSensitiveDetector

    void writeToJson(json11::Json::object & json);

    std::string Name;
    ACalorimeterProperties & Properties;
    int CalorimeterIndex;

    //run-time
    AHistogram3Dfixed * Data = nullptr;
    double VoxelVolume_mm3 = 0;

    AHistogram1D * EventDepoData = nullptr;
    double SumDepoOverEvent = 0;
};

class DelegatingCalorimeterSensitiveDetector : public G4VSensitiveDetector
{
public:
    DelegatingCalorimeterSensitiveDetector(const std::string & name);

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history) override;
};

class AnalyzerSensitiveDetector : public G4VSensitiveDetector
{
public:
    AnalyzerSensitiveDetector(const std::string & name);

    G4bool ProcessHits(G4Step * step, G4TouchableHistory * history) override;

    bool AnalyzeCreated = false;
};

#endif // SensitiveDetector_h
