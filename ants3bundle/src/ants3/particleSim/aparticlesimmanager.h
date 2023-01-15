#ifndef APARTICLESIMMANAGER_H
#define APARTICLESIMMANAGER_H

#include "afarmnoderecord.h"
#include "afilemerger.h"

#include <QObject>
#include <QString>

#include <vector>

class AParticleSimSettings;
class ASourceParticleGenerator;
class AFileParticleGenerator;
class AGeometryHub;
class A3WorkDistrConfig;
class QJsonObject;
class AParticleRunSettings;
class AEventTrackingRecord;
class AParticleGun;

class AParticleSimManager : public QObject
{
    Q_OBJECT

public:
    static AParticleSimManager & getInstance();

private:
    AParticleSimManager();
    ~AParticleSimManager();

    AParticleSimManager(const AParticleSimManager&)            = delete;
    AParticleSimManager(AParticleSimManager&&)                 = delete;
    AParticleSimManager& operator=(const AParticleSimManager&) = delete;
    AParticleSimManager& operator=(AParticleSimManager&&)      = delete;

public:
    AParticleSimSettings & SimSet;
    const AGeometryHub   & Geometry;

    ASourceParticleGenerator * Generator_Sources = nullptr;
    AFileParticleGenerator   * Generator_File    = nullptr;
    AParticleGun             * ParticleGun       = nullptr; // alias pointing to one of the generators above

    void simulate(int numLocalProc = -1);

    void abort();
    bool isAborted() const;

    QString buildTracks(const QString & fileName, const QStringList & LimitToParticles, const QStringList & ExcludeParticles,
                        bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
                        const int MaxTracks, int LimitToEvent = -1);

    QString fillTrackingRecord(const QString & fileName, int iEvent, AEventTrackingRecord * record);

signals:
    void requestUpdateResultsGUI();

private:
    AFileMerger HistoryFileMerger;
    AFileMerger ParticlesFileMerger;
    AFileMerger DepositionFileMerger;
    std::vector<QString> MonitorFiles;
    std::vector<QString> CalorimeterFiles;

    std::vector<QString> ReceiptFiles;

    int  getNumberEvents() const;
    void doPreSimChecks();
    void checkDirectories();
    void checkG4Settings();

    bool configureSimulation(const std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
    bool configureGDML(A3WorkDistrConfig & Request, const QString & ExchangeDir);
    void configureMaterials();
    void configureMonitors();
    void configureCalorimeters();
    bool configureParticleGun();

    void removeOutputFiles();
    void mergeOutput(bool binary);
    void processReply(const QJsonObject & Reply);
};

#endif // APARTICLESIMMANAGER_H
