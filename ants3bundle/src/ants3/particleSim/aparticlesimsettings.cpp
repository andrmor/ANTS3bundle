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
}

void AParticleSimSettings::writeToJson(QJsonObject & json) const
{
    {
        QJsonObject js;
            QString s;
                switch (GenerationMode)
                {
                case Sources: s = "Sources"; break;
                case File:    s = "File";    break;
                case Script:  s = "Script";  break;
                }
            js["ParticleGenerationMode"] = s;
            js["Events"] = Events;

            js["DoS1"] = bDoS1;
            js["DoS2"] = bDoS2;
            js["IgnoreNoHitsEvents"] = bIgnoreNoHits;
            js["IgnoreNoDepoEvents"] = bIgnoreNoDepo;
            js["ClusterMerge"] = bClusterMerge;
            js["ClusterMergeRadius"] = ClusterRadius;
            js["ClusterMergeTime"] = ClusterTime;
        json["SourceControlOptions"] = js;
    }

    SourceGenSettings.writeToJson(json);

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

    {
        QJsonObject js;
        bool ok = jstools::parseJson(json, "SourceControlOptions", js);
        if (!ok)
        {
            qWarning() << "Bad format of particle sim settings json";
            return;
        }

        QString PartGenMode = "Sources";
        jstools::parseJson(js, "ParticleGenerationMode", PartGenMode);
        if      (PartGenMode == "Sources") GenerationMode = Sources;
        else if (PartGenMode == "File")    GenerationMode = File;
        else if (PartGenMode == "Script")  GenerationMode = Script;
        else qWarning() << "Unknown particle generation mode";

        jstools::parseJson(js, "Events", Events);

        jstools::parseJson(js, "DoS1", bDoS1);
        jstools::parseJson(js, "DoS2", bDoS2);
        jstools::parseJson(js, "IgnoreNoHitsEvents", bIgnoreNoHits);
        jstools::parseJson(js, "IgnoreNoDepoEvents", bIgnoreNoDepo);
        jstools::parseJson(js, "ClusterMerge", bClusterMerge);
        jstools::parseJson(js, "ClusterMergeRadius", ClusterRadius);
        jstools::parseJson(js, "ClusterMergeTime", ClusterTime);
    }

    SourceGenSettings.readFromJson(json);

    {
        QJsonObject js;
            bool ok = jstools::parseJson(json, "GenerationFromFile", js);
        if (ok) FileGenSettings.readFromJson(js);
    }

    {
        QJsonObject js;
            bool ok = jstools::parseJson(json, "GenerationFromScript", js);
        if (ok) ScriptGenSettings.readFromJson(js);
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
    for (const AParticleSourceRecord * ps : ParticleSourcesData)
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
        if (ok) ParticleSourcesData << ps;
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
    for (AParticleSourceRecord * r : ParticleSourcesData) delete r;
    ParticleSourcesData.clear();

    TotalActivity = 0;

    MultiEnabled   = false;
    MultiNumber    = 1;
    MultiMode      = Constant;
}

int ASourceGenSettings::getNumSources() const
{
    return ParticleSourcesData.size();
}

const AParticleSourceRecord * ASourceGenSettings::getSourceRecord(int iSource) const
{
    if (iSource < 0 || iSource >= ParticleSourcesData.size()) return nullptr;
    return ParticleSourcesData.at(iSource);
}

AParticleSourceRecord *ASourceGenSettings::getSourceRecord(int iSource)
{
    if (iSource < 0 || iSource >= ParticleSourcesData.size()) return nullptr;
    return ParticleSourcesData.at(iSource);
}

void ASourceGenSettings::calculateTotalActivity()
{
    TotalActivity = 0;
    for (const AParticleSourceRecord * r : ParticleSourcesData)
        TotalActivity += r->Activity;
}

void ASourceGenSettings::append(AParticleSourceRecord * gunParticle)
{
    ParticleSourcesData.append(gunParticle);
    calculateTotalActivity();
}

bool ASourceGenSettings::clone(int iSource)
{
    if (iSource < 0 || iSource >= ParticleSourcesData.size()) return false;

    AParticleSourceRecord * r = ParticleSourcesData.at(iSource)->clone();
    r->name += "_c";
    ParticleSourcesData.insert(iSource + 1, r);
    return true;
}

void ASourceGenSettings::forget(AParticleSourceRecord *gunParticle)
{
    ParticleSourcesData.removeAll(gunParticle);
    calculateTotalActivity();
}

bool ASourceGenSettings::replace(int iSource, AParticleSourceRecord * gunParticle)
{
    if (iSource < 0 || iSource >= ParticleSourcesData.size()) return false;

    delete ParticleSourcesData[iSource];
    ParticleSourcesData[iSource] = gunParticle;
    calculateTotalActivity();
    return true;
}

void ASourceGenSettings::remove(int iSource)
{
    if (ParticleSourcesData.isEmpty()) return;
    if (iSource < 0 || iSource >= ParticleSourcesData.size()) return;

    delete ParticleSourcesData[iSource];
    ParticleSourcesData.remove(iSource);
    calculateTotalActivity();
}

void AParticleRunSettings::writeToJson(QJsonObject &json) const
{

}

void AParticleRunSettings::readFromJson(const QJsonObject &json)
{

}
