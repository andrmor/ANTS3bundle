#include "ademo.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

ADemo::ADemo(const QString & dir, const QString & fileName, bool debug) :
    FileDir(dir), FileName(fileName), Debug(debug) {}

ADemo::~ADemo()
{
    delete Log;

    if (Ofile) Ofile->close();
    delete Ofile;
}

void ADemo::start()
{
    QFile file(FileDir + "/" + FileName);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject json = loadDoc.object();

    int     id    = json["ID"].toInt();
    QString inFN  = json["Input"].toString();
    QString outFN = json["Output"].toString();
    QString from  = json["From"].toString();
    QString to    = json["To"].toString();

    if (Debug)
    {
        Ofile = new QFile(QString("demo_log_id%0.txt").arg(id));
        Ofile->open(QIODevice::WriteOnly | QFile::Text);
        Log = new QTextStream(Ofile);
        *Log << FileName << '\n';
        Log->flush();
    }

    QFile fileIn (FileDir + '/' + inFN);  fileIn .open(QIODevice::ReadOnly);
    QFile fileOut(FileDir + '/' + outFN); fileOut.open(QIODevice::WriteOnly);

    QTextStream in(&fileIn);
    QTextStream out(&fileOut);

    QTimer Timer;
    QObject::connect(&Timer, &QTimer::timeout, this, &ADemo::onProgressTimer);
    Timer.start(300);

    EventsProcessed = 0;
    while (!in.atEnd())
    {
          QString line = in.readLine();
          line.replace(from, to);
          out << line << '\n';
          EventsProcessed++;
          QThread::msleep(1000); //just to simulate long execution!
          qApp->processEvents();
    }
    fileIn.close();
    fileOut.close();

    exit(0);
}

void ADemo::onProgressTimer()
{
    std::cout << "$$>" << EventsProcessed << "<$$\n";
    std::cout.flush();
    if (Log)
    {
        *Log << EventsProcessed << '\n';
        Log->flush();
    }
    //qApp->processEvents();
}
