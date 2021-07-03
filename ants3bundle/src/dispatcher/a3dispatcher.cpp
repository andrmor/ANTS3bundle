#include "a3dispatcher.h"
#include "a3processhandler.h"  // in ANTS3 dir!
#include "a3workdistrconfig.h" // in ANTS3 dir!
#include "ajsontools.h"        // in ANTS3 dir!
#include "awebsocketsessionserver.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QSocketNotifier>
#include <QTextStream>
#include <QByteArray>
#include <QFile>
#include <QThread>
#include <QVector>
#include <QCoreApplication>
#include <QHostAddress>
#include <QDir>
#include <QTimer>

#include <iostream>
#include <ios>

A3Dispatcher::A3Dispatcher(quint16 port, QObject *parent) :
    QObject(parent), PortPersistentWS(port)
{
    QString dir = QDir::currentPath();
    StandaloneDir = dir + '/' + "DispatcherTmp" + QString::number(PortPersistentWS);
    if (!QDir(StandaloneDir).exists()) QDir().mkdir(StandaloneDir);

    OutFile = new QFile(StandaloneDir + "/dispLog.txt");
    OutFile->open(QIODevice::WriteOnly | QFile::Text);
    LogStream = new QTextStream(OutFile);

    if (PortPersistentWS == 0)
    {
        Notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
        connect(Notifier, SIGNAL(activated(int)), this, SLOT(onLocalCommandReceived()));
        log("---->Dispatcher is listening on std::cin");
    }
    else
    {
        WebSocketServer = new AWebSocketSessionServer(StandaloneDir, this);
        connect(WebSocketServer, &AWebSocketSessionServer::remoteCommandReceived, this, &A3Dispatcher::onRemoteCommandReceived);
        log("---->Standalone dispatcher started on WS port " + QString::number(port));
    }

    ProgressReportTimer = new QTimer();
    connect(ProgressReportTimer, &QTimer::timeout, this, &A3Dispatcher::onReportProgressTimer);
    ProgressReportTimer->setInterval(300);
}

A3Dispatcher::~A3Dispatcher()
{
    clearHandlers();

    log("====>Finished");
    OutFile->close();
    delete LogStream;
    delete OutFile;
}

void A3Dispatcher::start()
{
    if (PortPersistentWS != 0)
        WebSocketServer->StartListen(QHostAddress("127.0.0.1"), PortPersistentWS); // TODO -> IP from startup arguments
}

void A3Dispatcher::onLocalCommandReceived()
{
    QFile in;
    in.open(stdin, QIODevice::ReadOnly);
    QByteArray ba = in.read(10000);
    QString message(ba);
    log("--->Message from ants3:\n" + message + "\n<---");

    processLocalCommand(message);
}

void A3Dispatcher::onReportProgressTimer()
{
    double total = 0;
    for (A3WorkerHandler * h : Handlers)
        total += h->EventsDone;

    if (WebSocketServer)
    {
        WebSocketServer->sendProgress(total);
    }
    else
    {
        std::cout << "$$>" << total << "<$$\n";
        std::cout.flush();
    }
}

void A3Dispatcher::processLocalCommand(const QString & message)
{
    QJsonObject json = jstools::strToJson(message.toUtf8()); // Local1?
    A3WorkDistrConfig wdc;
    wdc.readFromJson(json);

    log("Current dir:\n" + QDir::currentPath());
    log("Exchange dir:\n" + wdc.ExchangeDir);

    if (wdc.Nodes.empty())
    {
        localReplyError("Work is not scheduled!");
        return;
    }

    // if there is work to perform locally, it will be the first node
    if (wdc.Nodes.front().isLocalNode())
    {
        bool ok = startLocalWork(wdc.Command, wdc.ExchangeDir, wdc.Nodes.front());
        if (!ok)
        {
            localReplyError("Could not start local worker process(es)");
            // TODO -> abort all handlers
            clearHandlers();
            return;
        }
        //wdc.Nodes.removeFirst();
        wdc.Nodes.erase(wdc.Nodes.begin());
    }

    if (!wdc.Nodes.empty())
    {
        QString err = startRemoteWork(wdc);
        if (!err.isEmpty())
        {
            localReplyError(err);
            // TODO -> abort all handlers
            clearHandlers();
            return;
        }
    }

    waitForWorkFinished();
    //qDebug() << "DEBUG:FINISHED";

    clearHandlers();

    // TODO -> sim error processing

    localReplyFinished();
}

void A3Dispatcher::onRemoteCommandReceived(QJsonObject json)
{
    A3WorkDistrConfig wdc;
    wdc.readFromJson(json);

    A3WorkNodeConfig & Node = wdc.Nodes.front();

    bool ok = startLocalWork(wdc.Command, StandaloneDir, Node);
    if (!ok)
    {
        WebSocketServer->sendError("Could not start worker process(es)");
        // TODO -> abort all handlers
        clearHandlers();
        return;
    }

    waitForWorkFinished(); // some sending could be done during waiting?
    clearHandlers();

    log("Sent notification that the work is finished");
    // TODO -> sim error processing
    WebSocketServer->sendWorkFinished("");
}

bool A3Dispatcher::startLocalWork(const QString & executable, const QString & exchangeDir, const A3WorkNodeConfig & localNode)
{
    for (size_t iWorker = 0; iWorker < localNode.Workers.size(); iWorker++)
    {
        log("...starting worker #" + QString::number(iWorker));
        A3ProcessHandler * h = new A3ProcessHandler("./" + executable, {exchangeDir + '/' + localNode.Workers[iWorker].ConfigFile, exchangeDir});
        Handlers.push_back(h);
        bool ok = h->start();
        if (!ok) return false;
    }
    return true;
}

#include "a3remotehandler.h"
QString A3Dispatcher::startRemoteWork(const A3WorkDistrConfig & wdc)
{
    for (const A3WorkNodeConfig & Node : wdc.Nodes)
    {
        log("...connecting to farm node " + Node.Address);
        A3RemoteHandler * h = new A3RemoteHandler(Node, wdc.Command, wdc.ExchangeDir, wdc.CommonFiles);
        Handlers.push_back(h);
        bool ok = h->start();
        if (!ok) return "Could not connect to remote host " + Node.Address;
    }
    return "";
}

void A3Dispatcher::waitForWorkFinished()
{
    ProgressReportTimer->start();

    bool bAllStopped;
    do
    {
        qApp->processEvents();
        QThread::usleep(100);
        bAllStopped = true;
        for (A3WorkerHandler * h : Handlers)
        {
            if (h->isRunning()) bAllStopped = false;
            //break;
        }
    }
    while (!bAllStopped);

    ProgressReportTimer->stop();
    qApp->processEvents();
    log("All workers finished");
    qApp->processEvents();
}

void A3Dispatcher::localReplyFinished()
{
    std::cout << A3ProcessHandler::makeFinishMessage().toLatin1().data() << std::endl;
    std::cout.flush();
    log("Command processing completed\n");
}

void A3Dispatcher::localReplyError(const QString & ErrorMessage)
{
    std::cout << A3ProcessHandler::makeErrorMessage(ErrorMessage).toLatin1().data() << std::endl;
    std::cout.flush();
    log("Error reported to ants3: " + ErrorMessage);
}

void A3Dispatcher::log(const QString &text)
{
    *LogStream << text << '\n';
    LogStream->flush();
}

void A3Dispatcher::clearHandlers()
{
    for (A3WorkerHandler * h : Handlers) delete h;
    Handlers.clear();
}
