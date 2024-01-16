#include "afiletools.h"
//#include "amessage.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

#ifdef GUI
#include <QMessageBox>
#endif

bool ftools::loadTextFromFile(QString & Text, const QString & FileName)
{
    QFile file(FileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) return false;

    QTextStream in(&file);
    Text = in.readAll();
    file.close();
    return true;
}

bool ftools::saveTextToFile(const QString & Text, const QString & FileName)
{
    QFile file(FileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) return false;

    QTextStream out(&file);
    out << Text;
    file.close();
    return true;
}


QString ftools::mergeTextFiles(const std::vector<QString> & FilesToMerge, QString FileName)
{
    QFile ofile(FileName);
    if (!ofile.open(QIODevice::WriteOnly | QFile::Text)) return "Cannot open output file:\n" + FileName;
    QTextStream out(&ofile);

    QByteArray buffer;
    buffer.reserve(1000);
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

QString ftools::loadDoubleVectorsFromFile(const QString & FileName, QVector<double> * x, QVector<double> * y, QString * header, int numLines)
{
    bool bGetHeader = (header && !header->isEmpty());
    QString HeaderId;
    if (bGetHeader)
    {
        HeaderId = *header;
        header->clear();
    }

    if (FileName.isEmpty()) return "Error: empty name was given to file loader!";

    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return "Could not open: " + FileName;

    QTextStream in(&file);
    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    x->resize(0);
    y->resize(0);
    while(!in.atEnd())
    {
        QString line = in.readLine();

        if (bGetHeader && line.startsWith(HeaderId) && numLines > 0)
        {
            if ( !header->isEmpty() ) *header += "\n";
            *header += line.remove(0, HeaderId.length());
            numLines--;
            continue;
        }

        QStringList fields = line.split(rx, Qt::SkipEmptyParts);

        bool ok1=false, ok2;
        double xx, yy;
        if (fields.size() > 1 )
        {
            xx = fields[0].toDouble(&ok1);  // potential problem with decimal separator!
            yy = fields[1].toDouble(&ok2);
        }
        if (ok1 && ok2)
        {
            x->append(xx);
            y->append(yy);
        }
    }
    file.close();

    if (x->isEmpty()) return "Error: Wrong format - read failed for file: " + FileName;

    return "";
}

QString ftools::loadDoubleVectorsFromFile(const QString & FileName, QVector<QVector<double> *> & V)
{
    if (FileName.isEmpty()) return("File name not provided");

    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) return QString("Could not open file %1").arg(FileName);

    const int Vsize = V.size();
    if (Vsize == 0) return "Received no vectors to load";
    for (QVector<double>* v : V) v->clear();

    QTextStream in(&file);
    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    while (!in.atEnd())
    {
        const QString line = in.readLine();
        QStringList fields = line.split(rx, Qt::SkipEmptyParts);

        bool fOK = true;
        QVector<double> tmp;
        if (fields.size() >= Vsize )
        {
            for (int i = 0; i < Vsize; i++)
            {
                double x = fields.at(i).toDouble(&fOK);
                if (!fOK) break;
                tmp << x;
            }
        }
        if (fOK && tmp.size() == Vsize)
            for (int i=0; i<Vsize; i++) V[i]->append( tmp.at(i) );
    }
    file.close();

    if (V.first()->isEmpty()) return QString("File %1 has invalid format").arg(FileName);

    return "";
}


QString ftools::saveDoubleVectorsToFile(const QVector<QVector<double> *> & V, const QString & FileName)
{
    if (V.isEmpty()) return "No data to save!";
    const int size = V.first()->size();
    for (int i = 1; i < V.size(); i++)
        if (V[i]->size() != size) return "Mismatch in vector size";

    QFile outFile(FileName);
    outFile.open(QIODevice::WriteOnly);
    if (!outFile.isOpen()) return "Cannot open file " + FileName + " for output";

    QTextStream outStream(&outFile);

    for (int iLine = 0; iLine < size; iLine++)
    {
         for (int iVec = 0; iVec < V.size(); iVec++)
         {
             if (iVec != 0) outStream << ' ';
             outStream << (*V[iVec])[iLine];
         }
         outStream << '\n';
    }

    outFile.close();
    return "";
}

// ==============================================================================================


/*
int LoadDoubleVectorsFromFile(QString FileName, QVector<double>* x)
{
  if (FileName.isEmpty())
      {
          message("Error: empty name was given to file loader!");
          return 1;
      }

  QFile file(FileName);
  if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
      message("Could not open: "+FileName);
      return 2;
    }

  QTextStream in(&file);
  QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
  x->resize(0);
  while(!in.atEnd())
       {
          QString line = in.readLine();
          QStringList fields = line.split(rx, QString::SkipEmptyParts);

          bool ok1= false;
          double xx;
          if (fields.size()>0) xx = fields[0].toDouble(&ok1);  //potential problem with decimal separator!

          if (ok1)
            {
              x->append(xx);
            }
        }
   file.close();

   if (x->isEmpty())
   {
       message("Error: Wrong format - nothing was red: "+FileName);
       return 3;
   }

   return 0;
}

int LoadDoubleVectorsFromFile(QString FileName, QVector<double>* x, QVector<double>* y, QVector<double>* z)
{
  if (FileName.isEmpty())
  {
      message("Error: empty name was given to file loader!");
      return 1;
  }

  QFile file(FileName);
  if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
      message("Could not open: "+FileName);
      return 2;
    }

  QTextStream in(&file);
  QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
  x->resize(0);
  y->resize(0);
  z->resize(0);
  while(!in.atEnd())
       {
          QString line = in.readLine();
          QStringList fields = line.split(rx, QString::SkipEmptyParts);

          bool ok1=false, ok2, ok3;
          double xx, yy, zz;
          if (fields.size() > 2 )
            {
               xx = fields[0].toDouble(&ok1);  // potential problem with decimal separator!
               yy = fields[1].toDouble(&ok2);
               zz = fields[2].toDouble(&ok3);
            }
          if (ok1 && ok2 && ok3)
            {
              x->append(xx);
              y->append(yy);
              z->append(zz);
            }
        }
   file.close();

   if (x->isEmpty())
   {
       message("Error: Wrong format - nothing was red: "+FileName);
       return 3;
   }

  return 0;
}

int SaveDoubleVectorsToFile(QString FileName, const QVector<double>* x, int count)
{
  if (count == -1) count = x->size();
  QFile outFile( FileName );
  outFile.open(QIODevice::WriteOnly);
  if(!outFile.isOpen())
    {
      qDebug() << "- Error, unable to open" << FileName << "for output";
#ifdef GUI
      QMessageBox mb;
      mb.setText("Unable to open file " +FileName+ " for writing!");
      mb.exec();
#endif
      return 1;
    }
  QTextStream outStream(&outFile);

  for (int i=0; i<count; i++)
    outStream << x->at(i) <<"\r\n";
  outFile.close();
  return 0;
}

int SaveDoubleVectorsToFile(QString FileName, const QVector<double> *x, const QVector<double> *y, int count)
{
  if (count == -1) count = x->size();
  QFile outFile( FileName );
  outFile.open(QIODevice::WriteOnly);
  if(!outFile.isOpen())
    {
      qDebug() << "- Error, unable to open" << FileName << "for output";
 #ifdef GUI
      QMessageBox mb;
      mb.setText("Unable to open file " +FileName+ " for writing!");
      mb.exec();
 #endif
      return 1;
    }
  QTextStream outStream(&outFile);

  for (int i=0; i<count; i++)
    outStream << x->at(i) << " " << y->at(i) <<"\r\n";
  outFile.close();
  return 0;
}

int SaveDoubleVectorsToFile(QString FileName, const QVector<double> *x, const QVector<double> *y, const QVector<double> *z, int count)
{
  if (count == -1) count = x->size();
  QFile outFile( FileName );
  outFile.open(QIODevice::WriteOnly);
  if(!outFile.isOpen())
    {
      qDebug() << "- Error, unable to open" << FileName << "for output";
 #ifdef GUI
      QMessageBox mb;
      mb.setText("Unable to open file " +FileName+ " for writing!");
      mb.exec();
 #endif
      return 1;
    }
  QTextStream outStream(&outFile);

  for (int i=0; i<count; i++)
    outStream << x->at(i) << " " << y->at(i) << " " << z->at(i) << "\r\n";
  outFile.close();
  return 0;
}

int LoadIntVectorsFromFile(QString FileName, QVector<int>* x)
{
  if (FileName.isEmpty())
      {
          message("Empty file: "+FileName);
          return 1;
      }

  QFile file(FileName);
  if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
      message("Could not open: "+FileName);
      return 2;
    }

  QTextStream in(&file);
  QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
  x->resize(0);
  while(!in.atEnd())
       {
          QString line = in.readLine();
          QStringList fields = line.split(rx);

          bool ok1;
          int xx = fields[0].toInt(&ok1);
          if (ok1)
            {
              x->append(xx);
            }
        }
   file.close();

   if (x->isEmpty())
   {
       message("Error: Wrong format - nothing was red: "+FileName);
       return 3;
   }

   return 0;
}

int LoadIntVectorsFromFile(QString FileName, QVector<int> *x, QVector<int> *y)
{
  if (FileName.isEmpty())
      {
          message("Empty file: "+FileName);
          return 1;
      }

  QFile file(FileName);
  if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
      message("Could not open: "+FileName);
      return 2;
    }

  QTextStream in(&file);
  QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
  x->resize(0);
  y->resize(0);
  while(!in.atEnd())
       {
          QString line = in.readLine();
          QStringList fields = line.split(rx);

          bool ok1, ok2;
          int xx = fields[0].toInt(&ok1);
          int yy = fields[1].toInt(&ok2);
          if (ok1 && ok2)
            {
              x->append(xx);
              y->append(yy);
            }
        }
   file.close();

   if (x->isEmpty())
   {
       message("Error: Wrong format - nothing was red: "+FileName);
       return 3;
   }

   return 0;
}

int SaveIntVectorsToFile(QString FileName, const QVector<int> *x, int count)
{
  if (count == -1) count = x->size();
  QFile outFile( FileName );
  outFile.open(QIODevice::WriteOnly);
  if(!outFile.isOpen())
    {
      qDebug() << "- Error, unable to open" << FileName << "for output";
 #ifdef GUI
      QMessageBox mb;
      mb.setText("Unable to open file " +FileName+ " for writing!");
      mb.exec();
 #endif
      return 1;
    }
  QTextStream outStream(&outFile);

  for (int i=0; i<count; i++)
    outStream << x->at(i) <<"\r\n";
  outFile.close();
  return 0;
}

int SaveIntVectorsToFile(QString FileName, const QVector<int> *x, const QVector<int> *y, int count)
{
  if (count == -1) count = x->size();
  QFile outFile( FileName );
  outFile.open(QIODevice::WriteOnly);
  if(!outFile.isOpen())
    {
      qDebug() << "- Error, unable to open" << FileName << "for output";
 #ifdef GUI
      QMessageBox mb;
      mb.setText("Unable to open file " +FileName+ " for writing!");
      mb.exec();
 #endif
      return 1;
    }
  QTextStream outStream(&outFile);

  for (int i=0; i<count; i++)
    outStream << x->at(i) << " " << y->at(i) <<"\r\n";
  outFile.close();
  return 0;
}

*/
