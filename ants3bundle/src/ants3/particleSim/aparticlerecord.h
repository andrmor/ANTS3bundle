#ifndef APARTICLERECORD_H
#define APARTICLERECORD_H

#include <string>

class G4ParticleDefinition;

class AParticleRecord
{
public:
    AParticleRecord(
        #ifdef GEANT4
                    G4ParticleDefinition * particle,
        #else
                    const std::string & particle,
        #endif
                    double x,  double y,  double z,
                    double vx, double vy, double vz,
                    double time,
                    double energy);
    AParticleRecord(
        #ifdef GEANT4
                    G4ParticleDefinition * particle,
        #else
                    const std::string    & particle,
        #endif
                    double * position, double time, double energy); // direction undefined!
    AParticleRecord(){}

  #ifdef GEANT4
    G4ParticleDefinition * particle = nullptr;
  #else
    std::string            particle; // particle name in Geant4
  #endif
    double      r[3];     // mm
    double      v[3];     // unitary vector
    double      time;     // ns
    double      energy;   // keV

    void ensureUnitaryLength();
};

#endif // APARTICLERECORD_H
