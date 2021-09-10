#ifndef APARTICLESIMMANAGER_H
#define APARTICLESIMMANAGER_H

#include "a3farmnoderecord.h"
#include "afilemerger.h"

#include <QString>

#include <vector>

class AParticleSimSettings;
class ASourceParticleGenerator;
class AGeometryHub;
class A3WorkDistrConfig;
class QJsonObject;
class AParticleRunSettings;

class AParticleSimManager
{
public:
    static AParticleSimManager & getInstance();

private:
    AParticleSimManager();
    ~AParticleSimManager();

    AParticleSimManager(const AParticleSimManager&)            = delete;
    AParticleSimManager(AParticleSimManager&&)                 = delete;
    AParticleSimManager& operator=(const AParticleSimManager&) = delete;
    AParticleSimManager& operator=(AParticleSimManager&&)      = delete;

public:
    AParticleSimSettings & SimSet;
    const AGeometryHub   & Geometry;

    QString ErrorString;

    ASourceParticleGenerator * Generator_Sources = nullptr;

    bool simulate(int numLocalProc = -1);

private:
    AFileMerger HistoryFileMerger;

    int  getNumberEvents() const;
    void addErrorLine(const QString & error);
    void doPreSimChecks();
    void checkDirectories();
    void checkG4Settings();

    bool configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
    bool configureGDML(A3WorkDistrConfig & Request, const QString & ExchangeDir);
    bool configureMonitors(A3WorkDistrConfig & Request, const QString & ExchangeDir); // !!!***
    void configureMaterials();

    void generateG4antsConfigCommon(AParticleRunSettings &RunSet, int ThreadIndex, QJsonObject & json); // !!!*** simset directly to json + custom RunSet
    void removeOutputFiles();
    void mergeOutput();
};

#endif // APARTICLESIMMANAGER_H
