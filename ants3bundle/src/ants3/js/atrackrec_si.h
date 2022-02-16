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

class ATrackRec_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ATrackRec_SI();
    ~ATrackRec_SI();

public slots:
    void configure(QString fileName, bool binary);

    //void countEvents(){}
    void setEvent(int iEvent);

    int  countPrimaries();
    void setPrimary(int iPrimary);

    QString      recordToString(bool includeSecondaries);
    QVariantList getTrackRecord();
    QString      getProductionProcess();  // returns empty string if it is primary particle

    int  countSteps();
    int  countSecondaries();

    void firstStep();
    void lastStep();

    bool makeStep();
    void gotoStep(int iStep);
    bool stepToProcess(QString processName);
    bool gotoNextNonTransportationStep();
    int  getCurrentStep();

    QVariantList getStepRecord();
    QVariantList getDirectionApproximate();
    bool         hadPriorInteraction();    // !!!*** suspicious step index of 2

    void enterSecondary(int indexOfSecondary);
    bool exitSecondary();

private:
    QString FileName;
    bool    bBinaryFile;

    AEventTrackingRecord          * EventRecord = nullptr;
    const AParticleTrackingRecord * ParticleRecord = nullptr;

    int CurrentEvent = 0;
    int CurrentStep  = 0;

    std::vector<int> ReturnStepIndex;

    static constexpr auto RecordNotSet = "Track unit not configured: select file, event and (optionally, if !=0) primary";

private:
    void clearData();
};

#endif // APTHISTORY_SI_H
