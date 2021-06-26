#include "a3remotehandler.h"
#include "awebsocketsession.h"
#include "ajsontools.h"

#include <QDebug>

A3RemoteHandler::A3RemoteHandler(const A3WorkNodeConfig &Node, const QString &Command, const QString &ExchangeDir, const QVector<QString> &CommonFiles) :
    A3WorkerHandler(), Node(Node), Command(Command), ExchangeDir(ExchangeDir), CommonFiles(CommonFiles) {}

bool A3RemoteHandler::start()
{
    Session = new AWebSocketSession();
    connect(Session, &AWebSocketSession::remoteWorkFinished, this, &A3RemoteHandler::onRemoteWorkFinished, Qt::QueuedConnection);
    connect(Session, &AWebSocketSession::progressReceived, this, &A3RemoteHandler::onProgressReceived, Qt::QueuedConnection);

    bool ok = Session->Connect( QString("ws://%0:%1").arg(Node.Address).arg(Node.Port), true );
    if (!ok) return false;

    for (const A3NodeWorkerConfig & worker : Node.Workers)
    {
        bool ok = Session->SendFile(ExchangeDir + '/' + worker.ConfigFile, worker.ConfigFile);
        if (!ok) return false;
        for (const QString & fn : worker.InputFiles)
        {
            ok = Session->SendFile(ExchangeDir + '/' + fn, fn);
            if (!ok) return false;
        }
        for (const QString & fn : CommonFiles)
        {
            ok = Session->SendFile(ExchangeDir + '/' + fn, fn);
            if (!ok) return false;
        }
    }

    A3WorkDistrConfig cf;
    cf.Command = Command;
    cf.Nodes << Node;

    QJsonObject js;
    cf.writeToJson(js);
    bRunning = true;
    //bResultsReady = false;
    ok = Session->SendText(jstools::jsonToString(js), false); //this is signal to start
    return true;
}

void A3RemoteHandler::abort()
{

}

bool A3RemoteHandler::isRunning()
{
    return bRunning;
}

void A3RemoteHandler::sendMessage(QString txt)
{
    Session->SendText(txt, false);
}

void A3RemoteHandler::transferOutputFiles()
{
    for (const A3NodeWorkerConfig & worker : Node.Workers)
    {
        for (const QString & fn : worker.OutputFiles)
        {
            qDebug() << "DEBUG:WS->Requesting file"<< fn << ExchangeDir + '/' + fn;
            bool ok = Session->RequestFile(fn, ExchangeDir + '/' + fn);
            qDebug() << "DEBUG:WS->Result:"<<ok;
        }
    }
    bRequestTransfer = false;
    bRunning = false;
    Session->Disconnect();
}

void A3RemoteHandler::onRemoteWorkFinished(QString message)
{
    //qDebug() << "DEBUG:WS->OneRemote:WorkFinished";
    bRequestTransfer = true;
    //bRunning = false;
}

void A3RemoteHandler::onProgressReceived(int eventsDone)
{
    EventsDone = eventsDone;
}
