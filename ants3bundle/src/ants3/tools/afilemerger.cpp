#include "afilemerger.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>

#include <fstream>
#include <ios>

void AFileMerger::clear()
{
    FilesToMerge.clear();
}

void AFileMerger::add(const QString &fileName)
{
    FilesToMerge.push_back(fileName);
}

void AFileMerger::mergeToFile(const QString & fileName, bool binary) const
{
    if (binary)
    {
        std::ofstream outFile(fileName.toLatin1().data(), std::ios::ate | std::ios::binary);
        if (!outFile.is_open())
        {
            AErrorHub::addQError("Cannot open output file:\n" + fileName);
            return;
        }
        for (const QString & fn : FilesToMerge)
        {
            std::ifstream inFile(fn.toLatin1().data(), std::ios::binary);
            if (!inFile.is_open())
            {
                AErrorHub::addQError("Cannot open input file:\n" + fn);
                return;
            }

            std::copy( (std::istreambuf_iterator<char>(inFile)),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(outFile) );

            inFile.close();
        }
        outFile.close();
    }
    else
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
}
