#include "a3dispatcher.h"
#include "a3processhandler.h"
#include "a3workdistrconfig.h" // in ANTS3 dir!
#include "ajsontools.h"        // in ANTS3 dir!

#ifdef WEBSOCKETS
    #include "awebsocketsessionserver.h"
    #include "a3remotehandler.h"
    #include <QHostAddress>
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QTextStream>
#include <QByteArray>
#include <QFile>
#include <QThread>
#include <QCoreApplication>
#include <QDir>
#include <QTimer>

#include <iostream>
#include <ios>

A3Dispatcher::A3Dispatcher(const QString & ip, quint16 port, int maxProcesses, QObject *parent) :
    QObject(parent),
    IP(ip), WebSocketPort(port), MaxNumberProcesses(maxProcesses)
{
    QString dir = QDir::currentPath();
    StandaloneDir = dir + '/' + "Dispatcher-" + QString::number(WebSocketPort);
    if (!QDir(StandaloneDir).exists()) QDir().mkdir(StandaloneDir);

    LogFile = new QFile(StandaloneDir + "/dispLog.txt");
    LogFile->open(QIODevice::WriteOnly | QFile::Text);
    LogStream = new QTextStream(LogFile);

    if (WebSocketPort != 0)
    {
#ifdef WEBSOCKETS
        WebSocketServer = new AWebSocketSessionServer(StandaloneDir, this);
        connect(WebSocketServer, &AWebSocketSessionServer::remoteCommandReceived, this, &A3Dispatcher::onRemoteCommandReceived, Qt::QueuedConnection);
        log("---->Standalone dispatcher started on WS port " + QString::number(port));
#else
        log("->Cannot start server: compilation without WEBSOCKET flag");
#endif
    }

    ProgressTimer = new QTimer();
    connect(ProgressTimer, &QTimer::timeout, this, &A3Dispatcher::onReportProgressTimer);
    ProgressTimer->setInterval(300);
}

A3Dispatcher::~A3Dispatcher()
{
    clearHandlers();

    log("====>Finished");
    LogFile->close();
    delete LogStream;
    delete LogFile;
}

void A3Dispatcher::start()
{
#ifdef WEBSOCKETS
    if (WebSocketPort != 0)
        WebSocketServer->startListen(QHostAddress("127.0.0.1"), WebSocketPort); // TODO -> IP from startup arguments !!!***
#endif
}

void A3Dispatcher::onReportProgressTimer()
{
    double total = 0;
    for (A3WorkerHandler * h : Handlers)
        total += h->EventsDone;

    if (WebSocketServer)
    {
#ifdef WEBSOCKETS
        WebSocketServer->sendProgress(total);
#endif
    }
    else
    {
        emit reportProgress(total);
    }
}

void A3Dispatcher::executeLocalCommand(QJsonObject json)
{
    A3WorkDistrConfig wdc;
    wdc.readFromJson(json);

    if (wdc.Command == "check")
    {
        checkFarmStatus(wdc);
        return;
    }

    //qDebug() << "Current dir:\n" << QDir::currentPath();
    //qDebug() << "Exchange dir:\n" << wdc.ExchangeDir;

    if (wdc.Nodes.empty())
    {
        localReplyError("Work is not scheduled!");
        return;
    }

    // if there is work to perform locally, it will be the first node
    if (wdc.Nodes.front().isLocalNode())
    {
        bool ok = startWorkHere(wdc.Command, wdc.ExchangeDir, wdc.Nodes.front());
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

#ifdef WEBSOCKETS
    if (!wdc.Nodes.empty())
    {
        QString err = startWorkFarm(wdc);
        if (!err.isEmpty())
        {
            localReplyError(err);
            // TODO -> abort all handlers
            clearHandlers();
            return;
        }
    }
#endif

    waitForWorkFinished();
    //qDebug() << "DEBUG:FINISHED";

    //bool ok = checkExitStatusOfWorkers();

    clearHandlers();

    // TODO -> sim error processing

    localReplyFinished();
}

bool A3Dispatcher::startWorkHere(const QString & executable, const QString & exchangeDir, const A3WorkNodeConfig & localNode)
{
    for (size_t iWorker = 0; iWorker < localNode.Workers.size(); iWorker++)
    {
        //log("...starting worker #" + QString::number(iWorker));
        //qDebug() << "...starting worker #" << iWorker;
        A3ProcessHandler * h = new A3ProcessHandler("./" + executable, {exchangeDir, localNode.Workers[iWorker].ConfigFile, QString::number(iWorker)});
        Handlers.push_back(h);
        bool ok = h->start();
        if (!ok) return false;
    }
    return true;
}

#ifdef WEBSOCKETS
void A3Dispatcher::onRemoteCommandReceived(QJsonObject json)
{
    A3WorkDistrConfig wdc;
    wdc.readFromJson(json);

    if (wdc.Command == "check")
    {
        qDebug() << "Received status request, answering with max number of processes of " << MaxNumberProcesses;
        WebSocketServer->sendStatus(MaxNumberProcesses);
        return;
    }

    A3WorkNodeConfig & Node = wdc.Nodes.front();

    bool ok = startWorkHere(wdc.Command, StandaloneDir, Node);
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

QString A3Dispatcher::startWorkFarm(const A3WorkDistrConfig & wdc)
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
#endif

void A3Dispatcher::waitForWorkFinished()
{
    ProgressTimer->start();

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

    ProgressTimer->stop();
    log("All workers finished");
    qApp->processEvents();
}

void A3Dispatcher::localReplyFinished()
{
    //qDebug() << "Signalling successful finish";
    QJsonObject js;
    js["Success"] = true;
    emit workFinished(js);
}

void A3Dispatcher::localReplyError(const QString & ErrorMessage)
{
    //qDebug() << "Signalling error";
    QJsonObject js;
    js["Success"] = false;
    js["Error"]   = ErrorMessage;
    emit workFinished(js);
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

void A3Dispatcher::checkFarmStatus(const A3WorkDistrConfig & wdc)
{
    qDebug() << "Checking farm status";

#ifdef WEBSOCKETS
    for (size_t iNode = 0; iNode < wdc.Nodes.size(); iNode++)
    {
        const A3WorkNodeConfig & Node = wdc.Nodes[iNode];
        {
            qDebug() << "...testing connection to to farm node " << Node.Address << ":" << Node.Port;
            A3RemoteHandler * h = new A3RemoteHandler(Node, wdc.Command, wdc.ExchangeDir, wdc.CommonFiles);
            Handlers.push_back(h);
            h->start();
        }
    }

    waitForWorkFinished();
    //qDebug() << "...finished";

    // !!!*** busy status?
    QJsonArray NodeStatus; // #processes, -1 if failed to connect

    for (A3WorkerHandler * h : Handlers)
    {
        A3RemoteHandler * rh = static_cast<A3RemoteHandler*>(h);

        QJsonObject json = jstools::strToJson(rh->Reply);
        qDebug() << "------------------>" << h->ErrorString << rh->Reply;
        int processes = -1;
        jstools::parseJson(json, "processes", processes);
        NodeStatus.push_back(processes);
    }

    clearHandlers();

    QJsonObject js;
    js["NodeStatus"] = NodeStatus;
    emit workFinished(js);

#else
    qDebug() << "WebSockets are not enabled!";
    localReplyFinished();
#endif
}
