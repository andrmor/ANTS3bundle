#include "a3remotehandler.h"
#include "a3wsclient.h"
#include "ajsontools.h"

#include <QDebug>
#include <QThread>

A3RemoteHandler::A3RemoteHandler(const A3WorkNodeConfig &Node, const QString &Command, const QString &ExchangeDir, const QVector<QString> &CommonFiles) :
    A3WorkerHandler(), Node(Node), Command(Command), ExchangeDir(ExchangeDir), CommonFiles(CommonFiles)
{
    Thread = new QThread();
    Client = new A3WSClient(Node, Command, ExchangeDir, CommonFiles);
    Client->moveToThread(Thread);

    connect(Thread, &QThread::started,         Client, &A3WSClient::start);
    //connect(this,   &A3RemoteHandler::doExit, Client, &A3WSClient::exit);
    //connect(Client, &A3WSClient::evalFinished, this,   &A3RemoteHandler::evalFinished);

    connect(Client, &A3WSClient::finished,     Thread, &QThread::quit);
    connect(Client, &A3WSClient::finished,     Client, &A3WSClient::deleteLater);
    connect(Thread, &QThread::finished,        Thread, &QThread::deleteLater);

    connect(Client, &A3WSClient::remoteWorkFinished, this, &A3RemoteHandler::onRemoteWorkFinished);
    connect(Client, &A3WSClient::progressReceived,   this, &A3RemoteHandler::onProgressReceived);
}

bool A3RemoteHandler::start()
{
    bRunning = true;
    Thread->start();
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
    //Client->SendText(txt, false);
}

void A3RemoteHandler::onRemoteWorkFinished(QString message)
{
    qDebug() << "DEBUG:WS->OneRemote:WorkFinished";
    bRunning = false;
}

void A3RemoteHandler::onProgressReceived(int eventsDone)
{
    EventsDone = eventsDone;
}
