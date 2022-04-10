#ifndef APARTICLESIM_SI_H
#define APARTICLESIM_SI_H

#include "ascriptinterface.h"

class AParticleSimManager;

class AParticleSim_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AParticleSim_SI();

public slots:
    void simulate(bool updateGui);

private:
    AParticleSimManager & SimMan;

};

#endif // APARTICLESIM_SI_H
