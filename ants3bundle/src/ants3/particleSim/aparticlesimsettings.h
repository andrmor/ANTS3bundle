#ifndef APARTICLEMODESETTINGS_H
#define APARTICLEMODESETTINGS_H

#include "aparticlesourcerecord.h"
//#include "afilegeneratorsettings.h"
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

/*
class AScriptGenSettings
{
public:
    QString Script;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();
};
*/

class AParticleSimSettings
{
public:
    enum EGenMode {Sources = 0, File = 1, Script = 2};

    EGenMode GenerationMode = Sources;
    int      Events         = 1;

    bool    bDoS1           = true;  // !!!*** to run settings?
    bool    bDoS2           = false; // !!!*** to run settings?

    bool    bIgnoreNoDepo   = false; // !!!*** ?

    bool    bClusterMerge   = false; // !!!*** to photon from depo settings
    double  ClusterRadius   = 0.1;   // !!!*** to photon from depo settings
    double  ClusterTime     = 1.0;   // !!!*** to photon from depo settings

    ASourceGeneratorSettings SourceGenSettings;
//    AFileGeneratorSettings   FileGenSettings;
//    AScriptGenSettings       ScriptGenSettings;

    AG4SimulationSettings    G4Set;
    AParticleRunSettings     RunSet;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool minimal = false) const;
    void readFromJson(const QJsonObject & json);  // !!!*** add error handling!
#endif

    void clearSettings();
};

#endif // APARTICLEMODESETTINGS_H
