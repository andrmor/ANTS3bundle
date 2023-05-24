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
    QVariantList getCalorimeterData(int calorimeterIndex, QString mode);
    QVariantList getCalorimeterProperties(int calorimeterIndex);
    void clearCalorimeterData();

    int countMonitors();
    QVariantList getMonitorHitsAll();
    // !!!*** get unit-aware monitor energy data

private:
    AParticleSimManager & SimMan;

};

#endif // APARTICLESIM_SI_H
