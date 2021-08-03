#include "aphotonsimulator.h"
#include "alogger.h"
#include "ajsontools.h"

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

    QJsonObject json;
    jstools::loadJsonFromFile(json, ConfigFN);



//    QFile fileIn (FileDir + '/' + inFN);  fileIn .open(QIODevice::ReadOnly);
//    QFile fileOut(FileDir + '/' + outFN); fileOut.open(QIODevice::WriteOnly);
//    QTextStream in(&fileIn);
//    QTextStream out(&fileOut);

//    QTimer Timer;
//    QObject::connect(&Timer, &QTimer::timeout, this, &A3PSim::onProgressTimer);
//    Timer.start(300);

//    EventsProcessed = 0;
//    while (!in.atEnd())
//    {
//          QString line = in.readLine();
//          line.replace(from, to);
//          out << line << '\n';
//          EventsProcessed++;
//          QThread::msleep(1000); //just to simulate long execution!
//          qApp->processEvents();
//    }
//    fileIn.close();
//    fileOut.close();

    exit(0);
}

void APhotonSimulator::onProgressTimer()
{
    std::cout << "$$>" << EventsProcessed << "<$$\n";
    std::cout.flush();
    LOG << EventsProcessed << '\n';
}
