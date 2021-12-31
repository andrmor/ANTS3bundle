#include "afilesettingsbase.h"
#include "ajsontools.h"

#include <QFileInfo>

bool AFileSettingsBase::isValidated() const
{
    if (FileName.isEmpty()) return false;
    if (FileFormat == Undefined || FileFormat == Invalid) return false;

    QFileInfo fi(FileName);
    if (!fi.exists()) return false;
    if (LastModified != fi.lastModified()) return false;

    return true;
}

QString AFileSettingsBase::getFormatName() const
{
    switch (FileFormat)
    {
    case Binary:  return "Binary";
    case Ascii:   return "Ascii";
    case Invalid: return "Invalid";
    default:;
    }
    return "Undefined";
}

void AFileSettingsBase::writeToJson(QJsonObject & json) const
{
    json["FileName"]     = FileName;
    json["NumEvents"]    = NumEvents;
    json["FileFormat"]   = getFormatName();
    json["LastModified"] = LastModified.toMSecsSinceEpoch();

    doWriteToJson(json);
}

void AFileSettingsBase::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "FileName",  FileName);
    jstools::parseJson(json, "NumEvents", NumEvents);

    QString fstr;
    jstools::parseJson(json, "FileFormat", fstr);
    if      (fstr == "G4Binary") FileFormat = Binary;
    else if (fstr == "G4Ascii")  FileFormat = Ascii;
    else if (fstr == "Invalid")  FileFormat = Invalid;
    else                         FileFormat = Undefined;

    qint64 lastMod;
    jstools::parseJson(json, "LastModified", lastMod);
    LastModified = QDateTime::fromMSecsSinceEpoch(lastMod);

    doReadFromJson(json);
}

void AFileSettingsBase::clear()
{
    FileName.clear();
    FileFormat   = Undefined;
    NumEvents    = 0;
    LastModified = QDateTime();

    clearStatistics();
}
