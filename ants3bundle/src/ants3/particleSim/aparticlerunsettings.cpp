#include "aparticlerunsettings.h"

#ifndef JSON11
    #include "ajsontools.h"
#endif

// ---

void ASaveParticlesSettings::clear()
{
    Enabled    = false;
    VolumeName.clear();
    StopTrack  = true;
    TimeWindow = false;
    TimeFrom   = 0;
    TimeTo     = 1e10;
}

#ifndef JSON11
void ASaveParticlesSettings::writeToJson(QJsonObject &json) const
{
    json["Enabled"]    = Enabled;
    json["FileName"]   = QString(FileName.data());
    json["VolumeName"] = QString(VolumeName.data());
    json["StopTrack"]  = StopTrack;
    json["TimeWindow"] = TimeWindow;
    json["TimeFrom"]   = TimeFrom;
    json["TimeTo"]     = TimeTo;
}
#endif

#ifdef JSON11
void ASaveParticlesSettings::readFromJson(const json11::Json::object & json)
#else
void ASaveParticlesSettings::readFromJson(const QJsonObject & json)
#endif
{
    clear();

    jstools::parseJson(json, "Enabled",    Enabled);
    jstools::parseJson(json, "FileName",   FileName);
    jstools::parseJson(json, "VolumeName", VolumeName);
    jstools::parseJson(json, "StopTrack",  StopTrack);
    jstools::parseJson(json, "TimeWindow", TimeWindow);
    jstools::parseJson(json, "TimeFrom",   TimeFrom);
    jstools::parseJson(json, "TimeTo",     TimeTo);
}

// ---

#ifndef JSON11
void AMaterialTransferRecord::writeToJson(QJsonObject & json) const
{
    json["Name"]     = QString(Name.data());
    json["GdmlName"] = QString(GdmlName.data());
    switch (Type)
    {
    case Direct :
        json["Type"] = "Direct";
        if (UseCustomMeanExEnergy) json["MeanExEnergy_eV"] = MeanExEnergy;
        break;
    case Nist :
        json["Type"] = "Nist";
        json["NistName"] = QString(NistName.data());
        break;
    case NCrystal :
        json["Type"] = "NCrystal";
        json["NCrystal"] = QString(NCrystalCfg.data());
        break;
    }
}
#endif

#ifdef JSON11
#include "aerrorhub.h"
void AMaterialTransferRecord::readFromJson(const json11::Json::object & json)
{
    jstools::parseJson(json, "Name",     Name);
    jstools::parseJson(json, "GdmlName", GdmlName);

    std::string type;
    jstools::parseJson(json, "Type", type);
    if      (type == "Direct")   Type = Direct;
    else if (type == "Nist")     Type = Nist;
    else if (type == "NCrystal") Type = NCrystal;
    else AErrorHub::addError("Unknown type \"" + type + "\" of material " + Name);

    jstools::parseJson(json, "NistName", NistName);
    jstools::parseJson(json, "NCrystal", NCrystalCfg);
    UseCustomMeanExEnergy = jstools::parseJson(json, "MeanExEnergy_eV", MeanExEnergy);
}
#endif

// ---

#ifndef JSON11
void AParticleRunSettings::writeToJson(QJsonObject & json, bool includeG4ants3Set) const
{
    json["OutputDirectory"]      = QString(OutputDirectory.data());

    json["SaveConfig"]           = SaveConfig;
    json["FileNameSaveConfig"]   = QString(FileNameSaveConfig.data());

    json["Seed"]                 = Seed;

    json["SaveTrackingHistory"]     = SaveTrackingHistory;
    json["FileNameTrackingHistory"] = QString(FileNameTrackingHistory.data());

    json["SaveDeposition"]       = SaveDeposition;
    json["FileNameDeposition"]   = QString(FileNameDeposition.data());
    {
        QJsonArray ar;
        for (const std::string & str : SaveDepositionVolumes) ar.push_back(QString(str.data()));
        json["SaveDepositionVolumes"] = ar;
    }
    json["SaveDepositionIncludeScintillators"] = SaveDepositionIncludeScintillators;
    {
        QJsonArray ar;
        for (const std::string & str : SaveDepositionScintVolumes) ar.push_back(QString(str.data()));
        json["SaveDepositionScintVolumes"] = ar;
    }

    json["AsciiOutput"]          = AsciiOutput;
    json["AsciiPrecision"]       = AsciiPrecision;

    QJsonObject sjs;
        SaveSettings.writeToJson(sjs);
    json["SaveParticles"] = sjs;

    QJsonObject mjs;
        MonitorSettings.writeToJson(mjs, includeG4ants3Set);
    json["MonitorSettings"] = mjs;

    QJsonObject cjs;
        CalorimeterSettings.writeToJson(cjs, includeG4ants3Set);
    json["CalorimeterSettings"] = cjs;

    // Particle analyzers
    {
        QJsonObject js;
        AnalyzerSettings.writeToJson(js, includeG4ants3Set);
        json["AnalyzerSettings"] = js;
    }

    if (includeG4ants3Set)
    {
        json["EventFrom"]            = EventFrom;
        json["EventTo"]              = EventTo;

        {
            QJsonArray ar;
            for (const AMaterialTransferRecord & mat : Materials)
            {
                QJsonObject js;
                mat.writeToJson(js);
                ar.push_back(js);
            }
            json["Materials"] = ar;
        }

        json["GDML"]                 = QString(GDML.data());
        json["Receipt"]              = QString(Receipt.data());

        json["GuiMode"]              = false; // to be changed by hand in the file if tests are required
    }
}
#endif

#ifdef JSON11
void AParticleRunSettings::readFromJson(const json11::Json::object & json)
#else
void AParticleRunSettings::readFromJson(const QJsonObject & json)
#endif
{
    clear();

    jstools::parseJson(json, "OutputDirectory",      OutputDirectory);

    jstools::parseJson(json, "SaveConfig",           SaveConfig);
    jstools::parseJson(json, "FileNameSaveConfig",   FileNameSaveConfig);

    jstools::parseJson(json, "Seed",                 Seed);

    jstools::parseJson(json, "SaveTrackingHistory",     SaveTrackingHistory);
    jstools::parseJson(json, "FileNameTrackingHistory", FileNameTrackingHistory);

    jstools::parseJson(json, "SaveDeposition",       SaveDeposition);
    jstools::parseJson(json, "FileNameDeposition",   FileNameDeposition);
    SaveDepositionVolumes.clear();
#ifdef JSON11
    {
        json11::Json::array ar;
        jstools::parseJson(json, "SaveDepositionVolumes", ar);
        for (size_t i = 0; i < ar.size(); i++) SaveDepositionVolumes.push_back(ar[i].string_value());
    }
    {
        json11::Json::array ar;
        jstools::parseJson(json, "SaveDepositionScintVolumes", ar);
        for (size_t i = 0; i < ar.size(); i++) SaveDepositionScintVolumes.push_back(ar[i].string_value());
    }
#else
    {
        QJsonArray ar;
        jstools::parseJson(json, "SaveDepositionVolumes", ar);
        for (int i = 0; i < ar.size(); i++) SaveDepositionVolumes.push_back(ar[i].toString().toLatin1().data());
    }
    // do not load SaveDepositionScintVolumes, filled automatically
#endif
    jstools::parseJson(json, "SaveDepositionIncludeScintillators", SaveDepositionIncludeScintillators);

#ifdef JSON11
    json11::Json::object sjs;
#else
    QJsonObject sjs;
#endif
    jstools::parseJson(json, "SaveParticles", sjs);
    SaveSettings.readFromJson(sjs);

#ifdef JSON11
    json11::Json::object mjs;
#else
    QJsonObject mjs;
#endif
    jstools::parseJson(json, "MonitorSettings", mjs);
    MonitorSettings.readFromJson(mjs);

#ifdef JSON11
    json11::Json::object cjs;
#else
    QJsonObject cjs;
#endif
    jstools::parseJson(json, "CalorimeterSettings", cjs);
    CalorimeterSettings.readFromJson(cjs);

    // Particle analyzers
    {
#ifdef JSON11
        json11::Json::object js;
#else
        QJsonObject js;
#endif
        jstools::parseJson(json, "AnalyzerSettings", js);
        AnalyzerSettings.readFromJson(js);
    }

    jstools::parseJson(json, "AsciiOutput",          AsciiOutput);
    jstools::parseJson(json, "AsciiPrecision",       AsciiPrecision);


#ifdef JSON11 // g4ants3 only!
    jstools::parseJson(json, "EventFrom",            EventFrom);
    jstools::parseJson(json, "EventTo",              EventTo);

    {
        json11::Json::array ar;
        jstools::parseJson(json, "Materials", ar);
        Materials.resize(ar.size());
        for (size_t i = 0; i < ar.size(); i++) Materials[i].readFromJson(ar[i].object_items());
    }

    jstools::parseJson(json, "GDML",                 GDML);
    jstools::parseJson(json, "Receipt",              Receipt);

    jstools::parseJson(json, "GuiMode",              GuiMode);
#endif
}

void AParticleRunSettings::clear()
{
    OutputDirectory.clear();
    SaveConfig = false;
    FileNameSaveConfig = "Config_ParticleSim.json";

    Seed = 0;

    AsciiOutput    = true;
    AsciiPrecision = 6;

    SaveTrackingHistory = false;
    FileNameTrackingHistory = "TrackingData.dat";

    SaveDeposition = false;
    FileNameDeposition = "Deposition.dat";
    SaveDepositionVolumes.clear();
    SaveDepositionIncludeScintillators = true;

    SaveSettings.clear();

    MonitorSettings.clear();

    CalorimeterSettings.clear();

    AnalyzerSettings.clear();

    EventFrom = 0;
    EventTo   = 0;

    Materials.clear();

    GDML = "";
    Receipt = "";

    GuiMode = false;
}

