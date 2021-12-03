#ifndef ARECORDBASE_H
#define ARECORDBASE_H

class QTextStream;
class QString;

class ADataIOBase
{
public:
    virtual ~ADataIOBase(){}

    virtual void writeAscii(QTextStream & stream) const = 0;
    virtual bool readAscii(QString & line) = 0;

    //virtual void writeBinary() const;
    //virtual bool readBinary();

    virtual void print(QString & text) = 0;
};

#endif // ARECORDBASE_H
