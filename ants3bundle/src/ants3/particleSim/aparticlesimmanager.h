#ifndef APARTICLESIMMANAGER_H
#define APARTICLESIMMANAGER_H

#include <QString>

class AParticleSimSettings;

class AParticleSimManager
{
public:
    static AParticleSimManager & getInstance();

private:
    AParticleSimManager();
    ~AParticleSimManager(){}

    AParticleSimManager(const AParticleSimManager&)            = delete;
    AParticleSimManager(AParticleSimManager&&)                 = delete;
    AParticleSimManager& operator=(const AParticleSimManager&) = delete;
    AParticleSimManager& operator=(AParticleSimManager&&)      = delete;

public:
    AParticleSimSettings & SimSet;
    QString ErrorString;

    bool simulate(int numLocalProc = -1);

private:
    void addErrorLine(const QString & error);
    bool checkDirectories();


};

#endif // APARTICLESIMMANAGER_H
