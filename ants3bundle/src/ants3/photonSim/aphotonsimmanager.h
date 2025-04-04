#ifndef APHOTONSIMMANAGER_H
#define APHOTONSIMMANAGER_H

#include "afarmnoderecord.h"
#include "afilemerger.h"

#include <QObject>
#include <QString>

#include <vector>

class A3WorkDistrConfig;
class APhotonSimSettings;
class AFileHandlerBase;
class A3NodeWorkerConfig;

class APhotonSimManager final : public QObject
{
    Q_OBJECT

public:
    static       APhotonSimManager & getInstance();
    static const APhotonSimManager & getConstInstance();

    void setSeed(double seed);

    bool simulate(int numLocalProc = -1);

    void abort();

    bool isAborted() const;

private:
    APhotonSimManager();
    ~APhotonSimManager(){}

    APhotonSimManager(const APhotonSimManager&)            = delete;
    APhotonSimManager(APhotonSimManager&&)                 = delete;
    APhotonSimManager& operator=(const APhotonSimManager&) = delete;
    APhotonSimManager& operator=(APhotonSimManager&&)      = delete;

signals:
    void requestUpdateResultsGUI();

private:
    bool configureSimulation(const std::vector<AFarmNodeRecord> &RunPlan, A3WorkDistrConfig & Request);
    bool checkDirectories();
    bool updateRuntimeForFunctionalModels();
    void processReply(const QJsonObject & json);
    void removeOutputFiles();  // !!!*** also remove files in exchange
    void clearFileMergers();
    void mergeOutput();

    AFileHandlerBase * makeInputFileHandler();
    void configureOutputFiles(A3NodeWorkerConfig & Worker, APhotonSimSettings & WorkSet, int iProcess);
    void makeWorkerConfigFile(A3NodeWorkerConfig & Worker, APhotonSimSettings & WorkSet, int iProcess);

    APhotonSimSettings & SimSet;

    AFileMerger          SignalFileMerger;
    AFileMerger          SensorLogFileMerger;
    AFileMerger          PhotonLogFileMerger;
    AFileMerger          TrackFileMerger;
    AFileMerger          BombFileMerger;

    std::vector<QString> StatisticsFiles;
    std::vector<QString> MonitorFiles;
    std::vector<QString> ReceiptFiles;
};

#endif // APHOTONSIMMANAGER_H
