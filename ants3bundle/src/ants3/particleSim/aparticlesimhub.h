#ifndef APARTICLESIMHUB_H
#define APARTICLESIMHUB_H

#include "ag4simulationsettings.h"

#include <QObject>

class AParticleSimHub : public QObject
{
    Q_OBJECT

public:
    static AParticleSimHub & getInstance();

private:
    AParticleSimHub(){}
    ~AParticleSimHub(){}

    AParticleSimHub(const AParticleSimHub&)            = delete;
    AParticleSimHub(AParticleSimHub&&)                 = delete;
    AParticleSimHub& operator=(const AParticleSimHub&) = delete;
    AParticleSimHub& operator=(AParticleSimHub&&)      = delete;

public:
    AG4SimulationSettings RunSet;
};

#endif // APARTICLESIMHUB_H
