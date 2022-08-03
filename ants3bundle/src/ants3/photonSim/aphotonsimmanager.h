#ifndef APHOTONSIMMANAGER_H
#define APHOTONSIMMANAGER_H

#include "a3farmnoderecord.h"
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

private:
    APhotonSimManager();
    ~APhotonSimManager(){}

    APhotonSimManager(const APhotonSimManager&)            = delete;
    APhotonSimManager(APhotonSimManager&&)                 = delete;
    APhotonSimManager& operator=(const APhotonSimManager&) = delete;
    APhotonSimManager& operator=(APhotonSimManager&&)      = delete;

public:
    bool simulate(int numLocalProc = -1);

signals:
    void requestUpdateResultsGUI();

private:
    bool configureSimulation(const std::vector<A3FarmNodeRecord> &RunPlan, A3WorkDistrConfig & Request);
    bool checkDirectories();
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
    AFileMerger          TrackFileMerger;
    AFileMerger          BombFileMerger;

    std::vector<QString> StatisticsFiles;
    std::vector<QString> MonitorFiles;
};

#endif // APHOTONSIMMANAGER_H
