#ifndef APHOTONSIM_SI_H
#define APHOTONSIM_SI_H

#include "ascriptinterface.h"

#include <QVariantList>
#include <QString>

class APhotonSimManager;

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



private:
    APhotonSimManager & SimMan;

};

#endif // APHOTONSIM_SI_H
