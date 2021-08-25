#include "a3dispatcher.h"

#include <QCoreApplication>
//#include <QString>
#include <QDebug>
//#include <QTimer>

#include <stdlib.h>

int main(int argc, char *argv[])
{
#ifndef WEBSOCKETS
    qDebug() << "Websockets are not enabled in the config. Exiting";
    exit(1);
#endif

    if (argc < 4)
    {
        qCritical() << "Executable should be called with three arguments: IP port maxProcesses";
        exit(1);
    }

    QCoreApplication a(argc, argv);

    QString ip = argv[1];

    QString portStr = argv[2];
    quint16 port = portStr.toUShort();

    QString maxPrStr = argv[3];
    quint16 maxPr = maxPrStr.toInt();

    A3Dispatcher Disp(ip, port, maxPr);
    Disp.start();
    //A3Dispatcher * Disp = new A3Dispatcher(&a);
    //QTimer::singleShot(0, Disp, &A3Dispatcher::start);
    return a.exec();
}
