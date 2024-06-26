#ifndef ANODERECORD_H
#define ANODERECORD_H

#include "adataiobase.h"

class QString;

class ANodeRecord : public ADataIOBase
{
public:
    ANodeRecord(double x, double y, double z, double time = 0, int numPhot = 0);
    ANodeRecord(){}

    double R[3];
    double Time = 0;
    int    NumPhot = 0;

    void writeAscii(QTextStream & stream) const override;
    bool readAscii(QString & line) override;

    //void writeBinary() const override;
    bool readBinary(std::ifstream & stream) override;

    void print(QString & text) override;
};
#endif // ANODERECORD_H
