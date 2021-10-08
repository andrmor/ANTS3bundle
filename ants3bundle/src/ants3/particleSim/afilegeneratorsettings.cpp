#include "afilegeneratorsettings.h"
#include "ajsontools.h"

#ifndef JSON11
#include <QString>
#include <QFileInfo>
#include <QDebug>
#endif

bool AFileGeneratorSettings::isValidated() const
{
    if (FileFormat == Undefined || FileFormat == Invalid) return false;

#ifndef JSON11
    QFileInfo fi(FileName.data());
    if (!fi.exists()) return false;
    if (FileLastModified != fi.lastModified()) return false;
#endif

    return true;
}

std::string AFileGeneratorSettings::getFormatName() const
{
    switch (FileFormat)
    {
    case G4Binary: return "G4Binary";
    case G4Ascii:  return "G4Ascii";
    case Invalid:  return "Invalid";
    default:;
    }
    return "Undefined";
}

#ifndef JSON11
void AFileGeneratorSettings::writeToJson(QJsonObject &json) const
{
    json["FileName"]   = FileName.data();
    json["NumEvents"]  = NumEvents;
    json["FileFormat"] = getFormatName().data();

    json["FileLastModified"] = FileLastModified.toMSecsSinceEpoch();
}
#endif

#ifdef JSON11
void AFileGeneratorSettings::readFromJson(const json11::Json::object &json)
#else
void AFileGeneratorSettings::readFromJson(const QJsonObject & json)
#endif
{
    clear();

    jstools::parseJson(json, "FileName",  FileName);
    jstools::parseJson(json, "NumEvents", NumEvents);

    std::string fstr;
    jstools::parseJson(json, "FileFormat", fstr);
    if      (fstr == "G4Binary") FileFormat = G4Binary;
    else if (fstr == "G4Ascii")  FileFormat = G4Ascii;
    else if (fstr == "Invalid")  FileFormat = Invalid;
    else                         FileFormat = Undefined;

#ifndef JSON11
    qint64 lastMod;
    jstools::parseJson(json, "FileLastModified", lastMod);
    FileLastModified = QDateTime::fromMSecsSinceEpoch(lastMod);
#endif
}

void AFileGeneratorSettings::clear()
{
    FileName.clear();
    FileFormat       = Undefined;
    NumEvents        = 0;

#ifndef JSON11
    FileLastModified = QDateTime();
#endif

    clearStatistics();
}

void AFileGeneratorSettings::clearStatistics()
{
    statNumEmptyEventsInFile = 0;
    statNumMultipleEvents    = 0;

    ParticleStat.clear();
}
