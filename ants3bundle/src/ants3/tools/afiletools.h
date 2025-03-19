#ifndef AFILETOOLS_H
#define AFILETOOLS_H

#include <QString>

#include <vector>
#include <complex>

namespace ftools
{
    bool loadTextFromFile(QString & text, const QString & fileName);
    bool saveTextToFile(const QString & text, const QString & fileName);

    QString mergeTextFiles(const std::vector<QString> & filesToMerge, QString fileName); //returns error string, otherwise ""

    QString loadPairs(const QString & fileName, std::vector<std::pair<double,double>> & data, bool enforceIncreasing = false);
    QString loadPairs(const QString & fileName, std::vector<std::pair<int,double>> & data, bool enforceIncreasing = false);
    QString loadDoubleComplexPairs(const QString & fileName, std::vector<std::pair<double,std::complex<double>>> & data, bool enforceIncreasing = false);

    // assume file's row is y and column is x  Top line will be the row with y-index of 0
    QString loadMatrix(const QString & fileName, std::vector<std::vector<double>> & data);

    QString loadDoubleVectorsFromFile(const QString & fileName, std::vector< std::vector<double>* > & vec);  //cleans previous data, returns error string
    QString saveDoubleVectorsToFile(const std::vector<std::vector<double> *> & vec, const QString & fileName);

/*
    //QString loadDoubleVectorsFromFile(const QString & FileName, QVector<double>* x, QVector<double>* y, QString * header = nullptr, int numLines = 10);  //cleans previous data
    //QString loadDoubleVectorsFromFile(const QString & FileName, QVector<QVector<double> *> & V);             //cleans previous data, returns error string
    //int LoadDoubleVectorsFromFile(QString FileName, QVector<double>* x);  //cleans previous data
    int LoadDoubleVectorsFromFile(QString FileName, QVector<double>* x, QVector<double>* y, QVector<double>* z);  //cleans previous data

    int SaveDoubleVectorsToFile(QString FileName, const QVector<double>* x, int count = -1);
    int SaveDoubleVectorsToFile(QString FileName, const QVector<double>* x, const QVector<double>* y, int count = -1);
    int SaveDoubleVectorsToFile(QString FileName, const QVector<double>* x, const QVector<double>* y, const QVector<double>* z, int count = -1);

    int SaveIntVectorsToFile(QString FileName, const QVector<int>* x, int count = -1);
    int SaveIntVectorsToFile(QString FileName, const QVector<int>* x, const QVector<int>* y, int count = -1);

    int LoadIntVectorsFromFile(QString FileName, QVector<int>* x);
    int LoadIntVectorsFromFile(QString FileName, QVector<int>* x, QVector<int>* y);
*/
}

#endif // AFILETOOLS_H
