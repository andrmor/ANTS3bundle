#include "afilegeneratorsettings.h"
#include "ajsontools.h"

#include <QString>
#include <QDebug>

std::string AFileGeneratorSettings::getFormatName() const
{
    /*
    switch (FileFormat)
    {
    case G4Binary:   return "G4-binary";
    case G4Ascii:    return "G4-txt";
    case Undefined:  return "?";
    case BadFormat:  return "Invalid";
    default:         return "Error";
    }
    */
    // !!!*** delegate to engine?
    return "notYetDone";
}

void AFileGeneratorSettings::writeToJson(QJsonObject &json) const
{
    json["FileName"]         = FileName.data();
    json["FileFormat"]       = FileFormat; //static_cast<int>(FileFormat);
}

void AFileGeneratorSettings::readFromJson(const QJsonObject &json)
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
}

void AFileGeneratorSettings::clear()
{
    FileName.clear();
    FileFormat         = Undefined;

    clearStatistics();
}

void AFileGeneratorSettings::clearStatistics()
{
    statNumEmptyEventsInFile = 0;
    statNumMultipleEvents    = 0;

    ParticleStat.clear();
}
