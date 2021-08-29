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
                    double time, double energy,
                    int secondaryOf = -1);
    AParticleRecord(){}

    QString particle;    // Geant4 name
    double  r[3];        //starting point
    double  v[3];        //starting vector
    double  time = 0;        //time on start
    double  energy;      //staring energy
    int     secondaryOf = -1; //use in primary tracker to designate secondary particles and link to their primary ***!!! to change to bool

    AParticleRecord * clone(); // TODO no need?
    void ensureUnitaryLength();
    void randomDir(); // !!!***

    // runtime properties
    AParticleTrackingRecord * ParticleRecord = nullptr; // used only of log is on!  it is != nullptr if secondary
    bool bInteracted = false;
};

#endif // APARTICLERECORD_H
