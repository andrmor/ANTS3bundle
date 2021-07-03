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

    connect(Session, &AWebSocketSession::remoteWorkFinished, this, &A3WSClient::onWorkFinished);
    connect(Session, &AWebSocketSession::progressReceived,   this, &A3WSClient::progressReceived);

    bool ok = Session->Connect( QString("ws://%0:%1").arg(Node.Address).arg(Node.Port), true );
    if (!ok)
    {
        ErrorString = "Cannot conect";
        return false;
    }

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
    cf.Nodes.push_back(Node);

    QJsonObject js;
    cf.writeToJson(js);

    ok = Session->SendText(jstools::jsonToString(js), false);
    return ok;
}

void A3WSClient::onWorkFinished(QString message)
{
    for (const A3NodeWorkerConfig & worker : Node.Workers)
    {
        for (const QString & fn : worker.OutputFiles)
        {
            //qDebug() << "DEBUG:WS->Requesting file"<< fn << ExchangeDir + '/' + fn;
            bool ok = Session->RequestFile(fn, ExchangeDir + '/' + fn);
            //qDebug() << "DEBUG:WS->Result:"<<ok;
        }
    }

    emit remoteWorkFinished(message);
    emit finished();
}
