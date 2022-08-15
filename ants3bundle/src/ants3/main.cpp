#include "adispatcherinterface.h"
#include "a3global.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "ascripthub.h"
#include "ajscriptmanager.h"

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

#include "ascriptinterface.h"

int main(int argc, char *argv[])
{
    int rootargc = 1;
    char *rootargv[] = {(char*)"qqq"};
    TApplication RootApp("My ROOT", &rootargc, rootargv);
    TH1::AddDirectory(false);  //a histograms objects will not be automatically created in root directory (TDirectory); special case is in TreeView and ResolutionVsArea

    qRegisterMetaType<AScriptInterface*>();
    qRegisterMetaType<TObject*>();

    std::unique_ptr<QCoreApplication> app;
#ifdef GUI
    if ( argc == 1   ||   (argc == 2 && QString(argv[1]).startsWith("-qml")) )
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
    GlobSet.init();

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QObject::connect(&(*app), &QCoreApplication::aboutToQuit, &Dispatcher, &ADispatcherInterface::aboutToQuit);

    AScriptHub::getInstance();

    AMaterialHub::getInstance().addNewMaterial("Vacuum", true);

    // should be last line in initialization!
    AConfig::getInstance().updateJSONfromConfig();

#ifdef GUI
    if (argc == 1)
    {
        QCoreApplication::setOrganizationName("ants3");
        QCoreApplication::setApplicationName("winpos");

        MainWindow * w = new MainWindow();
        w->show();
        app->exec();
        delete w;
    }
    else
#else // GUI
    if (argc > 1)  // later will be replaced with a proper line parser like in ants2
#endif // GUI
    {
        AJScriptManager * SM = &AScriptHub::getInstance().getJScriptManager();
        QTimer::singleShot(0, SM, [SM, argv](){SM->evaluate(argv[1]);});
        QObject::connect(SM, &AJScriptManager::finished, &(*app), &QCoreApplication::quit, Qt::QueuedConnection);
        app->exec();
    }

    AGeometryHub::getInstance().aboutToQuit();

    GlobSet.saveConfig();
    return 0;
}
