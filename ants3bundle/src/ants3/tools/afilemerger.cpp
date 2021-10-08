#include "afilemerger.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>

void AFileMerger::clear()
{
    FilesToMerge.clear();
}

void AFileMerger::add(const QString &fileName)
{
    FilesToMerge.push_back(fileName);
}

void AFileMerger::mergeToFile(const QString & fileName) const
{
    QFile ofile(fileName);
    if (!ofile.open(QIODevice::WriteOnly | QFile::Text))
    {
        AErrorHub::addError( QString("Cannot open output file:\n" + fileName).toLatin1().data() );
        return;
    }
    QTextStream out(&ofile);

    QByteArray buffer;
    buffer.reserve(BufferSize);
    for (const QString & fn : FilesToMerge)
    {
        QFile file(fn);
        if (!file.open(QIODevice::ReadOnly | QFile::Text))
        {
            AErrorHub::addError( QString("Cannot open input file:\n" + fn).toLatin1().data() );
            return;
        }

        while (!file.atEnd())
        {
            buffer = file.readLine();
            //qDebug() << "=>"<< buffer << "<=";
            out << buffer;
        }
        file.close();
    }
    ofile.close();
}
