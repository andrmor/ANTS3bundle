#ifndef ANODERECORD_H
#define ANODERECORD_H

#include "adataiobase.h"

class QString;

class ANodeRecord : public ADataIOBase
{
public:
    ANodeRecord(double x, double y, double z, double time = 0, int numPhot = -1);
    ANodeRecord(){}

    double R[3];
    double Time = 0;
    int    NumPhot = -1; // -1 means use standard numPhotons according to simulation settings

    void writeAscii(QTextStream & stream) const override;
    bool readAscii(QString & line) override;

    void print(QString & text) override;
};
#endif // ANODERECORD_H
