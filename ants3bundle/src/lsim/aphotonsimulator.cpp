#include "aphotonsimulator.h"
#include "alogger.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

APhotonSimulator::APhotonSimulator(const QString & fileName, const QString & dir, int id) :
    ConfigFN(fileName), WorkingDir(dir), ID(id) {}

void APhotonSimulator::start()
{
    ALogger::getInstance().open(QString("PhotonSimLog-%0.log").arg(ID));
    LOG << "Working Dir: " << WorkingDir << "\n";
    LOG << "Config file: " << ConfigFN   << "\n";

    /*
    QFile file(ConfigFN);
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
        Ofile = new QFile(QString("psim_log_id%0.txt").arg(id));
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
    QObject::connect(&Timer, &QTimer::timeout, this, &A3PSim::onProgressTimer);
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
    */

    exit(0);
}

void APhotonSimulator::onProgressTimer()
{
    std::cout << "$$>" << EventsProcessed << "<$$\n";
    std::cout.flush();
    LOG << EventsProcessed << '\n';
}
