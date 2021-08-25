#include "a3wsclient.h"
#include "awebsocketsession.h"
#include "ajsontools.h"

A3WSClient::A3WSClient(const A3WorkNodeConfig & Node, const QString & Command, const QString & ExchangeDir, const std::vector<QString> &CommonFiles) :
    Node(Node), Command(Command), ExchangeDir(ExchangeDir), CommonFiles(CommonFiles) {}

A3WSClient::~A3WSClient()
{
    //qDebug() << "DEBUG:Destr for A3WSClient";
}

bool A3WSClient::start()
{
    Session = new AWebSocketSession(this);

    connect(Session, &AWebSocketSession::remoteWorkFinished, this, &A3WSClient::onWorkFinished, Qt::QueuedConnection);
    connect(Session, &AWebSocketSession::progressReceived,   this, &A3WSClient::progressReceived, Qt::QueuedConnection);

    bool ok = Session->connect( QString("ws://%0:%1").arg(Node.Address).arg(Node.Port), true );
    if (!ok)
    {
        ErrorString = "Cannot conect";
        reportFinished();
        return false;
    }

    for (const A3NodeWorkerConfig & worker : Node.Workers)
    {
        bool ok = true;
        if (!worker.ConfigFile.isEmpty())
            ok = Session->sendFile(ExchangeDir + '/' + worker.ConfigFile, worker.ConfigFile);
        if (!ok)
        {
            reportFinished();
            return false;
        }
        for (const QString & fn : worker.InputFiles)
        {
            ok = Session->sendFile(ExchangeDir + '/' + fn, fn);
            if (!ok)
            {
                reportFinished();
                return false;
            }
        }
        for (const QString & fn : CommonFiles)
        {
            ok = Session->sendFile(ExchangeDir + '/' + fn, fn);
            if (!ok)
            {
                reportFinished();
                return false;
            }
        }
    }

    A3WorkDistrConfig cf;
    cf.Command = Command;
    cf.Nodes.push_back(Node);

    QJsonObject js;
    cf.writeToJson(js);

    ok = Session->sendText(jstools::jsonToString(js), false);
    return ok;
}

void A3WSClient::onWorkFinished(QString message)
{
    for (const A3NodeWorkerConfig & worker : Node.Workers)
    {
        for (const QString & fn : worker.OutputFiles)
        {
            qDebug() << "DEBUG:WCL->Requesting file"<< fn << ExchangeDir + '/' + fn;
            bool ok = Session->requestFile(fn, ExchangeDir + '/' + fn);
            qDebug() << "DEBUG:WCL->Result:"<<ok;
        }
    }

    reportFinished(message);
}

void A3WSClient::reportFinished(QString message)
{
    emit remoteWorkFinished(message);
    emit finished();
}
