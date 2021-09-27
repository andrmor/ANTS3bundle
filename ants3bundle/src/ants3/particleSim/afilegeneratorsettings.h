#ifndef AFILEGENERATORSETTINGS_H
#define AFILEGENERATORSETTINGS_H

#include <string>
#include <vector>

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
    // !!!*** delegate format handling to engine?
    enum            FileFormatEnum {Undefined = 0, BadFormat, G4Ascii, G4Binary};

    std::string     FileName;
    FileFormatEnum  FileFormat = Undefined;
    int             NumEvents  = 0;

    std::string     getFormatName() const;

    void            writeToJson(QJsonObject & json) const;
    void            readFromJson(const QJsonObject & json);

    //temporary! !!!***
    bool            isFormatBinary() const {return FileFormat == G4Binary;}

    void            clear();
    void            clearStatistics();

    //runtime
    int             statNumEmptyEventsInFile = 0;
    int             statNumMultipleEvents    = 0;
    std::vector<AParticleInFileStatRecord> ParticleStat;

private:
};


#endif // AFILEGENERATORSETTINGS_H
