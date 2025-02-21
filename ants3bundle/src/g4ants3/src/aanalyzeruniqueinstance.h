#ifndef AANALYZERUNIQUEINSTANCE_H
#define AANALYZERUNIQUEINSTANCE_H

#include "json11.hh"
#include "aparticleanalyzersettings.h"

class AHistogram1D;
class G4Step;

class AnalyzerParticleEntry
{
public:
    size_t Number = 0;
    AHistogram1D * Energy = nullptr;
};

class AAnalyzerUniqueInstance
{
public:
    AAnalyzerUniqueInstance(const AParticleAnalyzerRecord & properties, int globalIndexIfNoMerge = -1);

    bool processParticle(G4Step * step); // return stop tracking flag

    void writeToJson(json11::Json::object & json) const;

    const AParticleAnalyzerRecord & Properties;
    double EnergyFactor = 1.0;

    std::map<std::string, AnalyzerParticleEntry> ParticleMap;

    int GlobalIndexIfNoMerge = -1;
};

#endif // AANALYZERUNIQUEINSTANCE_H
