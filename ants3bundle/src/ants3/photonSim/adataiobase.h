#ifndef ARECORDBASE_H
#define ARECORDBASE_H

#include <fstream>

class QTextStream;
class QString;

class ADataIOBase
{
public:
    virtual ~ADataIOBase(){}

    virtual void writeAscii(QTextStream & stream) const = 0; // !!!*** check that it is not used to merge files
    virtual bool readAscii(QString & line) = 0;

    //virtual void writeBinary() const;
    virtual bool readBinary(std::ifstream & stream) = 0;

    virtual void print(QString & text) = 0;
};

#endif // ARECORDBASE_H
