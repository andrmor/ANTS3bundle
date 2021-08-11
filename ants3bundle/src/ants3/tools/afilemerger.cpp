#include "afilemerger.h"

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

QString AFileMerger::mergeToFile(const QString & fileName) const
{
    QFile ofile(fileName);
    if (!ofile.open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open output file:\n" + fileName;
    QTextStream out(&ofile);

    QByteArray buffer;
    buffer.reserve(BufferSize);
    for (const QString & fn : FilesToMerge)
    {
        QFile file(fn);
        if (!file.open(QIODevice::ReadOnly | QFile::Text)) return "Cannot open input file:\n" + fn;

        while (!file.atEnd())
        {
            buffer = file.readLine();
            //qDebug() << "=>"<< buffer << "<=";
            out << buffer;
        }
        file.close();
    }
    ofile.close();
    return "";
}
