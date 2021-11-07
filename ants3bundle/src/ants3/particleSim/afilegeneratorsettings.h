#ifndef AFILEGENERATORSETTINGS_H
#define AFILEGENERATORSETTINGS_H

#include <string>
#include <vector>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
    #include <QDateTime>
#endif

class QJsonObject;

struct AParticleInFileStatRecord
{
    AParticleInFileStatRecord(const std::string & name, double energy);

    std::string Name;
    int         Entries = 0;
    double      Energy  = 0;
};

class AFileGeneratorSettings
{
public:
    enum            FileFormatEnum {Undefined = 0, Invalid, G4Ascii, G4Binary};

    std::string     FileName;

    FileFormatEnum  FileFormat = Undefined;
    int             NumEvents  = 0;

#ifndef JSON11
    QDateTime       FileLastModified;
#endif

    bool            isValidated() const;
    std::string     getFormatName() const;

#ifdef JSON11
    void            readFromJson(const json11::Json::object & json); // Error handling !!!***
#else
    void            writeToJson(QJsonObject & json) const;
    void            readFromJson(const QJsonObject & json); // Error handling !!!***
#endif

    //temporary! !!!***
    bool            isFormatBinary() const {return FileFormat == G4Binary;}

    void            clear();
    void            clearStatistics();

    //runtime
    int             statNumEmptyEventsInFile = 0;
    int             statNumMultipleEvents    = 0;
    std::vector<AParticleInFileStatRecord> ParticleStat;

};

#endif // AFILEGENERATORSETTINGS_H
