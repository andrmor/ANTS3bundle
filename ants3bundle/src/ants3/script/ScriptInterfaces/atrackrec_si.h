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
class ATrackingDataImporter;

class ATrackRec_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ATrackRec_SI();
    ~ATrackRec_SI();

    AScriptInterface * cloneBase() const {return new ATrackRec_SI();}

public slots:
    void open(QString fileName);

    int  countEvents();
    void setEvent(int iEvent);
    bool nextEvent();

    int  countPrimaries();
    void setPrimary(int iPrimary);

    QString      recordToString(bool includeSecondaries);
    QVariantList getTrackRecord();
    QString      getProductionProcess();  // returns empty string if it is primary particle

    int  countSteps();
    int  countSecondaries();

    bool makeStep();
    void gotoFirstStep();
    void gotoLastStep();
    void gotoStep(int iStep);
    bool gotoNextProcessStep(QString processName);
    bool gotoNextNonTransportationStep();
    int  getCurrentStep();

    QVariantList getPreStepRecord();
    QVariantList getPostStepRecord();
    QVariantList getDirectionApproximate();
    bool         hadPriorInteraction();    // !!!*** suspicious step index of 2

    void enterSecondary(int indexOfSecondary);
    bool exitSecondary();

private:
    QString FileName;

    ATrackingDataImporter * Importer = nullptr;

    int NumEvents = -1;

    AEventTrackingRecord          * EventRecord = nullptr;
    const AParticleTrackingRecord * ParticleRecord = nullptr;

    int CurrentEvent = 0;
    int CurrentStep  = 0;

    std::vector<int> ReturnStepIndex;

    static constexpr auto RecordNotSet = "'tracks' unit not configured: open file, select event and (optionally, if !=0) select primary";
    static constexpr auto NoPrimaries = "This event does not have recorded primaries";

    bool checkImporter();
    bool checkImporterAndPrimary();
    QVariantList doGetStepRecord(bool flag_PreTrue_PostFalse);
};

#endif // APTHISTORY_SI_H
