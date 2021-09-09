#include "aparticlerunsettings.h"

#ifndef JSON11
    #include "ajsontools.h"
#endif

#ifndef JSON11
void AParticleRunSettings::writeToJson(QJsonObject &json) const
{
    json["Seed"]                 = Seed;

    json["EventFrom"]            = EventFrom;
    json["EventTo"]              = EventTo;

    QJsonArray matAr;
        for (const auto & mat : Materials) matAr.push_back(mat.data());
    json["Materials"]            = matAr;

    json["OutputDirectory"]      = QString(OutputDirectory.data());

    json["AsciiOutput"]          = AsciiOutput;
    json["AsciiPrecision"]       = AsciiPrecision;

    json["SaveTrackingData"]     = SaveTrackingData;
    json["FileNameTrackingData"] = QString(FileNameTrackingData.data());
}
#endif

#ifdef JSON11
void AParticleRunSettings::readFromJson(const json11::Json::object & json)
#else
void AParticleRunSettings::readFromJson(const QJsonObject & json)
#endif
{
    clear();

    jstools::parseJson(json, "Seed",                 Seed);

    jstools::parseJson(json, "EventFrom",            EventFrom);
    jstools::parseJson(json, "EventTo",              EventTo);

#ifdef JSON11 // runtime only!
    json11::Json::array arMat;
    jstools::parseJson(json, "Materials", arMat);
    for (size_t i = 0; i < arMat.size(); i++) Materials.push_back(arMat[i].string_value());
#endif

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

    Materials.clear();

    OutputDirectory.clear();

    SaveTrackingData = true;
    FileNameTrackingData = "TrackingData.txt";
}
