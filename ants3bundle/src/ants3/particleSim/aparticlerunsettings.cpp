#include "aparticlerunsettings.h"

#ifndef JSON11
    #include "ajsontools.h"
#endif

#ifndef JSON11
void AParticleRunSettings::writeToJson(QJsonObject &json) const
{
    json["OutputDirectory"]      = QString(OutputDirectory.data());

    json["Seed"]                 = Seed;

    json["SaveTrackingData"]     = SaveTrackingData;
    json["FileNameTrackingData"] = QString(FileNameTrackingData.data());

    json["AsciiOutput"]          = AsciiOutput;
    json["AsciiPrecision"]       = AsciiPrecision;

    json["EventFrom"]            = EventFrom;
    json["EventTo"]              = EventTo;

    QJsonArray matAr;
        for (const auto & mat : Materials) matAr.push_back(mat.data());
    json["Materials"]            = matAr;

    json["GDML"]                 = QString(GDML.data());

    json["GuiMode"]              = false;

    QJsonArray nistAr;
        for (const auto & mat : MaterialsFromNist)
        {
            QJsonArray el;
            el << QString(mat.first.data()) << QString(mat.second.data());
            nistAr.push_back(el);
        }
    json["MaterialsFromNist"]    = nistAr;
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

    jstools::parseJson(json, "Seed",                 Seed);

    jstools::parseJson(json, "SaveTrackingData",     SaveTrackingData);
    jstools::parseJson(json, "FileNameTrackingData", FileNameTrackingData);

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

    jstools::parseJson(json, "GDML",                 GDML);
    jstools::parseJson(json, "GuiMode",              GuiMode);


#endif
}

void AParticleRunSettings::clear()
{
    OutputDirectory.clear();

    Seed = 0;

    SaveTrackingData = true;
    FileNameTrackingData = "TrackingData.txt";

    AsciiOutput    = true;
    AsciiPrecision = 6;



    Materials.clear();
}
