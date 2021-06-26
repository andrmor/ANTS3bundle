#include "a3particlesimmanager.h"
#include "a3scriptres.h"
#include "a3scriptmanager.h"
#include "a3dispinterface.h"
#include "a3global.h"

// GUI
#ifdef GUI
    #include <QApplication>
    #include "mainwindow.h"
    // #include "aproxystyle.h"
#else
    #include <QCoreApplication>
#endif

#include <QDebug>
#include <QLocale>
#include <QTimer>

#include <memory>

//Root
#include "TApplication.h"
#include "TH1.h"

#include <QNetworkAccessManager>

int main(int argc, char *argv[])
{
    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.configureDirectories();

    //starting cern ROOT application
    int rootargc = 1;
    char *rootargv[] = {(char*)"qqq"};
    TApplication RootApp("My ROOT", &rootargc, rootargv);

    std::unique_ptr<QCoreApplication> app;

#ifdef GUI
    if (argc == 1)
    {
        QApplication * qa = new QApplication(argc, argv);
        //qa->setStyle(new AProxyStyle);
        app.reset(qa);
    }
    else app.reset(new QCoreApplication(argc, argv));
#else
    app.reset(new QCoreApplication(argc, argv));
#endif

    QLocale::setDefault(QLocale("en_US"));

    //QString FilterRules = "\nqt.network.ssl.warning=false"; //to suppress warnings about ssl
    //QLoggingCategory::setFilterRules(FilterRules);

    //AGlobalSettings & GlobSet = AGlobalSettings::getInstance();
    //qDebug() << "Global settings object created";
    TH1::AddDirectory(false);  //a histograms objects will not be automatically created in root directory (TDirectory); special case is in TreeView and ResolutionVsArea

    A3DispInterface * Dispatch = new A3DispInterface(&(*app));
    Dispatch->start();

    A3ParticleSimManager * PSM = new A3ParticleSimManager(*Dispatch, &(*app));

    A3ScriptRes ScrRes;
    ScrRes.ParticleSim = PSM;

    A3ScriptManager * SM = new A3ScriptManager(ScrRes, &(*app));

#ifdef GUI
    if(argc == 1)
    {
        MainWindow * w = new MainWindow(*SM, ScrRes);
        QObject::connect(Dispatch, &A3DispInterface::updateProgress, w, &MainWindow::onProgressReceived);
        w->show();
        int res = app->exec();
        delete w;
        return res;
    }
    else
#else // GUI
    if (argc > 1)
#endif // GUI
    {
        QTimer::singleShot(0, [SM, argv](){SM->evaluate(argv[1]);});
        QObject::connect(SM, &A3ScriptManager::finished, &(*app), &QCoreApplication::quit, Qt::QueuedConnection);
        app->exec();
    }

    return 0;
}
