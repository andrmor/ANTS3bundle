#ifndef APARTICLEANALYZERSETTINGS_H
#define APARTICLEANALYZERSETTINGS_H

#include <string>
#include <vector>

#ifdef JSON11
#include "js11tools.hh"
#else
class QJsonObject;
#endif

class AParticleAnalyzerRecord
{
public:
    int         EnergyBins  = 100;
    double      EnergyFrom  = 0;
    double      EnergyTo    = 1000.0;
    std::string EnergyUnits = "keV";

    bool        UseTimeWindow  = false;
    double      TimeWindowFrom = 0;
    double      TimeWindowTo   = 100.0; // ns

    bool        StopTracking = false;

    bool        SingleInstanceForAllCopies = true;

    static bool isAllowedEnergyUnit(const std::string & str);

    // Geant4-related properties, runtime
    std::string              VolumeBaseName; // Note that instances modify the object name, so here the original name is stored
    std::vector<std::string> VolumeNames;    // Will be very short (most typical value is just one), no need to optimize

    int                      TypeIndex;   // tmp, not saved
    int                      UniqueIndex; // tmp, not saved

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool includeGeant4Features) const;
    void readFromJson(const QJsonObject & json);  // !!!*** error for Geant4 side
#endif

    void addVolumeNameIfNew(const std::string & name);
};

class AParticleAnalyzerSettings
{
public:
    AParticleAnalyzerSettings();

    bool        Enabled  = false;
    std::string FileName = "ParticleAnalyzers.json";

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool includeG4ants3Set) const;
    void readFromJson(const QJsonObject & json);
    void initFromHub();
#endif

    // --> Geant4 interface
    std::vector<AParticleAnalyzerRecord> AnalyzerTypes; // can be shared by many unique analyzers

    std::vector<size_t> UniqueToTypeLUT;   // unique index -to-> type index      ( TypeIndex[UniqueIndex] )    used by configurator in G4Ants3
    std::vector<size_t> GlobalToUniqueLUT; // global index -to-> unique index    ( UniqueIndex[GlobalIndex] )  used by SensitiveDetector in G4Ants3

    size_t NumberOfUniqueAnalyzers = 0;
    // number of types can be deduced from the size of AnalyzerTypes
    // <--

    void clear();
};

#endif // APARTICLEANALYZERSETTINGS_H
