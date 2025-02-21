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

signals:
    void requestUpdateResultsGUI();

private:
    AFileMerger HistoryFileMerger;
    AFileMerger ParticlesFileMerger;
    AFileMerger DepositionFileMerger;
    std::vector<QString> MonitorFiles;
    std::vector<QString> CalorimeterFiles;
    std::vector<QString> AnalyzerFiles;

    std::vector<QString> ReceiptFiles;

    int  getNumberEvents() const;
    void doPreSimChecks();  // !!!*** check if there are scaled TGeo!!!
    void checkDirectories();
    void checkG4Settings();

    bool configureSimulation(const std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
    bool configureGDML(A3WorkDistrConfig & Request, const QString & ExchangeDir);
    void configureMaterials();
    void configureMonitors();
    void configureCalorimeters();
    void configureAnalyzers();
    bool configureParticleGun();
    void configureScintillators();

    void removeOutputFiles();
    void mergeOutput(bool binary);
    void processReply(const QJsonObject & Reply);
};

#endif // APARTICLESIMMANAGER_H
