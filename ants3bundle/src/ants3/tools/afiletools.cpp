#include "afiletools.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

bool ftools::loadTextFromFile(QString & text, const QString & fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) return false;

    QTextStream in(&file);
    text = in.readAll();
    file.close();
    return true;
}

bool ftools::saveTextToFile(const QString & text, const QString & fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) return false;

    QTextStream out(&file);
    out << text;
    file.close();
    return true;
}


QString ftools::mergeTextFiles(const std::vector<QString> & filesToMerge, QString fileName)
{
    QFile ofile(fileName);
    if (!ofile.open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open output file:\n" + fileName;
    QTextStream out(&ofile);

    QByteArray buffer;
    buffer.reserve(1000);
    for (const QString & fn : filesToMerge)
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

QString ftools::loadPairs(const QString & fileName, std::vector<std::pair<double, double>> & data, bool enforceIncreasing)
{
    if (fileName.isEmpty()) return "Error: empty name was given to file loader!";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return "Could not open: " + fileName;

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    data.clear();
    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();

        if (line.isEmpty()) continue;
        if (line.startsWith('#')) continue;

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.size() != 2) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain two numbers";

        bool ok1, ok2;
        double x, y;
        x = fields[0].toDouble(&ok1);
        y = fields[1].toDouble(&ok2);
        if (!ok1 || !ok2) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain two numbers";

        if (enforceIncreasing)
            if (data.size() > 1)
                if (data.back().first <= data[data.size()-2].first) return "Data should have increasing values in the first column";

        data.push_back({x, y});
    }
    file.close();

    if (data.empty()) return "Nothing was loaded";
    return "";
}

QString ftools::loadPairs(const QString & fileName, std::vector<std::pair<int, double>> & data, bool enforceIncreasing)
{
    if (fileName.isEmpty()) return "Error: empty name was given to file loader!";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return "Could not open: " + fileName;

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    data.clear();
    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();

        if (line.isEmpty()) continue;
        if (line.startsWith('#')) continue;

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.size() != 2) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain two numbers, an int and a double";

        bool ok1, ok2;
        double x, y;
        x = fields[0].toInt(&ok1);
        y = fields[1].toDouble(&ok2);
        if (!ok1 || !ok2) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain two numbers, an int and a double";

        if (enforceIncreasing)
            if (data.size() > 1)
                if (data.back().first <= data[data.size()-2].first) return "Data should have increasing values in the first column";

        data.push_back({x, y});
    }
    file.close();

    if (data.empty()) return "Nothing was loaded";
    return "";
}

QString ftools::loadDoubleComplexPairs(const QString & fileName, std::vector<std::pair<double, std::complex<double>>> & data, bool enforceIncreasing)
{
    if (fileName.isEmpty()) return "Error: empty name was given to file loader!";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return "Could not open: " + fileName;

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    data.clear();
    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();

        if (line.isEmpty()) continue;
        if (line.startsWith('#')) continue;

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.size() != 3) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain three numbers";

        bool ok1, ok2, ok3;
        double x, re, im;
        x  = fields[0].toDouble(&ok1);
        re = fields[1].toDouble(&ok2);
        im = fields[2].toDouble(&ok3);
        if (!ok1 || !ok2 || !ok3) return "Each line of the file (besides empty lines and comments starting with '#' symbol) should contain three numbers";

        if (enforceIncreasing)
            if (data.size() > 1)
                if (data.back().first <= data[data.size()-2].first) return "Data should have increasing values in the first column";

        data.push_back({x, {re,im}});
    }
    file.close();

    if (data.empty()) return "Nothing was loaded";
    return "";
}

QString ftools::loadMatrix(const QString & fileName, std::vector<std::vector<double>> & data)
{
    if (fileName.isEmpty()) return "Error: empty name was given to file loader!";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return "Could not open: " + fileName;

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    data.clear();
    int numInLine = -1;
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (line.isEmpty()) continue;
        if (line.simplified()[0] == '#') continue;

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (numInLine != -1 && fields.size() != numInLine)
        {
            data.clear();
            return "All rows of the matrix should have the same size";
        }

        numInLine = fields.size();
        std::vector<double> row(numInLine);

        bool ok;
        for (int iField = 0; iField < numInLine; iField++)
        {
            row[iField] = fields[iField].toDouble(&ok);
            if (!ok)
            {
                data.clear();
                return "Each line of the matrix (besides empty lines and comments starting with '#' symbol) should contain only numbers";
            }
        }
        data.push_back(row);
    }
    file.close();

    if (data.empty()) return "Nothing was loaded";
    return "";
}

QString ftools::loadDoubleVectorsFromFile(const QString & fileName, std::vector< std::vector<double>* > & vec)
{
    if (fileName.isEmpty()) return "File name not provided";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return QString("Could not open file %1").arg(fileName);

    const size_t Vsize = vec.size();
    if (Vsize == 0) return "Received no vectors to load";
    for (auto * v : vec) v->clear();

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    while (!in.atEnd())
    {
        const QStringList fields = in.readLine().split(rx, Qt::SkipEmptyParts);

        bool ok = true;
        std::vector<double> tmp(Vsize);
        if (fields.size() >= Vsize )
        {
            for (size_t i = 0; i < Vsize; i++)
            {
                double x = fields[i].toDouble(&ok);
                if (!ok) break;
                tmp[i] = x;
            }
        }
        if (ok)
            for (size_t i = 0; i < Vsize; i++)
                vec[i]->push_back(tmp[i]);
    }
    file.close();

    if (vec.front()->empty()) return QString("File %1 has invalid format").arg(fileName);

    return "";
}

QString ftools::saveDoubleVectorsToFile(const std::vector<std::vector<double> *> & vec, const QString & fileName)
{
    if (vec.empty()) return "No data to save!";
    const size_t size = vec.front()->size();
    for (size_t i = 1; i < vec.size(); i++)
        if (vec[i]->size() != size) return "Mismatch in vector size";

    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly);
    if (!outFile.isOpen()) return "Cannot open file " + fileName + " for output";

    QTextStream outStream(&outFile);

    for (size_t iLine = 0; iLine < size; iLine++)
    {
         for (size_t iVec = 0; iVec < vec.size(); iVec++)
         {
             if (iVec != 0) outStream << ' ';
             outStream << (*vec[iVec])[iLine];
         }
         outStream << '\n';
    }

    outFile.close();
    return "";
}

QString ftools::saveArrayOfDoublesToFile(const QString & fileName, const std::vector<double> & vec)
{
    if (vec.empty()) return "No data to save!";

    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly);
    if (!outFile.isOpen()) return "Cannot open file " + fileName + " for output";

    QTextStream outStream(&outFile);

    for (double val : vec)
        outStream << val << '\n';

    outFile.close();
    return "";
}

QString ftools::saveArrayOfDoublePairsToFile(const QString &fileName, const std::vector<std::pair<double, double>> & vec)
{
    if (vec.empty()) return "No data to save!";

    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly);
    if (!outFile.isOpen()) return "Cannot open file " + fileName + " for output";

    QTextStream outStream(&outFile);

    for (const std::pair<double, double> & pair : vec)
        outStream << pair.first << " " << pair.second << '\n';

    outFile.close();
    return "";
}
