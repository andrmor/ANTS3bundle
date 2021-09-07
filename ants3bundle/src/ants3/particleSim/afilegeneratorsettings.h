#ifndef AFILEGENERATORSETTINGS_H
#define AFILEGENERATORSETTINGS_H

#include <QString>
#include <QDateTime>
#include <QStringList>

#include <vector>

class QJsonObject;

struct AParticleInFileStatRecord
{
    //AParticleInFileStatRecord(const std::string & NameStd, double Energy);
    //AParticleInFileStatRecord(const QString     & NameQt,  double Energy);
    AParticleInFileStatRecord() {}

    std::string NameStd;
    QString     NameQt;
    int         Entries = 0;
    double      Energy  = 0;
};

class AFileGeneratorSettings
{
public:
    enum            ValidStateEnum {None = 0, Relaxed = 1, Strict = 2};  // !!!*** still need?
    enum            FileFormatEnum {Undefined = 0, BadFormat = 1, Simplistic = 2, G4Ascii = 3, G4Binary = 4};

    QString         FileName;
    FileFormatEnum  FileFormat         = Undefined;
    int             NumEventsInFile    = 0;
    ValidStateEnum  ValidationMode     = None;
    ValidStateEnum  LastValidationMode = None;
    QDateTime       FileLastModified;
    QStringList     ValidatedWithParticles;

    bool            isValidated() const;
    void            invalidateFile();    //forces the file to be inspected again during next call of Init()

    QString         getFormatName() const;
    bool            isFormatG4() const;
    bool            isFormatBinary() const;

    void            writeToJson(QJsonObject & json) const;
    void            readFromJson(const QJsonObject & json);

    void            clear();
    void            clearStatistics();

    //runtime
    int             statNumEmptyEventsInFile = 0;
    int             statNumMultipleEvents    = 0;
    std::vector<AParticleInFileStatRecord> ParticleStat;

private:
};


#endif // AFILEGENERATORSETTINGS_H
