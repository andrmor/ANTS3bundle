#include "afilegeneratorsettings.h"
#include "ajsontools.h"

#include <QFileInfo>
#include <QDebug>

bool AFileGeneratorSettings::isValidated() const
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

void AFileGeneratorSettings::invalidateFile()
{
    FileFormat         = Undefined;
    NumEventsInFile    = 0;
    ValidationMode     = None;
    LastValidationMode = None;
    FileLastModified   = QDateTime();
    ValidatedWithParticles.clear();
    clearStatistics();
}

QString AFileGeneratorSettings::getFormatName() const
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

bool AFileGeneratorSettings::isFormatG4() const
{
    return (FileFormat == G4Ascii || FileFormat == G4Binary);
}

bool AFileGeneratorSettings::isFormatBinary() const
{
    return (FileFormat == G4Binary);
}

void AFileGeneratorSettings::writeToJson(QJsonObject &json) const
{
    json["FileName"]         = FileName;
    json["FileFormat"]       = FileFormat; //static_cast<int>(FileFormat);
    json["NumEventsInFile"]  = NumEventsInFile;
    json["LastValidation"]   = LastValidationMode; //static_cast<int>(LastValidationMode);
    json["FileLastModified"] = FileLastModified.toMSecsSinceEpoch();

    QJsonArray ar; ar.fromStringList(ValidatedWithParticles);
    json["ValidatedWithParticles"] = ar;
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

void AFileGeneratorSettings::clear()
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

void AFileGeneratorSettings::clearStatistics()
{
    NumEventsInFile          = 0;
    statNumEmptyEventsInFile = 0;
    statNumMultipleEvents    = 0;

    ParticleStat.clear();
}
