#include "a3dispatcher.h"

#include <QCoreApplication>
//#include <QString>
#include <QDebug>
//#include <QTimer>

#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        qCritical() << "Executable should be called with the argument = websocket server port";
        exit(1);
    }

    QCoreApplication a(argc, argv);

    QString portStr = argv[1];
    quint16 port = portStr.toUShort();

    A3Dispatcher Disp(port);
    Disp.start();
    //A3Dispatcher * Disp = new A3Dispatcher(&a);
    //QTimer::singleShot(0, Disp, &A3Dispatcher::start);
    return a.exec();
}
