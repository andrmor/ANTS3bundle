#include "aphotonsimulator.h"
#include "ageometryhub.h"

#include "TApplication.h"
#include "TH1.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        qDebug() << "Need 3 arguments: ConfigFileName,  WorkingDir, Id(int)";
        exit(1);
    }

    int rootargc = 1;
    char *rootargv[] = {(char*)"qqq"};
    TApplication RootApp("My ROOT", &rootargc, rootargv);
    TH1::AddDirectory(false);

    QCoreApplication a(argc, argv);
    APhotonSimulator Sim(argv[1], argv[2], atoi(argv[3]));
    QTimer::singleShot(0, &Sim, &APhotonSimulator::start);
    a.exec();

    AGeometryHub::getInstance().aboutToQuit();

    return 0;
}
