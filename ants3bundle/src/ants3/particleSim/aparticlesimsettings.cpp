#include "aparticlesimsettings.h"
#include "ajsontools.h"
#include "aparticlesourcerecord.h"

#include <QDebug>
#include <QFileInfo>

void AParticleSimSettings::clearSettings()
{
    GenerationMode = Sources;
    Events     = 1;

    bDoS1          = true;
    bDoS2          = false;
    bIgnoreNoDepo  = false;
    bClusterMerge  = false;
    ClusterRadius  = 0.1;
    ClusterTime    = 1.0;

    SourceGenSettings.clear();
//    FileGenSettings.clear();
//    ScriptGenSettings.clear();

    G4Set.clear();
    RunSet.clear();
}

#ifndef JSON11
void AParticleSimSettings::writeToJson(QJsonObject & json, bool minimal) const
{
    //Run settings -> modified by the simulation manager for each process!
    {
        QJsonObject js;
        RunSet.writeToJson(js);
        json["RunSettings"] = js;
    }

    //Geant4 settings
    {
        QJsonObject js;
        G4Set.writeToJson(js);
        json["Geant4Settings"] = js;
    }

    QString s;
    switch (GenerationMode)
    {
        case Sources: s = "Sources"; break;
        case File:    s = "File";    break;
        case Script:  s = "Script";  break;
    }
    json["ParticleGenerationMode"] = s;
    json["Events"] = Events;

    json["DoS1"] = bDoS1;
    json["DoS2"] = bDoS2;
    json["IgnoreNoDepoEvents"] = bIgnoreNoDepo;
    json["ClusterMerge"] = bClusterMerge;
    json["ClusterMergeRadius"] = ClusterRadius;
    json["ClusterMergeTime"] = ClusterTime;

    if (!minimal || GenerationMode == Sources)
    {
        QJsonObject js;
            SourceGenSettings.writeToJson(js);
        json["GenerationFromSources"] = js;
    }

/*
    if (!minimal || GenerationMode == File)
    {
        QJsonObject js;
            FileGenSettings.writeToJson(js);
        json["GenerationFromFile"] = js;
    }
*/

/*
    if (!minimal || GenerationMode == Script)
    {
        QJsonObject js;
            ScriptGenSettings.writeToJson(js);
        json["GenerationFromScript"] = js;
    }
*/
}
#endif

#ifdef JSON11
void AParticleSimSettings::readFromJson(const json11::Json::object & json)
#else
void AParticleSimSettings::readFromJson(const QJsonObject & json)
#endif
{
    clearSettings();

    // run
    {
#ifdef JSON11
        json11::Json::object js;
#else
        QJsonObject js;
#endif
        jstools::parseJson(json, "RunSettings", js);
        RunSet.readFromJson(js);
    }

    // geant4
    {
#ifdef JSON11
        json11::Json::object js;
#else
        QJsonObject js;
#endif
        jstools::parseJson(json, "Geant4Settings", js);
        G4Set.readFromJson(js);
    }

    std::string PartGenMode = "Sources";
    jstools::parseJson(json, "ParticleGenerationMode", PartGenMode);
    if      (PartGenMode == "Sources") GenerationMode = Sources;
    else if (PartGenMode == "File")    GenerationMode = File;
    else if (PartGenMode == "Script")  GenerationMode = Script;
//    else qWarning() << "Unknown particle generation mode";  !!!*** Errorhandling!

    jstools::parseJson(json, "Events", Events);

    jstools::parseJson(json, "DoS1", bDoS1);
    jstools::parseJson(json, "DoS2", bDoS2);
    jstools::parseJson(json, "IgnoreNoDepoEvents", bIgnoreNoDepo);
    jstools::parseJson(json, "ClusterMerge", bClusterMerge);
    jstools::parseJson(json, "ClusterMergeRadius", ClusterRadius);
    jstools::parseJson(json, "ClusterMergeTime", ClusterTime);

    // sources
    {
#ifdef JSON11
        json11::Json::object js;
#else
        QJsonObject js;
#endif
        jstools::parseJson(json, "GenerationFromSources", js);
        SourceGenSettings.readFromJson(js);
    }

/*
    // file
    {
        QJsonObject js;
        jstools::parseJson(json, "GenerationFromFile", js);
        FileGenSettings.readFromJson(js);
    }
*/

/*
    // script
    {
        QJsonObject js;
        jstools::parseJson(json, "GenerationFromScript", js);
        ScriptGenSettings.readFromJson(js);
    }
*/
}

// ------------------------------------
/*
void AScriptGenSettings::writeToJson(QJsonObject & json) const
{
    json["Script"] = Script;
}

void AScriptGenSettings::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "Script", Script);
}

void AScriptGenSettings::clear()
{
    Script.clear();
}
*/
