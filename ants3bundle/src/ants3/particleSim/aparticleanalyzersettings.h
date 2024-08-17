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

    bool        SingleInstanceForAllCopies = false;

    static bool isAllowedEnergyUnit(const std::string & str);

    // Geant4-related properties, runtime
    std::vector<std::string> VolumeNames;
    int                      UniqueIndex;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool includeGeant4Features) const;
    void readFromJson(const QJsonObject & json);  // !!!*** error for Geant4 side
#endif
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

    std::vector<AParticleAnalyzerRecord> Analyzers;

    void clear();
};

#endif // APARTICLEANALYZERSETTINGS_H
