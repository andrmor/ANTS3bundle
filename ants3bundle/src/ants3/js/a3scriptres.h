#ifndef A3SCRIPTRES_H
#define A3SCRIPTRES_H

class A3ParticleSimManager;
class APhotonSimManager;

struct A3ScriptRes
{
    A3ParticleSimManager * ParticleSim = nullptr;
    APhotonSimManager    * PhotonSim   = nullptr;
};

#endif // A3SCRIPTRES_H
