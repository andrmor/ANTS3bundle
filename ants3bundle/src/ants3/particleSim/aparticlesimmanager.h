#ifndef APARTICLESIMMANAGER_H
#define APARTICLESIMMANAGER_H

#include "a3farmnoderecord.h"

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
    void addErrorLine(const QString & error);
    void checkDirectories();
    void checkG4Settings();
    bool configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
    void generateG4antsConfigCommon(const AParticleRunSettings  & RunSet, int ThreadIndex, QJsonObject & json);
};

#endif // APARTICLESIMMANAGER_H
