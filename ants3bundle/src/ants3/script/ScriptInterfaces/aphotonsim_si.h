#ifndef APHOTONSIM_SI_H
#define APHOTONSIM_SI_H

#include "ascriptinterface.h"

#include <QVariantList>
#include <QVariantMap>
#include <QString>

class APhotonSimManager;
class TH1D;

class APhotonSim_SI : public AScriptInterface
{
    Q_OBJECT

public:
    APhotonSim_SI();
    ~APhotonSim_SI();

    AScriptInterface * cloneBase() const {return new APhotonSim_SI();}

public slots:
    void simulate();

    void setSeed(double seed);

    int countMonitors();
    void loadMonitorData(QString fileName);
    QVariantList getMonitorGlobalPositionsAll();
    QVariantList getMonitorHitsAll();
    QVariantList getMonitorWaveIndex(int monitorIndex);
    QVariantList getMonitorWavelength(int monitorIndex);
    QVariantList getMonitorTime(int monitorIndex, QString units);
    QVariantList getMonitorAngle(int monitorIndex);
    QVariantList getMonitorXY(int monitorIndex);

    void loadStatistics(QString fileName);
    QVariantMap getStatistics();
    QVariantList getStatistics_SensorAngular();
    QVariantList getStatistics_SensorWaveIndex();
    QVariantList getStatistics_SensorWavelength();
    QVariantList getStatistics_SensorTime();
    QVariantList getStatistics_SensorTransitions();

    int          mesh_build(int num);
    QVariantList mesh_getVertices();
    QVariantList mesh_getTriangles();
    int          mesh_findIndex(double x, double y, double z);
    //int          findIndexSlow(double x, double y, double z);

private:
    APhotonSimManager & SimMan;

    QVariantList getStatisticsDistribution(const TH1D * data, const QString & errorMessage);

};

#endif // APHOTONSIM_SI_H
