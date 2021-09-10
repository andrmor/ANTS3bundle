#include "ademo.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        qDebug() << "The 1st argument should be the dir for file exchange and the 2nd file name (no dir, should be in the exchange dir)";
        exit(1);
    }

    QCoreApplication a(argc, argv);
    ADemo Demo(argv[1], argv[2], true);
    QTimer::singleShot(0, &Demo, &ADemo::start);
    a.exec();

    return 0;
}
