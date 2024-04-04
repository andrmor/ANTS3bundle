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

bool ASensorSignalArray::readBinary(std::ifstream &stream)
{
    const size_t num = Signals.size();
    for (size_t i = 0; i < num; i++)
        stream.read((char*)&Signals[i], sizeof(float));
    if (stream.fail())
    {
        AErrorHub::addError("Unexpected format of a line in the binary file with sensor signals");
        return false;
    }
    return true;
}

void ASensorSignalArray::print(QString & text)
{
    for (float v : Signals)
        text += QString::number(v) + " ";
    text += '\n';
}
