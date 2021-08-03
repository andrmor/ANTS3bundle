#include "alogger.h"

#include <QString>
#include <QFile>
#include <QTextStream>

ALogger & ALogger::getInstance()
{
    static ALogger instance;
    return instance;
}

ALogger::~ALogger()
{
    if (Log) *Log << "END";
    delete Log;

    if (Ofile) Ofile->close();
    delete Ofile;
}

void ALogger::open(const QString & fileName)
{
    Ofile = new QFile(fileName);
    Ofile->open(QIODevice::WriteOnly | QFile::Text);
    Log = new QTextStream(Ofile);
    *Log << "START\n";
    Log->flush();
}

QTextStream & ALogger::log()
{
    ALogger & logger = getInstance();
    return *logger.Log;
}
