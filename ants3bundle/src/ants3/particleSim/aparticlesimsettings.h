#ifndef APARTICLEMODESETTINGS_H
#define APARTICLEMODESETTINGS_H

#include "aparticlesourcerecord.h"

#include <QString>
#include <QDateTime>
#include <QStringList>

#include <vector>

class QJsonObject;
class ASourceParticleGenerator;

// TODO !!!*** error handling

class ASourceGenSettings
{
public:
    enum EMultiMode {Constant = 0, Poisson = 1};

    std::vector<AParticleSourceRecord> SourceData;

    bool       MultiEnabled = false;
    EMultiMode MultiMode    = Constant;
    double     MultiNumber  = 1.0;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json); // Error handling !!!***

    void   clear();

    int    getNumSources() const;
    double calculateTotalActivity() const;

    std::string check() const;

    void   append(AParticleSourceRecord & source);
    bool   clone(int iSource);
    //void   forget(AParticleSourceRecord * source);
    bool   replace(int iSource, AParticleSourceRecord & source);
    void   remove(int iSource);

};

struct AParticleInFileStatRecord
{
    AParticleInFileStatRecord(const std::string & NameStd, double Energy);
    AParticleInFileStatRecord(const QString     & NameQt,  double Energy);
    AParticleInFileStatRecord() {}

    std::string NameStd;
    QString     NameQt;
    int         Entries = 0;
    double      Energy  = 0;
};

class AFileGenSettings
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

class AScriptGenSettings
{
public:
    QString Script;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();
};

class AParticleRunSettings
{
public:
    int     Seed = 0;

    int     EventFrom = 0;
    int     EventTo   = 0;

    bool    AsciiOutput    = true;
    int     AsciiPrecision = 6;

    QString OutputDirectory;

    bool    SaveTrackingData = true;
    QString FileNameTrackingData = "TrackingData.txt";

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();

    QString getGdmlFileName() const {return "Detector.gdml";}
    QString getPrimariesFileName(int index) const {return QString("primaries-%1.txt").arg(index); }
};


// -------------- Main -------------

#include "ag4simulationsettings.h"

class AParticleSimSettings
{
public:
    enum EGenMode   {Sources = 0, File = 1, Script = 2};

    EGenMode GenerationMode = Sources;
    int      Events         = 1;

    bool    bDoS1           = true;  // !!!*** to run settings?
    bool    bDoS2           = false; // !!!*** to run settings?

    bool    bIgnoreNoHits   = false; // !!!*** ?
    bool    bIgnoreNoDepo   = false; // !!!*** ?

    bool    bClusterMerge   = false; // !!!*** to photon from depo settings
    double  ClusterRadius   = 0.1;   // !!!*** to photon from depo settings
    double  ClusterTime     = 1.0;   // !!!*** to photon from depo settings

    ASourceGenSettings SourceGenSettings;
    AFileGenSettings   FileGenSettings;
    AScriptGenSettings ScriptGenSettings;

    AG4SimulationSettings G4Set;
    AParticleRunSettings  RunSet;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clearSettings();

};

#endif // APARTICLEMODESETTINGS_H
