#ifndef APARTICLEMODESETTINGS_H
#define APARTICLEMODESETTINGS_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QStringList>

#include <vector>

class QJsonObject;
class AParticleSourceRecord;
class ASourceParticleGenerator;

class ASourceGenSettings   // ! this class uses MpCollection obtained through singleton GlobalSettings. read from json for non-existing particles triggers global update!
{
public:
    QVector<AParticleSourceRecord*> ParticleSourcesData;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();

    int    getNumSources() const;

    const AParticleSourceRecord * getSourceRecord(int iSource) const;
    AParticleSourceRecord * getSourceRecord(int iSource);

    void   calculateTotalActivity();
    double getTotalActivity() const {return TotalActivity;}

    void   append(AParticleSourceRecord * gunParticle);
    bool   clone(int iSource);
    void   forget(AParticleSourceRecord * gunParticle);
    bool   replace(int iSource, AParticleSourceRecord * gunParticle);
    void   remove(int iSource);

private:
    double TotalActivity = 0;
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

    QString OutputDirectory;

    //bool    SaveSensorSignals     = true;
    //QString FileNameSensorSignals = "SensorSignals.txt";

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};


// -------------- Main -------------

#include "ag4simulationsettings.h"

class AParticleSimSettings
{
public:
    enum AGenMode   {Sources = 0, File = 1, Script = 2};
    enum AMultiMode {Constant = 0, Poisson = 1};   // !!!*** to ASourceGenSettings

    AGenMode GenerationMode = Sources;
    int     EventsToDo      = 1;

    bool    bMultiple       = false;           // !!!*** to ASourceGenSettings
    double  MeanPerEvent    = 1.0;           // !!!*** to ASourceGenSettings
    AMultiMode MultiMode    = Constant;    // !!!*** to ASourceGenSettings

    bool    bDoS1           = true;  // !!!*** to run settings?
    bool    bDoS2           = false; // !!!*** to run settings?

    bool    bIgnoreNoHits   = false; // !!!*** ?
    bool    bIgnoreNoDepo   = false; // !!!*** ?

    bool    bClusterMerge   = false; // !!!*** to output?
    double  ClusterRadius   = 0.1; // !!!*** to output?
    double  ClusterTime     = 1.0; // !!!*** to output?

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
