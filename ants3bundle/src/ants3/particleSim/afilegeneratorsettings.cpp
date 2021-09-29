#include "afilegeneratorsettings.h"
#include "ajsontools.h"

#include <QString>
#include <QDebug>

void AFileGeneratorSettings::setFileName(const std::string &fileName)
{
    if (FileName == fileName) return;

    clear();
    FileName = fileName;
}

#include <QFileInfo>
bool AFileGeneratorSettings::isValidated() const
{
    if (FileFormat == Undefined || FileFormat == Invalid) return false;

    QFileInfo fi(FileName.data());
    if (!fi.exists()) return false;
    if (FileLastModified != fi.lastModified()) return false;

    return true;
}

std::string AFileGeneratorSettings::getFormatName() const
{
    switch (FileFormat)
    {
    case G4Binary:   return "G4Binary";
    case G4Ascii:    return "G4Ascii";
    case Undefined:  return "Undefined";
    case Invalid:  return "Invalid";
    }
}

void AFileGeneratorSettings::writeToJson(QJsonObject &json) const
{
    json["FileName"]   = FileName.data();
    json["NumEvents"]  = NumEvents;
    json["FileFormat"] = getFormatName().data();

    json["FileLastModified"] = FileLastModified.toMSecsSinceEpoch();
}

void AFileGeneratorSettings::readFromJson(const QJsonObject &json)
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

    qint64 lastMod;
    jstools::parseJson(json, "FileLastModified", lastMod);
    FileLastModified = QDateTime::fromMSecsSinceEpoch(lastMod);
}

void AFileGeneratorSettings::clear()
{
    FileName.clear();
    FileFormat       = Undefined;
    NumEvents        = 0;
    FileLastModified = QDateTime();

    clearStatistics();
}

void AFileGeneratorSettings::clearStatistics()
{
    statNumEmptyEventsInFile = 0;
    statNumMultipleEvents    = 0;

    ParticleStat.clear();
}
