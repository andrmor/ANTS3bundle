#ifndef ATRACKINGDATAIMPORTER_H
#define ATRACKINGDATAIMPORTER_H

#include <vector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>

class AEventTrackingRecord;
class AParticleTrackingRecord;
class QFile;
class QTextStream;
class ATrackingStepData;

class ATrackingDataImporter
{
public:
    ATrackingDataImporter(const QString & fileName, bool binary, std::vector<AEventTrackingRecord*> & history);
    ~ATrackingDataImporter();

    QString processFile(int FromEvent, int ToEvent);

private:
    QString FileName;
    bool    bBinaryInput;
    std::vector<AEventTrackingRecord*> & History;

    AEventTrackingRecord    * CurrentEventRecord   = nullptr;   // history of the current event
    AParticleTrackingRecord * CurrentParticleRecord = nullptr;  // current particle - can be primary or secondary

    QMap<int, AParticleTrackingRecord*> PromisedSecondaries;   // <index in file, secondary AEventTrackingRecord *>  // *** avoid using QMap - slow!

    enum Status {ExpectingEvent, ExpectingTrack, ExpectingStep, TrackOngoing};

    Status CurrentStatus = ExpectingEvent;
    int ExpectedEvent;
    int EndEvent;

    QString Error;

    //resources for ascii input
    QFile *       inTextFile = nullptr;
    QTextStream * inTextStream = nullptr;
    QString       currentLine;
    QStringList   inputSL;

    //resources for binary input
    std::ifstream * inStream = nullptr;
    char          binHeader;
    int           BtrackId;
    int           BparentTrackId;
    std::string   BparticleName = "______________________________________________________"; //reserve long
    double        Bpos[3];
    double        Btime;
    double        BkinEnergy;
    int           BnextMat;
    std::string   BnextVolName  = "______________________________________________________"; //reserve long
    int           BnextVolIndex;
    std::string   BprocessName  = "______________________________________________________"; //reserve long
    double        BdepoEnergy;
    QVector<int>  BsecVec;

    //to speedup, maybe add QString fields to mirrow std::strings appear?

private:
    bool isEndReached() const;
    void readBuffer();
    bool isNewEvent();
    bool isNewTrack();

    void processNewEvent();
    void processNewTrack();
    void processNewStep();

    bool isErrorInPromises();

    void prepareImportResources(const QString & FileName);
    void clearImportResources();

    int  extractEventId();
    void readNewTrack();
    bool isPrimaryRecord() const;
    AParticleTrackingRecord * createAndInitParticleTrackingRecord() const;
    int  getNewTrackIndex() const;
    void updatePromisedSecondary(AParticleTrackingRecord * secrec);
    void readNewStep();
    void addHistoryStep();
    bool isTransportationStep() const;
    ATrackingStepData * createHistoryTransportationStep() const;
    ATrackingStepData * createHistoryStep() const;
    void readSecondaries();
    void readString(std::string & str) const;

};

#endif // ATRACKINGDATAIMPORTER_H
