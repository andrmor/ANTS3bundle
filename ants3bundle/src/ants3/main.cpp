#include "a3scriptmanager.h"
#include "adispatcherinterface.h"
#include "a3global.h"
#include "ageometryhub.h"
#include "amaterialhub.h"

#include "ademo_si.h"
#include "amath_si.h"
#include "afarm_si.h"
#include "aphotonsim_si.h"

#ifdef GUI
    #include <QApplication>
    #include "mainwindow.h"
    #include "aproxystyle.h"
#else
    #include <QCoreApplication>
#endif

#include <QDebug>
#include <QLocale>
#include <QTimer>
#include <QObject>

#include <memory>

#include "TApplication.h"
#include "TH1.h"


int main(int argc, char *argv[])
{
    int rootargc = 1;
    char *rootargv[] = {(char*)"qqq"};
    TApplication RootApp("My ROOT", &rootargc, rootargv);
    TH1::AddDirectory(false);  //a histograms objects will not be automatically created in root directory (TDirectory); special case is in TreeView and ResolutionVsArea

    std::unique_ptr<QCoreApplication> app;
#ifdef GUI
    if (argc == 1)
    {
        QApplication * qa = new QApplication(argc, argv);
        qa->setStyle(new AProxyStyle);
        app.reset(qa);
    }
    else app.reset(new QCoreApplication(argc, argv));
#else
    app.reset(new QCoreApplication(argc, argv));
#endif

    QLocale::setDefault(QLocale("en_US"));

    //QString FilterRules = "\nqt.network.ssl.warning=false"; //to suppress warnings about ssl
    //QLoggingCategory::setFilterRules(FilterRules);

    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.configureDirectories();

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QObject::connect(&(*app), &QCoreApplication::aboutToQuit, &Dispatcher, &ADispatcherInterface::aboutToQuit);

    A3ScriptManager * SM = new A3ScriptManager(&(*app));
    SM->registerInterface(new ADemo_SI(),      "demo");
    SM->registerInterface(new AMath_SI(),      "math");
    SM->registerInterface(new AFarm_SI(),      "farm");
    SM->registerInterface(new APhotonSim_SI(), "lsim");

    AMaterialHub::getInstance().addNewMaterial("Dummy", true);

#ifdef GUI
    if (argc == 1)
    {
        MainWindow * w = new MainWindow(*SM);
        w->show();
        app->exec();
        delete w;
    }
    else
#else // GUI
    if (argc > 1)  // later will be replaced with a proper line parser like in ants2
#endif // GUI
    {
        QTimer::singleShot(0, SM, [SM, argv](){SM->evaluate(argv[1]);});
        QObject::connect(SM, &A3ScriptManager::finished, &(*app), &QCoreApplication::quit, Qt::QueuedConnection);
        app->exec();
    }

    AGeometryHub::getInstance().aboutToQuit();

    return 0;
}
