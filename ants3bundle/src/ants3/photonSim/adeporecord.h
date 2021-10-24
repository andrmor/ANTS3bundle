#ifndef ADEPORECORD_H
#define ADEPORECORD_H

#include "avector.h"
#include <QString>

class ADepoRecord
{
public:
    ADepoRecord(double energy, const AVector3 & pos, double time, const QString & particle, int matIndex) :
        Energy(energy), Pos(pos), Time(time), Particle(particle), MatIndex(matIndex) {}
    ADepoRecord(){}

    double   Energy; // in keV
    AVector3 Pos;    // in mmm
    double   Time;
    QString  Particle;
    int      MatIndex;
};

#endif // ADEPORECORD_H
