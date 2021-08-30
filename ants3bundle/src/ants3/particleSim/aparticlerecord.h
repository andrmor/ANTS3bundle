#ifndef APARTICLERECORD_H
#define APARTICLERECORD_H

#include <QString>

class AParticleTrackingRecord;

class AParticleRecord
{
public:
    AParticleRecord(const QString & particle,
                    double x, double y, double z,
                    double vx, double vy, double vz,
                    double time,
                    double energy);
    AParticleRecord(){}

    QString particle;    // particle name in Geant4
    double  r[3];        // starting position, in mm
    double  v[3];        // starting direction vector (unitary)
    double  time = 0;    // starting time, in ns
    double  energy;      // staring energy, in keV

    AParticleRecord * clone(); // TODO no need?
    void ensureUnitaryLength();
    void randomDir(); // !!!*** need?

};

#endif // APARTICLERECORD_H
