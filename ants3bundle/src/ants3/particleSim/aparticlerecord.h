#ifndef APARTICLERECORD_H
#define APARTICLERECORD_H

#include <string>

class AParticleRecord
{
public:
    AParticleRecord(const std::string & particle,
                    double x,  double y,  double z,
                    double vx, double vy, double vz,
                    double time,
                    double energy);
    AParticleRecord(){}

    std::string particle; // particle name in Geant4
    double      r[3];     // mm
    double      v[3];     // unitary vector
    double      time;     // ns
    double      energy;   // keV

    void ensureUnitaryLength();
};

#endif // APARTICLERECORD_H
