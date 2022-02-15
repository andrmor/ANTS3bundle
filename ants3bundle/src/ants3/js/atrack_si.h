#ifndef APTHISTORY_SI_H
#define APTHISTORY_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>

class AEventTrackingRecord;
class AParticleTrackingRecord;
class ATrackingHistoryCrawler;

class ATrack_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ATrack_SI();
    //~ATrack_SI();

public slots:
    void configure(QString fileName, bool binary);

    //void countEvents(){}
    void setEvent(int iEvent);



    void set(int iEvent, int iPrimary);

    int countPrimaries(int iEvent);
    QString recordToString(int iEvent, int iPrimary, bool includeSecondaries);

    QVariantList getTrackRecord();
    QString getProductionProcess();  // returns empty string if it is primary particle
    bool step();
    bool step(int iStep);
    bool stepToProcess(QString processName);
    int  getCurrentStep();
    void firstStep();
    bool firstNonTransportationStep();
    void lastStep();
    QVariantList getStepRecord();
    QVariantList getDirections();
    int  countSteps();
    int  countSecondaries();
    bool hadPriorInteraction();
    void in(int indexOfSecondary);
    bool out();                      // sets Step to 0   TODO: add QVector with Step before "in" and restore on "out"

private:
    QString FileName;
    bool    bBinaryFile;

    AEventTrackingRecord    * EventRecord = nullptr;
    AParticleTrackingRecord * ParticleRecord = nullptr;
    int   CurrentStep = 0;
};


#endif // APTHISTORY_SI_H
