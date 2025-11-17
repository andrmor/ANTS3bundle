#include "adispatcherinterface.h"
#include "a3global.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "ascriptinterface.h"
#include "ascriptobjstore.h"
#include "amainwindow.h"
#include "afiletools.h"
#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "awebsocketserver.h"
#include <csignal>
#ifdef ANTS3_PYTHON
#include "apythonscriptmanager.h"
#endif

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTimer>
#include <QObject>
#include <QCommandLineParser>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QHostAddress>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>

#include <memory>
#include <signal.h>

#include "TApplication.h"
#include "TH1.h"

Q_DECLARE_METATYPE(TObject*)

void signal_handler(int)
{
    QCoreApplication::exit();
}

int main(int argc, char *argv[])
{
    qDebug() << "Starting ANTS3";
    // need to start Root application before QtApplication
    int rootargc = 1;
    char* rootargv[] = {(char*)"dummy"};
    TApplication RootApp("MyROOT", &rootargc, rootargv);
    TH1::AddDirectory(false);  //a histograms objects will not be automatically created in root directory (TDirectory); special case is in TreeView

    std::unique_ptr<QCoreApplication> app;
    bool guiMode = ( argc == 1 || (argc == 2 && QString(argv[1]).startsWith("-qml")) ); // -qml is obsolete with new Qt versions?
    if (guiMode)
    {
        QApplication * qa = new QApplication(argc, argv);
        app.reset(qa);
    }
    else app.reset(new QCoreApplication(argc, argv));

    QCommandLineParser parser;
    parser.setApplicationDescription("\nANTS3\nTo run in the GUI mode, do not add any arguments");
    parser.addHelpOption();
    parser.addPositionalArgument("scriptFile", "File with the script to run");
    parser.addPositionalArgument("ip",         "Web socket server IP");
    parser.addPositionalArgument("port",       "Web socket server port");

    QCommandLineOption serverOption({"s", "server"}, "Run ANTS3 in WebSocket server mode");
    parser.addOption(serverOption);

    QCommandLineOption javascriptOption(QStringList() << "j" << "javascript", "Run javascript from <scriptFile>", "scriptFile");
    parser.addOption(javascriptOption);

    QCommandLineOption pythonOption({"y", "python"}, "Run python script from <scriptFile>", "scriptFile");
    parser.addOption(pythonOption);

    QCommandLineOption ipOption(QStringList() << "i" << "ip",       "Set serverj  IP.",               "ip");
    parser.addOption(ipOption);

    QCommandLineOption portOption(QStringList() << "p" << "port",   "Set server port.",             "port");
    parser.addOption(portOption);

    parser.process(*app);

    qRegisterMetaType<AScriptInterface*>();
    qRegisterMetaType<TObject*>();

    QLocale::setDefault(QLocale("en_US"));

    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.init();

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QObject::connect(&(*app), &QCoreApplication::aboutToQuit, &Dispatcher, &ADispatcherInterface::aboutToQuit);

    AScriptHub::getInstance();
    AMaterialHub::getInstance().addNewMaterial("Vacuum");

    AGeometryHub::getInstance().populateGeoManager();

    AConfig::getInstance().updateJSONfromConfig(); // should be the last line in initialization !!!

    // --- init phase completed ---

    if (guiMode)
    {
        QCoreApplication::setOrganizationName("ants3");
        QCoreApplication::setApplicationName("winpos");

        AMainWindow * w = new AMainWindow();
        w->show();
        app->exec();
        delete w;

        GlobSet.saveConfig();
    }
    else
    {
        AScriptHub::getInstance().finalizeInit();

        if ( parser.isSet(javascriptOption) )
        {
            QString fileName = parser.value(javascriptOption);
            QString err = AScriptHub::getInstance().evaluateScriptAndWaitToFinish(fileName, EScriptLanguage::JavaScript);
            if (!err.isEmpty())
            {
                qDebug() << "";
                qDebug() << "Script reports an error:";
                qDebug() << err;
                qDebug() << "";
            }
            else qDebug() << QString("Finished at %1").arg(QTime::currentTime().toString());
        }
        else if ( parser.isSet(pythonOption) )
        {
            QString fileName = parser.value(pythonOption);
            QString err = AScriptHub::getInstance().evaluateScriptAndWaitToFinish(fileName, EScriptLanguage::Python);
            if (!err.isEmpty())
            {
                qDebug() << "";
                qDebug() << "Script reports an error:";
                qDebug() << err;
                qDebug() << "";
            }
            else qDebug() << QString("Finished at %1").arg(QTime::currentTime().toString());
        }
        else if ( parser.isSet(serverOption) )
        {
            QHostAddress ip = QHostAddress::Null;
            if (parser.isSet(ipOption))
            {
                QString ips = parser.value(ipOption);
                ip = QHostAddress(ips);
            }
            if (ip.isNull())
            {
                qCritical("IP is not provided, exiting!");
                exit(103);
            }
            quint16 Port = parser.value(portOption).toUShort();
            qDebug() << "Starting server. IP =" << ip.toString() << "port =" << Port;
            AWebSocketServer & WebServer = AWebSocketServer::getInstance();
            WebServer.stopListen();
            WebServer.startListen(QHostAddress(ip.toString()), Port);
            qDebug() << "To connect, use "<< WebServer.getUrl();
            std::signal(SIGINT, signal_handler); // to catch Ctrl-C
            app->exec();
            qDebug() << QString("Finished at %1").arg(QTime::currentTime().toString());
        }
        else
        {
            qDebug() << "Unknown ANTS3 mode! Try ants3 -h";
            // need to start app to properly finish destruction of worker threads
            QTimer::singleShot(50, [](){QCoreApplication::exit();});
            app->exec();
        }
    }

    //qDebug() << "About to quit...";
    AScriptObjStore::getInstance().Trees.clear(); // crash if clear is in the destr
    AGeometryHub::getInstance().aboutToQuit();
    AScriptHub::getInstance().aboutToQuit();
    app->processEvents();
    QThread::usleep(100);

    return 0;
}
