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

    // !!!*** load calocrimeter data
    int countCalorimeters();
    QVariantList getCalorimeterData(int calorimeterIndex, QString mode); // !!!*** update!
    QVariantList getCalorimeterProperties(int calorimeterIndex);
    void clearCalorimeterData();

    // !!!*** monitor data
    int countMonitors();
    QVariantList getMonitorHitsAll();
    QVariantList getMonitorEnergy(int monitorIndex, QString units);

private:
    AParticleSimManager & SimMan;

};

#endif // APARTICLESIM_SI_H
