#ifndef APARTICLEMODESETTINGS_H
#define APARTICLEMODESETTINGS_H

#include "aparticlesourcerecord.h"
#include "afilegeneratorsettings.h"
#include "aparticlerunsettings.h"
#include "asourcegeneratorsettings.h"
#include "ag4simulationsettings.h"

#include <vector>
#include <string>

#ifdef JSON11
#include "js11tools.hh"
#else
class QJsonObject;
#endif

class AParticleSimSettings
{
public:
    enum EGenMode {Sources = 0, File = 1, Script = 2};

    EGenMode GenerationMode = Sources;
    double   Events         = 1;

    ASourceGeneratorSettings SourceGenSettings;
    AFileGeneratorSettings   FileGenSettings;

    AG4SimulationSettings    G4Set;
    AParticleRunSettings     RunSet;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool exportSimulation) const;
    void readFromJson(const QJsonObject & json);  // !!!*** add error handling!
#endif

    void clearSettings();
};

#endif // APARTICLEMODESETTINGS_H
