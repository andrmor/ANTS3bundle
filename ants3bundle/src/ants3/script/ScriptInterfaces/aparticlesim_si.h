#ifndef APARTICLESIM_SI_H
#define APARTICLESIM_SI_H

#include "ascriptinterface.h"

#include <QVariantList>

#include <array>

class AParticleSimManager;
class AVector3;

class AParticleSim_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AParticleSim_SI();

    AScriptInterface * cloneBase() const {return new AParticleSim_SI();}

public slots:
    void simulate();

    void setSeed(double seed);

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

    QVariantList getThreeGammasForPositronium(); // [ [dx1,dy1,dz1,e1], [dx2,dy2,dz2,e2], [dx3,dy3,dz1,e3] ]

private:
    AParticleSimManager & SimMan;

    void makeCandidateVectors(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies);
};

#endif // APARTICLESIM_SI_H
