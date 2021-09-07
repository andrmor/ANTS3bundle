#include "aparticlerunsettings.h"

#include "ajsontools.h"

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
