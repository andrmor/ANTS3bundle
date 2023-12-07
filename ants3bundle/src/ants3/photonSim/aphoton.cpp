#include "aphoton.h"
#include "arandomhub.h"
#include "aerrorhub.h"

#include <QStringList>
#include <QTextStream>

#include "TMath.h"   // !!!*** remove?

APhoton::APhoton() {}

APhoton::APhoton(double * pos, double * dir, int waveIndex, double time) :
    time(time), waveIndex(waveIndex), SecondaryScint(false)
{
    for (int i=0; i<3; i++)
    {
        r[i] = pos[i];
        v[i] = dir[i];
    }
}

void APhoton::copyFrom(const APhoton & CopyFrom)
{
    for (int i = 0; i < 3; i++)
    {
        r[i] = CopyFrom.r[i];
        v[i] = CopyFrom.v[i];
    }

    time           = CopyFrom.time;
    waveIndex      = CopyFrom.waveIndex;
    SecondaryScint = CopyFrom.SecondaryScint;
}

void APhoton::ensureUnitaryLength()
{
    double mod = 0;
    for (int i=0; i<3; i++)
        mod += ( v[i] * v[i] );

    if (mod == 1.0) return;
    mod = TMath::Sqrt(mod);

    if (mod != 0)
        for (int i=0; i<3; i++) v[i] /= mod;
    else
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 1.0;
    }
}

void APhoton::generateRandomDir()
{
    ARandomHub & RandomHub = ARandomHub::getInstance();

    //Sphere function of Root:
    double a = 0, b = 0, r2 = 1.0;
    while (r2 > 0.25)
    {
        a  = RandomHub.uniform() - 0.5;
        b  = RandomHub.uniform() - 0.5;
        r2 = a*a + b*b;
    }
    double scale = 8.0 * TMath::Sqrt(0.25 - r2);
    v[0] = a * scale;
    v[1] = b * scale;
    v[2] = ( -1.0 + 8.0 * r2 );
}

void APhoton::writeAscii(QTextStream & stream) const
{
    //X Y Z dX dY dZ Time iWave
    //0 1 2 3  4  5    6    7
    stream << r[0]      << ' '
           << r[1]      << ' '
           << r[2]      << ' '
           << v[0]      << ' '
           << v[1]      << ' '
           << v[2]      << ' '
           << time      << ' '
           << waveIndex
           << '\n';
}

bool APhoton::readAscii(QString & line)
{
    const QStringList fields = line.split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 8)
    {
        AErrorHub::addError("Format error in ascii photon record file (photon record)");
        return false;
    }
    //X Y Z dX dY dZ Time iWave
    //0 1 2 3  4  5    6    7
    r[0]      =  fields[0].toDouble();
    r[1]      =  fields[1].toDouble();
    r[2]      =  fields[2].toDouble();
    v[0]      =  fields[3].toDouble();
    v[1]      =  fields[4].toDouble();
    v[2]      =  fields[5].toDouble();
    time      =  fields[6].toDouble();
    waveIndex =  fields[7].toInt();
    return true;
}

bool APhoton::readBinary(std::ifstream & stream)
{
    stream.read((char*)&r[0],      sizeof(double));
    stream.read((char*)&r[1],      sizeof(double));
    stream.read((char*)&r[2],      sizeof(double));
    stream.read((char*)&v[0],      sizeof(double));
    stream.read((char*)&v[1],      sizeof(double));
    stream.read((char*)&v[2],      sizeof(double));
    stream.read((char*)&time,      sizeof(double));
    stream.read((char*)&waveIndex, sizeof(int));
    if (stream.fail())
    {
        AErrorHub::addError("Unexpected format of a line in the binary file with the node records");
        return false;
    }
    return true;
}

void APhoton::print(QString & text)
{
    text += QString("Pos:(%0,%1,%2)mm  ").arg(r[0]).arg(r[1]).arg(r[2]) +
            QString("Dir:(%0,%1,%2)  ").arg(v[0]).arg(v[1]).arg(v[2]) +
            QString("Time:%0ns  ").arg(time) +
            QString("iWave:%0\n").arg(waveIndex);
}
