#include "aparticlesimhub.h"

AParticleSimHub & AParticleSimHub::getInstance()
{
    static AParticleSimHub instance;
    return instance;
}
