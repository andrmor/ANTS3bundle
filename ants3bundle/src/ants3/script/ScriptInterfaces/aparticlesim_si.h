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

    void loadCalorimeterData(QString fileName);
    int countCalorimeters();
    QVariantList getCalorimeterGlobalPositionsAll();
    QVariantList getCalorimeterBinning(int calorimeterIndex);
    QVariantList getCalorimeterData(int calorimeterIndex);
    QVariantList getCalorimeterDataProjection(int calorimeterIndex, QString mode);
    QVariantList getCalorimeterOverEventData(int calorimeterIndex);
    void clearCalorimeterData();

    void loadMonitorData(QString fileName);
    int countMonitors();
    QVariantList getMonitorGlobalPositionsAll();
    QVariantList getMonitorHitsAll();
    QVariantList getMonitorEnergy(int monitorIndex, QString units);
    QVariantList getMonitorTime(int monitorIndex, QString units);
    QVariantList getMonitorAngle(int monitorIndex);
    QVariantList getMonitorXY(int monitorIndex);

    void loadAnalyzerData(QString fileName);
    int countAnalyzers();
    QVariantList getAnalyzerUniqueToGlobalIndex();
    QVariantList getAnalyzerPositionsByGlobalIndex();
    QVariantList getAnalyzerDataAll();

    void setTrackingHistoryFileName(QString fileName);
    void buildTracks(int maxTracks);
    void buildTracksSpecific(bool skipPrimaries, bool skipPrimNoInter, bool skipSecondaries, QVariantList limitToParticleList, QVariantList excludeParticles, int maxTracks);
    void buildTracksSingleEvent(int eventIndex);

    // reformat to use aorthopositroniumgammagenerator.h
    QVariantList getThreeGammasForPositronium(); // [ [dx1,dy1,dz1,e1], [dx2,dy2,dz2,e2], [dx3,dy3,dz1,e3] ]

private:
    AParticleSimManager & SimMan;

    QString TrackingHistoryFileName;

    void makeCandidateVectors(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies);
};

#endif // APARTICLESIM_SI_H
