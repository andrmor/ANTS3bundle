#include "aparticlerecord.h"

#include <cmath>

AParticleRecord::AParticleRecord(const std::string & particle,
                                 double x,  double y,  double z,
                                 double vx, double vy, double vz,
                                 double time, double energy) :
    particle(particle), time(time), energy(energy)
{
    r[0] = x;
    r[1] = y;
    r[2] = z;

    v[0] = vx;
    v[1] = vy;
    v[2] = vz;
}

AParticleRecord::AParticleRecord(const std::string & particle, double * position, double time, double energy) :
    particle(particle), time(time), energy(energy)
{
    for (int i = 0; i < 3; i++) r[i] = position[i];
}

void AParticleRecord::ensureUnitaryLength()
{
    double mod = 0;
    for (int i=0; i<3; i++)
        mod += ( v[i] * v[i] );

    if (mod == 1.0) return;
    mod = sqrt(mod);

    if (mod != 0)
    {
        for (int i=0; i<3; i++)
            v[i] /= mod;
    }
    else
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 1.0;
    }
}
