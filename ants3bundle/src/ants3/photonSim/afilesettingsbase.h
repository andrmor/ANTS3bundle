#ifndef AFILESETTINGSBASE_H
#define AFILESETTINGSBASE_H

#include <QString>
#include <QDateTime>

class QJsonObject;

class AFileSettingsBase
{
public:
    virtual ~AFileSettingsBase(){}

    enum           FileFormatEnum {Undefined = 0, Invalid, Ascii, Binary};

    QString        FileName;
    FileFormatEnum FileFormat = Undefined;
    int            NumEvents  = 0;
    QDateTime      LastModified;

    bool           isValidated() const;
    QString        getFormatName() const;

    void           writeToJson(QJsonObject & json) const;   // all specifics are handled by doWriteToJson
    void           readFromJson(const QJsonObject & json);  // all specifics are handled by doReadFromJson

    void           clear();
    virtual void   clearStatistics(){}

protected:
    virtual void   doWriteToJson(QJsonObject & /*json*/) const {}
    virtual void   doReadFromJson(const QJsonObject & /*json*/) {}
};

#endif // AFILESETTINGSBASE_H
