#include "ademo.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        qDebug() << "The first argument should be the config file name and the second argument is the dir name with the data files";
        exit(1);
    }

    QCoreApplication a(argc, argv);
    ADemo Demo(argv[1], argv[2], true);
    QTimer::singleShot(0, &Demo, &ADemo::start);
    a.exec();

    return 0;
}
