#include "adeporecord.h"
#include "aerrorhub.h"

#include <QTextStream>
#include <QStringList>

void ADepoRecord::writeAscii(QTextStream & stream) const
{
    stream << Particle << ' '
           << MatIndex << ' '
           << Energy   << ' '
           << Pos[0]   << ' '
           << Pos[1]   << ' '
           << Pos[2]   << ' '
           << Time     << ' '
           << VolIndex << '\n';
}

bool ADepoRecord::readAscii(QString & line)
{
    const QStringList fields = line.split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 7)
    {
        AErrorHub::addError("Format error in ascii depo file (deposition record)");
        return false;
    }
    //particle mId dE x y z t       nodeIndex
    // 0        1   2 3 4 5 6       7
    // not interested in node index
    Particle =  fields[0];
    MatIndex =  fields[1].toInt();
    Energy   =  fields[2].toDouble();
    Pos      = {fields[3].toDouble(),
                fields[4].toDouble(),
                fields[5].toDouble()};
    Time     =  fields[6].toDouble();
    VolIndex =  fields[7].toInt();
    return true;
}

void ADepoRecord::writeBinary(std::ofstream & stream) const
{
    stream << char(0xFF);

    stream << Particle.toLatin1().data() << char(0x00);

    stream.write((char*)&MatIndex, sizeof(int));
    stream.write((char*)&Energy,   sizeof(double));
    stream.write((char*)&Pos[0],   sizeof(double));
    stream.write((char*)&Pos[1],   sizeof(double));
    stream.write((char*)&Pos[2],   sizeof(double));
    stream.write((char*)&Time,     sizeof(double));
    stream.write((char*)&VolIndex, sizeof(int));
}

bool ADepoRecord::readBinary(std::ifstream & stream)
{
    char ch;
    std::string pn;

    while (stream >> ch)
    {
        if (ch == (char)0x00) break;
        pn += ch;
    }

    Particle = pn.data();
    stream.read((char*)&MatIndex, sizeof(int));
    stream.read((char*)&Energy,   sizeof(double));
    stream.read((char*)&Pos[0],   sizeof(double));
    stream.read((char*)&Pos[1],   sizeof(double));
    stream.read((char*)&Pos[2],   sizeof(double));
    stream.read((char*)&Time,     sizeof(double));
    stream.read((char*)&VolIndex, sizeof(int));
    if (stream.fail())
    {
        AErrorHub::addError("ADepoRecord::readBinary: Unexpected format of a line in the binary file with the deposition data");
        return false;
    }

    return true;
}

void ADepoRecord::print(QString & text)
{
    text += Particle + "  " +
            QString("iMat:%0  ").arg(MatIndex) +
            QString("Energy:%0keV  ").arg(Energy) +
            QString("Pos:(%0,%1,%2)mm  ").arg(Pos[0]).arg(Pos[1]).arg(Pos[2]) +
            QString("Time:%0ns  ").arg(Time) +
            QString("VolIndex:%0\n").arg(VolIndex);
}
