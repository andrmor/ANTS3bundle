#ifndef APARTICLESIM_SI_H
#define APARTICLESIM_SI_H

#include "ascriptinterface.h"

#include <QVariantList>

class AParticleSimManager;

class AParticleSim_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AParticleSim_SI();

    AScriptInterface * cloneBase() const {return new AParticleSim_SI();}

public slots:
    void simulate(bool updateGui);

    int countCalorimeters();
    void loadCalorimeterData(QString fileName);
    QVariantList getCalorimeterGlobalPositionsAll();
    //QVariantList getCalorimeterData(int calorimeterIndex, QString mode); // !!!*** update!
    QVariantList getCalorimeterData(int calorimeterIndex);
    QVariantList getCalorimeterOverEventData(int calorimeterIndex);
    QVariantList getCalorimeterBinning(int calorimeterIndex);
    void clearCalorimeterData();

    int countMonitors();
    void loadMonitorData(QString fileName);
    QVariantList getMonitorGlobalPositionsAll();
    QVariantList getMonitorHitsAll();
    QVariantList getMonitorEnergy(int monitorIndex, QString units);
    QVariantList getMonitorTime(int monitorIndex, QString units);
    QVariantList getMonitorAngle(int monitorIndex);
    QVariantList getMonitorXY(int monitorIndex);

private:
    AParticleSimManager & SimMan;

};

#endif // APARTICLESIM_SI_H
