#ifndef APHOTONSIMMANAGER_H
#define APHOTONSIMMANAGER_H

#include "a3farmnoderecord.h"
#include "afilemerger.h"

#include <QObject>
#include <QString>

#include <vector>

class A3WorkDistrConfig;
class APhotonSimSettings;

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
    QString ErrorString;

    bool simulate(int numLocalProc = -1);

signals:
    void requestUpdateResultsGUI();

private:
    bool configureSimulation(const std::vector<A3FarmNodeRecord> &RunPlan, A3WorkDistrConfig & Request);
    bool checkDirectories();
    void processReply(const QJsonObject & json);
    void removeOutputFiles();  // !!!*** also remov efiles in exchange
    void mergeOutput();
    void addErrorLine(const QString & error);

    APhotonSimSettings & SimSet;

    AFileMerger          SignalFileMerger;
    AFileMerger          TrackFileMerger;
    AFileMerger          BombFileMerger;

    std::vector<QString> StatisticsFiles;
    std::vector<QString> MonitorFiles;
};

#endif // APHOTONSIMMANAGER_H
