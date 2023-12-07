#include "anoderecord.h"
#include "aerrorhub.h"

#include <QTextStream>
#include <QStringList>

ANodeRecord::ANodeRecord(double x, double y, double z, double time, int numPhot) :
    Time(time), NumPhot(numPhot)
{
    R[0] = x;
    R[1] = y;
    R[2] = z;
}

void ANodeRecord::writeAscii(QTextStream & stream) const
{
    //X Y Z Time Num
    //0 1 2  3    4
    stream << R[0]    << ' '
           << R[1]    << ' '
           << R[2]    << ' '
           << Time    << ' '
           << NumPhot << '\n';
}

bool ANodeRecord::readAscii(QString & line)
{
    const QStringList fields = line.split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 5)
    {
        AErrorHub::addError("Format error in ascii depo file (deposition record)");
        return false;
    }
    //X Y Z Time Num
    //0 1 2  3    4
    R[0]    =  fields[0].toDouble();
    R[1]    =  fields[1].toDouble();
    R[2]    =  fields[2].toDouble();
    Time    =  fields[3].toDouble();
    NumPhot =  fields[4].toInt();
    return true;
}

bool ANodeRecord::readBinary(std::ifstream & stream)
{
    stream.read((char*)&R[0],    sizeof(double));
    stream.read((char*)&R[1],    sizeof(double));
    stream.read((char*)&R[2],    sizeof(double));
    stream.read((char*)&Time,    sizeof(double));
    stream.read((char*)&NumPhot, sizeof(int));
    if (stream.fail())
    {
        AErrorHub::addError("Unexpected format of a line in the binary file with records of photon emission nodes");
        return false;
    }
    return true;
}

void ANodeRecord::print(QString & text)
{
    text += QString("Pos:(%0,%1,%2)mm  ").arg(R[0]).arg(R[1]).arg(R[2]) +
            QString("Time:%0ns  ").arg(Time) +
            QString("Num:%0\n").arg(NumPhot);
}
