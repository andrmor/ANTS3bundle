#ifndef APARTICLERECORD_H
#define APARTICLERECORD_H

#include <string>

class G4ParticleDefinition;

class AParticleRecord
{
public:
    AParticleRecord(const std::string & particle,
                    double x,  double y,  double z,
                    double vx, double vy, double vz,
                    double time,
                    double energy);
    AParticleRecord(const std::string & particle, double * position, double time, double energy); // direction undefined!
    AParticleRecord(){}

    std::string particle; // particle name in Geant4
    double      r[3];     // mm
    double      v[3];     // unitary vector
    double      time;     // ns
    double      energy;   // keV

    void ensureUnitaryLength();

    // run-time
    G4ParticleDefinition * DefinitionG4 = nullptr;
};

#endif // APARTICLERECORD_H
