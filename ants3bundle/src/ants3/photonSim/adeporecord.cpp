#include "adeporecord.h"
#include "aerrorhub.h"

#include <QTextStream>

void ADepoRecord::writeAscii(QTextStream & stream) const
{
    stream << Particle << ' '
           << MatIndex << ' '
           << Energy   << ' '
           << Pos[0]   << ' '
           << Pos[1]   << ' '
           << Pos[2]   << ' '
           << Time     << '\n';
}

bool ADepoRecord::readAscii(QString & line)
{
    const QStringList fields = line.split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 7)
    {
        AErrorHub::addError("Format error in ascii depo file (deposition record)");
        return false;
    }
    //particle mId dE x y z t
    // 0        1   2 3 4 5 6
    Particle =  fields[0];
    MatIndex =  fields[1].toInt();
    Energy   =  fields[2].toDouble();
    Pos      = {fields[3].toDouble(),
                fields[4].toDouble(),
                fields[5].toDouble()};
    Time     =  fields[6].toDouble();
    return true;
}
