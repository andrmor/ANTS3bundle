#ifndef ADEPORECORD_H
#define ADEPORECORD_H

#include "adataiobase.h"
#include "avector.h"

#include <QString>

class ADepoRecord : public ADataIOBase
{
public:
    ADepoRecord(double energy, const AVector3 & pos, double time, const QString & particle, int matIndex) :
        Energy(energy), Pos(pos), Time(time), Particle(particle), MatIndex(matIndex) {}
    ADepoRecord(){}

    double   Energy; // in keV
    AVector3 Pos;    // in mm
    double   Time;
    QString  Particle;
    int      MatIndex;
    int      VolIndex;

    void writeAscii(QTextStream & stream) const override;
    bool readAscii(QString & line) override;

    //void writeBinary() const override;
    //bool readBinary() override;

    void print(QString & text) override;
};

#endif // ADEPORECORD_H
