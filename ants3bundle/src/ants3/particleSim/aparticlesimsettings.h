#ifndef APARTICLEMODESETTINGS_H
#define APARTICLEMODESETTINGS_H

#include "aparticlesourcerecord.h"
#include "afilegeneratorsettings.h"
#include "aparticlerunsettings.h"
#include "asourcegeneratorsettings.h"
#include "ag4simulationsettings.h"

#include <QString>

#include <vector>

class QJsonObject;

class AScriptGenSettings
{
public:
    QString Script;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();
};

// -------------- Main -------------


class AParticleSimSettings
{
public:
    enum EGenMode {Sources = 0, File = 1, Script = 2};

    EGenMode GenerationMode = Sources;
    int      Events         = 1;

    bool    bDoS1           = true;  // !!!*** to run settings?
    bool    bDoS2           = false; // !!!*** to run settings?

    bool    bIgnoreNoHits   = false; // !!!*** ?
    bool    bIgnoreNoDepo   = false; // !!!*** ?

    bool    bClusterMerge   = false; // !!!*** to photon from depo settings
    double  ClusterRadius   = 0.1;   // !!!*** to photon from depo settings
    double  ClusterTime     = 1.0;   // !!!*** to photon from depo settings

    ASourceGeneratorSettings SourceGenSettings;
    AFileGeneratorSettings   FileGenSettings;
    AScriptGenSettings       ScriptGenSettings;

    AG4SimulationSettings    G4Set;
    AParticleRunSettings     RunSet;

    void writeToJson(QJsonObject & json, bool minimal = false) const;
    void readFromJson(const QJsonObject & json);

    void clearSettings();
};

#endif // APARTICLEMODESETTINGS_H
