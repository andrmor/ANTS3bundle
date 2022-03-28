#ifndef APARTANALYSIS_SI_H
#define APARTANALYSIS_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>

class AEventTrackingRecord;
class AParticleTrackingRecord;
class ATrackingHistoryCrawler;
class AFindRecordSelector;
class AHistorySearchProcessor_findDepositedEnergy;
class AHistorySearchProcessor_Border;

class APartAnalysis_SI : public AScriptInterface
{
    Q_OBJECT

public:
    APartAnalysis_SI();
    ~APartAnalysis_SI();

    //bool InitOnRun() override;
    //void ForceStop() override;

public slots:
    void configure(QString fileName, bool binary, int numThreads = -1);

    void clearCriteria();
    void setParticle(QString particleName);
    void setOnlyPrimary();
    void setOnlySecondary();
    void setLimitToFirstInteractionOfPrimary();
    void setMaterial(int matIndex);
    void setVolume(QString volumeName);
    void setVolumeIndex(int volumeIndex);

    void setFromMaterial(int matIndex);
    void setToMaterial(int matIndex);
    void setFromVolume(QString volumeName);
    void setToVolume(QString volumeName);
    void setFromIndex(int volumeIndex);
    void setToIndex(int volumeIndex);
    void setOnlyCreated();
    void setOnlyEscaping();

    void test(int numThreads);

    QVariantList findParticles();
    QVariantList findProcesses(int All0_WithDepo1_TrackEnd2 = 0, bool onlyHadronic = false, QString targetIsotopeStartsFrom = "");
    QVariantList findDepositedEnergies(int bins, double from, double to);
    QVariantList findDepositedEnergiesWithSecondaries(int bins, double from, double to);
    QVariantList findDepositedEnergiesOverEvent(int bins, double from, double to);
    QVariantList findDepositedEnergyStats();
    QVariantList findDepositedEnergyStats(double timeFrom, double timeTo);
    QVariantList findTravelledDistances(int bins, double from, double to);

    QVariantList findhadronicChannels();

    QVariantList findOnBorder(QString what, QString cuts, int bins, double from, double to);
    QVariantList findOnBorder(QString what, QString vsWhat, QString cuts, int bins, double from, double to);
    QVariantList findOnBorder(QString what, QString vsWhat, QString cuts, int bins1, double from1, double to1, int bins2, double from2, double to2);
    QVariantList findOnBorder(QString what, QString vsWhat, QString andVsWhat, QString cuts, int bins1, double from1, double to1, int bins2, double from2, double to2);

private:
    QString FileName;
    bool    bBinaryFile;
    int     NumThreads = -1;

    ATrackingHistoryCrawler * Crawler  = nullptr;
    AFindRecordSelector     * Criteria = nullptr;

    bool initCrawler();
    QVariantList findDepE(AHistorySearchProcessor_findDepositedEnergy & p);
    const QVariantList findOB_1D(AHistorySearchProcessor_Border & p);
    const QVariantList findOB_2D(AHistorySearchProcessor_Border & p);
};

#endif // APARTANALYSIS_SI_H
