#include "asensorsignalarray.h"

#include "aerrorhub.h"

#include <QTextStream>
#include <QStringList>

void ASensorSignalArray::writeAscii(QTextStream & stream) const
{
    for (float v : Signals)
        stream << v << ' ';
    stream << '\n';
}

bool ASensorSignalArray::readAscii(QString & line)
{
    const QStringList fields = line.split(' ', Qt::SkipEmptyParts);
    if (fields.size() != (int)Signals.size())
    {
        AErrorHub::addError("Format error or wrong number of sensors in the signal record");
        return false;
    }

    for (int i = 0; i < fields.size(); i++)
        Signals[i] = fields[i].toFloat();
    return true;
}

void ASensorSignalArray::print(QString & text)
{
    for (float v : Signals)
        text += QString::number(v) + " ";
    text += '\n';
}
