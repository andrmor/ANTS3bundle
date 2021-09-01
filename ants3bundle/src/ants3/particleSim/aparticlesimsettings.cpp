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
    bIgnoreNoHits  = false;
    bIgnoreNoDepo  = false;
    bClusterMerge  = false;
    ClusterRadius  = 0.1;
    ClusterTime    = 1.0;

    SourceGenSettings.clear();
    FileGenSettings.clear();
    ScriptGenSettings.clear();

    G4Set.clear();
    RunSet.clear();
}

void AParticleSimSettings::writeToJson(QJsonObject & json) const
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
    json["IgnoreNoHitsEvents"] = bIgnoreNoHits;
    json["IgnoreNoDepoEvents"] = bIgnoreNoDepo;
    json["ClusterMerge"] = bClusterMerge;
    json["ClusterMergeRadius"] = ClusterRadius;
    json["ClusterMergeTime"] = ClusterTime;


    {
        QJsonObject js;
            SourceGenSettings.writeToJson(js);
        json["GenerationFromSources"] = js;
    }

    {
        QJsonObject js;
            FileGenSettings.writeToJson(js);
        json["GenerationFromFile"] = js;
    }

    {
        QJsonObject js;
            ScriptGenSettings.writeToJson(js);
        json["GenerationFromScript"] = js;
    }
}

void AParticleSimSettings::readFromJson(const QJsonObject & json)
{
    clearSettings();

    // run
    {
        QJsonObject js;
        jstools::parseJson(json, "RunSettings", js);
        RunSet.readFromJson(js);
    }

    // geant4
    {
        QJsonObject js;
        jstools::parseJson(json, "Geant4Settings", js);
        G4Set.readFromJson(js);
    }

    QString PartGenMode = "Sources";
    jstools::parseJson(json, "ParticleGenerationMode", PartGenMode);
    if      (PartGenMode == "Sources") GenerationMode = Sources;
    else if (PartGenMode == "File")    GenerationMode = File;
    else if (PartGenMode == "Script")  GenerationMode = Script;
    else qWarning() << "Unknown particle generation mode";

    jstools::parseJson(json, "Events", Events);

    jstools::parseJson(json, "DoS1", bDoS1);
    jstools::parseJson(json, "DoS2", bDoS2);
    jstools::parseJson(json, "IgnoreNoHitsEvents", bIgnoreNoHits);
    jstools::parseJson(json, "IgnoreNoDepoEvents", bIgnoreNoDepo);
    jstools::parseJson(json, "ClusterMerge", bClusterMerge);
    jstools::parseJson(json, "ClusterMergeRadius", ClusterRadius);
    jstools::parseJson(json, "ClusterMergeTime", ClusterTime);

    // sources
    {
        QJsonObject js;
        jstools::parseJson(json, "GenerationFromSources", js);
        SourceGenSettings.readFromJson(js);
    }

    // file
    {
        QJsonObject js;
        jstools::parseJson(json, "GenerationFromFile", js);
        FileGenSettings.readFromJson(js);
    }

    // script
    {
        QJsonObject js;
        jstools::parseJson(json, "GenerationFromScript", js);
        ScriptGenSettings.readFromJson(js);
    }
}

// ------------------------------------

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

bool AFileGenSettings::isValidated() const
{
    if (FileFormat == Undefined || FileFormat == BadFormat) return false;

    switch (ValidationMode)
    {
    case None:
        return false;
    case Relaxed:
        break;         // not enforcing anything
    case Strict:
    {
//        if (LastValidationMode != Strict) return false;
//        QSet<QString> ValidatedList = QSet<QString>::fromList(ValidatedWithParticles);
//        AMaterialParticleCollection * MpCollection = AGlobalSettings::getInstance().getMpCollection();
//        QSet<QString> CurrentList = QSet<QString>::fromList(MpCollection->getListOfParticleNames());
//        if ( !CurrentList.contains(ValidatedList) ) return false;
        break;
    }
    default: qWarning() << "Unknown validation mode!";
    }

    QFileInfo fi(FileName);
    if (!fi.exists()) return false;
    if (FileLastModified != fi.lastModified()) return false;

    return true;
}

void AFileGenSettings::invalidateFile()
{
    FileFormat         = Undefined;
    NumEventsInFile    = 0;
    ValidationMode     = None;
    LastValidationMode = None;
    FileLastModified   = QDateTime();
    ValidatedWithParticles.clear();
    clearStatistics();
}

QString AFileGenSettings::getFormatName() const
{
    switch (FileFormat)
    {
    case Simplistic: return "Simplistic";
    case G4Binary:   return "G4-binary";
    case G4Ascii:    return "G4-txt";
    case Undefined:  return "?";
    case BadFormat:  return "Invalid";
    default:         return "Error";
    }
}

bool AFileGenSettings::isFormatG4() const
{
    return (FileFormat == G4Ascii || FileFormat == G4Binary);
}

bool AFileGenSettings::isFormatBinary() const
{
    return (FileFormat == G4Binary);
}

void AFileGenSettings::writeToJson(QJsonObject &json) const
{
    json["FileName"]         = FileName;
    json["FileFormat"]       = FileFormat; //static_cast<int>(FileFormat);
    json["NumEventsInFile"]  = NumEventsInFile;
    json["LastValidation"]   = LastValidationMode; //static_cast<int>(LastValidationMode);
    json["FileLastModified"] = FileLastModified.toMSecsSinceEpoch();

    QJsonArray ar; ar.fromStringList(ValidatedWithParticles);
    json["ValidatedWithParticles"] = ar;
}

void AFileGenSettings::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "FileName", FileName);

    FileFormat = Undefined;
    if (json.contains("FileFormat"))
    {
        int im;
        jstools::parseJson(json, "FileFormat", im);
        if (im >= 0 && im < 5)
            FileFormat = static_cast<FileFormatEnum>(im);
    }

    jstools::parseJson(json, "NumEventsInFile", NumEventsInFile);

    int iVal = static_cast<int>(None);
    jstools::parseJson(json, "LastValidation", iVal);
    LastValidationMode = static_cast<ValidStateEnum>(iVal);
    ValidationMode = LastValidationMode;

    qint64 lastMod;
    jstools::parseJson(json, "FileLastModified", lastMod);
    FileLastModified = QDateTime::fromMSecsSinceEpoch(lastMod);

    QJsonArray ar;
    jstools::parseJson(json, "ValidatedWithParticles", ar);
    ValidatedWithParticles.clear();
    for (int i=0; i<ar.size(); i++) ValidatedWithParticles << ar.at(i).toString();
}

void AFileGenSettings::clear()
{
    FileName.clear();
    FileFormat         = Undefined;
    NumEventsInFile    = 0;
    ValidationMode     = None;
    LastValidationMode = None;
    FileLastModified   = QDateTime();
    ValidatedWithParticles.clear();

    clearStatistics();
}

void AFileGenSettings::clearStatistics()
{
    NumEventsInFile          = 0;
    statNumEmptyEventsInFile = 0;
    statNumMultipleEvents    = 0;

    ParticleStat.clear();
}

void ASourceGenSettings::writeToJson(QJsonObject &json) const
{
    QJsonArray ja;
    for (const AParticleSourceRecord * ps : SourceData)
    {
        QJsonObject js;
        ps->writeToJson(js);
        ja.append(js);
    }
    json["ParticleSources"] = ja;

    QJsonObject js;
        js["Enabled"] = MultiEnabled;
        js["Mode"]    = ( MultiMode == Constant ? "Constant" : "Poisson" );
        js["Number"]  = MultiNumber;
    json["MultiplePerEvent"] = js;
}

void ASourceGenSettings::readFromJson(const QJsonObject &  json)
{
    clear();

    if (!json.contains("ParticleSources"))  // !*! move to outside
    {
        qWarning() << "--- Json does not contain config for particle sources!";
        return;
    }

    QJsonArray ar = json["ParticleSources"].toArray();
    if (ar.isEmpty()) return;

    for (int iSource = 0; iSource < ar.size(); iSource++)
    {
        QJsonObject js = ar.at(iSource).toObject();
        AParticleSourceRecord * ps = new AParticleSourceRecord();
        bool ok = ps->readFromJson(js);
        if (ok) SourceData.push_back(ps);
        else
        {
            qWarning() << "||| Load particle source #" << iSource << "from json failed!";
            delete ps;
        }
    }

    calculateTotalActivity();

    QJsonObject js;
    jstools::parseJson(json, "MultiplePerEvent", js);
    {
        jstools::parseJson(js, "Enabled", MultiEnabled);
        jstools::parseJson(js, "Number",  MultiNumber);

        QString strMulti;
        jstools::parseJson(js, "Mode",    strMulti);
        if (strMulti == "Poisson")
            MultiMode = Poisson;
        else
            MultiMode = Constant;
    }
}

void ASourceGenSettings::clear()
{
    for (AParticleSourceRecord * r : SourceData) delete r;
    SourceData.clear();

    TotalActivity = 0;

    MultiEnabled   = false;
    MultiNumber    = 1;
    MultiMode      = Constant;
}

int ASourceGenSettings::getNumSources() const
{
    return SourceData.size();
}

const AParticleSourceRecord * ASourceGenSettings::getSourceRecord(int iSource) const
{
    if (iSource < 0 || iSource >= SourceData.size()) return nullptr;
    return SourceData.at(iSource);
}

AParticleSourceRecord *ASourceGenSettings::getSourceRecord(int iSource)
{
    if (iSource < 0 || iSource >= SourceData.size()) return nullptr;
    return SourceData.at(iSource);
}

void ASourceGenSettings::calculateTotalActivity()
{
    TotalActivity = 0;
    for (const AParticleSourceRecord * r : SourceData)
        TotalActivity += r->Activity;
}

void ASourceGenSettings::append(AParticleSourceRecord * source)
{
    SourceData.push_back(source);
    calculateTotalActivity();
}

bool ASourceGenSettings::clone(int iSource)
{
    if (iSource < 0 || iSource >= SourceData.size()) return false;

    AParticleSourceRecord * r = SourceData.at(iSource)->clone();
    r->name += "_c";
    SourceData.insert(SourceData.begin() + iSource + 1, r);
    return true;
}

/*
void ASourceGenSettings::forget(AParticleSourceRecord *source)
{
    SourceData.removeAll(source);
    calculateTotalActivity();
}
*/

bool ASourceGenSettings::replace(int iSource, AParticleSourceRecord * source)
{
    if (iSource < 0 || iSource >= SourceData.size()) return false;

    delete SourceData[iSource];
    SourceData[iSource] = source;
    calculateTotalActivity();
    return true;
}

void ASourceGenSettings::remove(int iSource)
{
    if (SourceData.empty()) return;
    if (iSource < 0 || iSource >= SourceData.size()) return;

    delete SourceData[iSource];
    SourceData.erase(SourceData.begin() + iSource);
    calculateTotalActivity();
}

void AParticleRunSettings::writeToJson(QJsonObject &json) const
{
    json["Seed"]                 = Seed;

    json["EventFrom"]            = EventFrom;
    json["EventTo"]              = EventTo;

    json["OutputDirectory"]      = OutputDirectory;

    json["AsciiOutput"]          = AsciiOutput;
    json["AsciiPrecision"]       = AsciiPrecision;

    json["SaveTrackingData"]     = SaveTrackingData;
    json["FileNameTrackingData"] = FileNameTrackingData;
}

void AParticleRunSettings::readFromJson(const QJsonObject &json)
{
    clear();

    jstools::parseJson(json, "Seed",                 Seed);

    jstools::parseJson(json, "EventFrom",            EventFrom);
    jstools::parseJson(json, "EventTo",              EventTo);

    jstools::parseJson(json, "OutputDirectory",      OutputDirectory);

    jstools::parseJson(json, "AsciiOutput",          AsciiOutput);
    jstools::parseJson(json, "AsciiPrecision",       AsciiPrecision);

    jstools::parseJson(json, "SaveTrackingData",     SaveTrackingData);
    jstools::parseJson(json, "FileNameTrackingData", FileNameTrackingData);
}

void AParticleRunSettings::clear()
{
    AsciiOutput    = true;
    AsciiPrecision = 6;

    OutputDirectory.clear();

    SaveTrackingData = true;
    FileNameTrackingData = "TrackingData.txt";
}
