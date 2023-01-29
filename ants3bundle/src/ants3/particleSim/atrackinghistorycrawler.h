#ifndef ATRACKINGHISTORYCRAWLER_H
#define ATRACKINGHISTORYCRAWLER_H

#include "aeventtrackingrecord.h"

#include <mutex>

#include <QString>
#include <QSet>
#include <QMap>

#include "TString.h"

class TH1D;
class TH2D;
class TFormula;

// --- Search processors ---

class AHistorySearchProcessor
{
public:
    virtual ~AHistorySearchProcessor(){}

    virtual void beforeSearch(){}
    virtual void afterSearch(){}

    // ---------------

    virtual void onNewEvent(){}
    virtual bool onNewTrack(const AParticleTrackingRecord & ){return false;} // master track control - the bool flag will be returned in the onTrackEnd

    virtual void onLocalStep(const ATrackingStepData & ){} // anything but transportation

    virtual void onTransitionOut(const ATrackingStepData & ){} // "from" step
    virtual void onTransitionIn (const ATrackingStepData & ){} // "from" step
    virtual void onTransition(const ATrackingStepData & , const ATrackingStepData & ){} // "fromfrom" step, "from" step - "Creation" step cannot call this method!

    virtual void onTrackEnd(bool /*bMaster*/){} // flag is the value returned by onNewTrack()
    virtual void onEventEnd(){}

    virtual AHistorySearchProcessor * clone() const = 0;

    bool isInlineSecondaryProcessing() const {return bInlineSecondaryProcessing;}
    bool isIgnoreParticleSelectors()   const {return bIgnoreParticleSelectors;}

    virtual bool mergeResuts(const AHistorySearchProcessor & other) = 0;

protected:
    bool bInlineSecondaryProcessing = false;
    bool bIgnoreParticleSelectors   = false;
};

class AHistorySearchProcessor_findParticles : public AHistorySearchProcessor
{
public:
    bool onNewTrack(const AParticleTrackingRecord & pr) override;
    void onLocalStep(const ATrackingStepData & tr) override;
    void onTrackEnd(bool) override;

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    std::map<QString, int> FoundParticles;

    void getResults(std::vector<std::pair<QString,int>> & data) const;

protected:
    QString Candidate;
    bool bConfirmed = false;
};

class AHistorySearchProcessor_findProcesses : public AHistorySearchProcessor
{
public:
    enum SelectionMode {All, WithEnergyDeposition, TrackEnd};

    AHistorySearchProcessor_findProcesses(SelectionMode Mode, bool onlyHadronic, const QString & targetIsotopeStartsFrom);
    AHistorySearchProcessor_findProcesses(){}

    void onLocalStep(const ATrackingStepData & tr) override;
    void onTransitionOut(const ATrackingStepData & tr) override;
    void onTransitionIn (const ATrackingStepData & tr) override;

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    void getResults(std::vector<std::pair<QString,int>> & data) const;

    std::map<QString, int> FoundProcesses;

protected:
    SelectionMode Mode = All;
    bool OnlyHadronic = false;
    QString TargetIsotopeStartsFrom;

    bool validateStep(const ATrackingStepData & tr) const;
};

class AHistorySearchProcessor_findHadronicChannels : public AHistorySearchProcessor
{
public:
    AHistorySearchProcessor_findHadronicChannels();

    bool onNewTrack(const AParticleTrackingRecord & pr) override;
    void onLocalStep(const ATrackingStepData & tr) override;

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    std::map<QString, int> Channels;

    void getResults(std::vector<std::pair<QString,int>> & data) const;

private:
    std::vector<std::pair<QString,QString>> Aliases;

    QString Particle;
    const AParticleTrackingRecord * TrackRecord = nullptr;

    const QString & getAlias(const QString & name);
};

class AHistorySearchProcessor_findDepositedEnergy : public AHistorySearchProcessor
{
public:
    enum CollectionMode {Individual, WithSecondaries, OverEvent};

    AHistorySearchProcessor_findDepositedEnergy(CollectionMode mode, int bins, double from = 0, double to = 0);
    AHistorySearchProcessor_findDepositedEnergy(){}
    ~AHistorySearchProcessor_findDepositedEnergy();

    void onNewEvent() override;
    bool onNewTrack(const AParticleTrackingRecord & pr) override;
    void onLocalStep(const ATrackingStepData & tr) override;
    void onTransitionOut(const ATrackingStepData & tr) override; // in Geant4 energy loss can happen on transition
    void onTrackEnd(bool bMaster) override;
    void onEventEnd() override;

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    CollectionMode Mode = Individual;
    double Depo = 0;
    TH1D * Hist = nullptr;
    bool bSecondaryTrackingStarted = false;

protected:
    virtual void clearData();
    virtual void fillDeposition(const ATrackingStepData & tr);
    virtual void fillHistogram();
};

class AHistorySearchProcessor_findDepositedEnergyTimed : public AHistorySearchProcessor_findDepositedEnergy
{
public:
    AHistorySearchProcessor_findDepositedEnergyTimed(CollectionMode mode,
                                                     int binsE, double fromE, double toE,
                                                     int binsT, double fromT, double toT);
    ~AHistorySearchProcessor_findDepositedEnergyTimed();

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    double Time = 0;
    TH2D * Hist2D = nullptr;

protected:
    void clearData() override;
    void fillDeposition(const ATrackingStepData & tr) override;
    void fillHistogram() override;
};

struct AParticleDepoStat
{
    AParticleDepoStat(int num, double sum, double sumOfSquares) : num(num), sum(sum), sumOfSquares(sumOfSquares) {}
    AParticleDepoStat(){}

    void append(double val) {num++; sum += val; sumOfSquares += val*val;}

    void merge(const AParticleDepoStat & other) {num += other.num; sum += other.sum; sumOfSquares += other.sumOfSquares;}

    int num = 0;
    double sum = 0;
    double sumOfSquares = 0;
};

class AHistorySearchProcessor_getDepositionStats : public AHistorySearchProcessor
{
public:
    bool onNewTrack(const AParticleTrackingRecord & pr) override;
    void onLocalStep(const ATrackingStepData & tr) override;
    void onTransitionOut(const ATrackingStepData & tr) override; // in Geant4 energy loss can happen on transition

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    // Particle SumDepo FractionDepo Number Mean Sigma     --> Mean!=0 if Number>1 ; Sigma !=0 if Number > 5
    double getResults(std::vector<std::tuple<QString, double, double, int, double, double> > & data) const;

    std::map<QString, AParticleDepoStat> DepoData;

protected:
    QString ParticleName;
    bool bAlreadyFound = false;
    std::map<QString, AParticleDepoStat>::iterator itParticle;
};

class AHistorySearchProcessor_getDepositionStatsTimeAware : public AHistorySearchProcessor_getDepositionStats
{
public:
    AHistorySearchProcessor_getDepositionStatsTimeAware(float timeFrom, float timeTo);

    void onLocalStep(const ATrackingStepData & tr) override;
    void onTransitionOut(const ATrackingStepData & tr) override; // in Geant4 energy loss can happen on transition

    AHistorySearchProcessor * clone() const override;

private:
    float timeFrom;
    float timeTo;
};

class AHistorySearchProcessor_findTravelledDistances : public AHistorySearchProcessor
{
public:
    AHistorySearchProcessor_findTravelledDistances(int bins, double from = 0, double to = 0);
    ~AHistorySearchProcessor_findTravelledDistances();

    bool onNewTrack(const AParticleTrackingRecord & pr) override;
    void onLocalStep(const ATrackingStepData & tr) override;
    void onTransitionOut(const ATrackingStepData & tr) override; // "from" step
    void onTransitionIn (const ATrackingStepData & tr) override; // "from" step
    void onTrackEnd(bool) override;

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override;

    float Distance = 0;
    float LastPosition[3];
    bool bStarted = false;
    TH1D * Hist = nullptr;
};

class AHistorySearchProcessor_Border : public AHistorySearchProcessor
{
public:
    AHistorySearchProcessor_Border(const QString & what,
                                   const QString & cuts,
                                   int bins, double from, double to);
    AHistorySearchProcessor_Border(const QString & what, const QString & vsWhat,
                                   const QString & cuts,
                                   int bins, double from, double to);
    AHistorySearchProcessor_Border(const QString & what, const QString & vsWhat,
                                   const QString & cuts,
                                   int bins1, double from1, double to1,
                                   int bins2, double from2, double to2);
    AHistorySearchProcessor_Border(const QString & what, const QString & vsWhat, const QString & andVsWhat,
                                   const QString & cuts,
                                   int bins1, double from1, double to1,
                                   int bins2, double from2, double to2);

    ~AHistorySearchProcessor_Border(); // !!!*** delete num hists?

    void afterSearch() override;

    // direction info can be [0,0,0] !!!
    void onTransition(const ATrackingStepData & fromfromTr, const ATrackingStepData & fromTr) override; // "from" step

    AHistorySearchProcessor * clone() const override;

    bool mergeResuts(const AHistorySearchProcessor & other) override; // !!!*** merge statistics of histograms

    QString ErrorString;  // after constructor, valid if ErrorString is empty
    bool bRequiresDirections = false;

    TFormula * formulaWhat1 = nullptr;
    TFormula * formulaWhat2 = nullptr;
    TFormula * formulaWhat3 = nullptr;
    TFormula * formulaCuts = nullptr;

    //double  x, y, z, time, energy, vx, vy, vz
    //        0  1  2    3     4      5   6   7
    double par[8];
    TH1D * Hist1D = nullptr;
    TH1D * Hist1Dnum = nullptr;
    TH2D * Hist2D = nullptr;
    TH2D * Hist2Dnum = nullptr;

private:
    TFormula * parse(QString & expr);
};


// --------------------------------------

class AFindRecordSelector
{
public:
  //track level
    bool bParticle = false;
    QString Particle;
    bool bPrimary = false;
    bool bSecondary = false;
    bool bLimitToFirstInteractionOfPrimary = false;

  //transportation
    //from
    bool bFromMat = false;
    bool bFromVolume = false;
    bool bFromVolIndex = false;
    bool bEscaping = false;
    int  FromMat = 0;
    TString FromVolume;
    int  FromVolIndex = 0;
    //to
    bool bToMat = false;
    bool bToVolume = false;
    bool bToVolIndex = false;
    bool bCreated = false;
    int  ToMat = 0;
    TString ToVolume;
    int  ToVolIndex = 0;

  //step level
    bool bMaterial = false;
    int Material = 0;

    bool bVolume = false;
    TString Volume;

    bool bVolumeIndex = false;
    int VolumeIndex = 0;

};

// !!!*** abort for multithreaded!
#include <QObject>
class ATrackingHistoryCrawler : public QObject
{
    Q_OBJECT

public:
    ATrackingHistoryCrawler(const QString & fileName) : QObject(), FileName(fileName) {}

    void find(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor, int numThreads, int eventsPerThread);

public slots:
    void abort() {bAbortRequested = true;}

private:
    enum ProcessType {Creation, Local, NormalTransportation, ExitingWorld};

    void findRecursive(const AParticleTrackingRecord & pr, const AFindRecordSelector &opt, AHistorySearchProcessor & processor) const;

    QString FileName;

    bool bAbortRequested = false;

    std::mutex CrawlerMutex;
    int NumEventsProcessed = 0;

    void findSingleThread(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor);
    void findMultithread(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor, int numThreads, int eventsPerThread);

signals:
    void reportProgress(int numEventsDone);
};

#endif // ATRACKINGHISTORYCRAWLER_H
