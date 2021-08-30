#include "aparticlerecord.h"

#include <cmath>

AParticleRecord::AParticleRecord(const QString & particle,
                                 double x, double y, double z,
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

AParticleRecord *AParticleRecord::clone()
{
    return new AParticleRecord(particle, r[0], r[1], r[2], v[0], v[1], v[2], time, energy);
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

#include "arandomhub.h"
void AParticleRecord::randomDir()
{
    //Sphere function of Root:
    double a = 0, b = 0, r2 = 1.0;
    while (r2 > 0.25)
    {
        a  = ARandomHub::getInstance().uniform() - 0.5;
        b  = ARandomHub::getInstance().uniform() - 0.5;
        r2 =  a*a + b*b;
    }
    v[2] = ( -1.0 + 8.0 * r2 );
    const double scale = 8.0 * sqrt(0.25 - r2);
    v[0] = a*scale;
    v[1] = b*scale;
}
