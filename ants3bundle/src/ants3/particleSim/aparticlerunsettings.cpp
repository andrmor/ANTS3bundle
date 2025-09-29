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

        QJsonArray matAr;
            for (const auto & mat : Materials) matAr.push_back(mat.data());
        json["Materials"]            = matAr;

        {
            QJsonArray ar;
            for (const auto & mat : MaterialsFromNist)
            {
                QJsonArray el;
                el << QString(mat.first.data()) << QString(mat.second.data());
                ar.push_back(el);
            }
            json["MaterialsFromNist"] = ar;
        }

        {
            QJsonArray ar;
            for (const auto & mat : MaterialsFromNCrystal)
            {
                QJsonArray el;
                el << QString(mat.first.data()) << QString(mat.second.data());
                ar.push_back(el);
            }
            json["MaterialsFromNCrystal"] = ar;
        }

        {
            QJsonArray ar;
            for (const auto & mat : MaterialsMeanExEnergy)
            {
                QJsonArray el;
                el << QString(mat.first.data()) << mat.second;
                ar.push_back(el);
            }
            json["MaterialsWithCustomMeanExcitationEnergy"] = ar;
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

    json11::Json::array arMat;
    jstools::parseJson(json, "Materials", arMat);
    for (size_t i = 0; i < arMat.size(); i++) Materials.push_back(arMat[i].string_value());

    json11::Json::array arNist;
    jstools::parseJson(json, "MaterialsFromNist", arNist);
    for (size_t i = 0; i < arNist.size(); i++)
    {
        json11::Json::array el = arNist[i].array_items();
        MaterialsFromNist.push_back( {el[0].string_value(), el[1].string_value()} );
    }

    {
        json11::Json::array ar;
        jstools::parseJson(json, "MaterialsFromNCrystal", ar);
        for (size_t i = 0; i < ar.size(); i++)
        {
            json11::Json::array el = ar[i].array_items();
            MaterialsFromNCrystal.push_back( {el[0].string_value(), el[1].string_value()} );
        }
    }

    {
        json11::Json::array ar;
        jstools::parseJson(json, "MaterialsWithCustomMeanExcitationEnergy", ar);
        for (size_t i = 0; i < ar.size(); i++)
        {
            json11::Json::array el = ar[i].array_items();
            MaterialsMeanExEnergy.push_back( {el[0].string_value(), el[1].number_value()} );
        }
    }

    jstools::parseJson(json, "GDML",                 GDML);
    jstools::parseJson(json, "Receipt",              Receipt);

    jstools::parseJson(json, "GuiMode",              GuiMode);
#endif
}

void AParticleRunSettings::clear()
{
    OutputDirectory.clear();

    Seed = 0;

    SaveTrackingHistory = false;
    FileNameTrackingHistory = "TrackingData.dat";

    SaveDeposition = false;
    FileNameDeposition = "Deposition.dat";

    SaveSettings.clear();

    MonitorSettings.clear();

    CalorimeterSettings.clear();

    AnalyzerSettings.clear();

    AsciiOutput    = true;
    AsciiPrecision = 6;

    EventFrom = 0;
    EventTo   = 0;

    Materials.clear();

    GDML = "";
    Receipt = "";

    GuiMode = false;
}

