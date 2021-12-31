#ifndef ASENSORSIGNALARRAY_H
#define ASENSORSIGNALARRAY_H

#include "adataiobase.h"

#include <vector>

class ASensorSignalArray : public ADataIOBase
{
public:
    ASensorSignalArray(){}

    std::vector<float> Signals;

    void writeAscii(QTextStream & stream) const override;
    bool readAscii(QString & line) override;

    //void writeBinary() const override;
    //bool readBinary() override;

    void print(QString & text) override;
};

#endif // ASENSORSIGNALARRAY_H
