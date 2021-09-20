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
    ATrackingDataImporter(const QString & fileName, bool binary);
    ~ATrackingDataImporter();

    bool extractEvent(int iEvent, AEventTrackingRecord * EventRecord);
    bool isEndReached() const;

    QString ErrorString;

private:
    bool gotoEvent(int iEvent);
    bool readCurrentEvent();

    bool processFile(bool SeekMode = false);

    QString FileName;
    bool    bBinaryInput;

    AEventTrackingRecord    * CurrentEventRecord    = nullptr;
    AParticleTrackingRecord * CurrentParticleRecord = nullptr;  // current particle - can be primary or secondary

    QMap<int, AParticleTrackingRecord*> PromisedSecondaries;   // <index in file, secondary AEventTrackingRecord *>  // *** avoid using QMap - slow!

    enum EStatus {ExpectingEvent, ExpectingTrack, ExpectingStep, TrackOngoing, Initialization};

    EStatus CurrentStatus = ExpectingEvent;

    int CurrentEvent = -1;

    int SeekEvent = 0;

    int FileEndEvent = -1;

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
    void readBuffer();
    bool isNewEvent();
    bool isNewTrack();

    void processNewEvent(bool SeekMode = false);
    void processNewTrack(bool SeekMode);
    void processNewStep(bool SeekMode);

    bool isErrorInPromises();

    void prepareImportResources(const QString & FileName);
    void clearImportResources();

    int  extractEventId();
    void readNewTrack(bool SeekMode);
    bool isPrimaryRecord() const;
    AParticleTrackingRecord * createAndInitParticleTrackingRecord() const;
    int  getNewTrackIndex() const;
    void updatePromisedSecondary(AParticleTrackingRecord * secrec);
    void readNewStep(bool SeekMode);
    void addHistoryStep();
    bool isTransportationStep() const;
    ATrackingStepData * createHistoryTransportationStep() const;
    ATrackingStepData * createHistoryStep() const;
    void readSecondaries();
    void readString(std::string & str) const;

};

#endif // ATRACKINGDATAIMPORTER_H
