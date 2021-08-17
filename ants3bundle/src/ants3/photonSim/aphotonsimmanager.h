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

public slots:
    bool simulate(int numLocalProc = -1);

private:
    bool configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
    void mergeOutput();

    const APhotonSimSettings & SimSet;

    AFileMerger          SignalFileMerger;
    AFileMerger          TrackFileMerger;
    AFileMerger          BombFileMerger;
    std::vector<QString> StatisticsFiles;

};

#endif // APHOTONSIMMANAGER_H
