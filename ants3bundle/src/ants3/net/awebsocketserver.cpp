#include "awebsocketserver.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QDebug>
#include <QNetworkInterface>
#include <QFile>
#include <QHostAddress>

AWebSocketServer &AWebSocketServer::getInstance()
{
    static AWebSocketServer instance;
    return instance;
}

AWebSocketServer::AWebSocketServer() :
    Server(new QWebSocketServer(QStringLiteral("ANTS3"), QWebSocketServer::NonSecureMode, this))
{
    connect(Server, &QWebSocketServer::newConnection, this, &AWebSocketServer::onNewConnection);
    connect(Server, &QWebSocketServer::closed,        this, &AWebSocketServer::closed);
}

AWebSocketServer::~AWebSocketServer()
{
    Server->close();
}

bool AWebSocketServer::startListen(QHostAddress ip, quint16 port)
{
    if ( !Server->listen(ip, port) ) //QHostAddress::LocalHost QHostAddress::Any QHostAddress::AnyIPv4
    {
        QString s = QString("WebSocket was unable to start listening on IP ") + ip.toString() + " and port " + QString::number(port);
        qCritical() << s.toLatin1();
        return false;
    }

    if (bDebug)
    {
        qDebug() << "WebSocket server started";
        qDebug() << "--URL:" << getUrl();
    }

    emit reportToGUI("> Server started listening -> URL: " + getUrl());

    return true;
}

void AWebSocketServer::stopListen()
{
    Server->close();
    qDebug() << "WebSocket server stopped";
    emit reportToGUI("< Server is not listening\n");
}

bool AWebSocketServer::isRunning()
{
    return Server->isListening();
}

bool AWebSocketServer::assureCanReply()
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState) return true;

    QString err = "AWebSocketServer: cannot reply - connection not established";
    qDebug() << err;
    emit requestAbort(err);
    return false;
}

void AWebSocketServer::replyWithText(const QString & message)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Reply text message:" << message;

    Client->sendTextMessage(message);
    bReplied = true;
}

void AWebSocketServer::replyWithTextFromObject(const QVariantMap & object)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Reply with object as text";

    QJsonObject js = QJsonObject::fromVariantMap(object);
    QJsonDocument doc(js);
    QString s(doc.toJson(QJsonDocument::Compact));
    Client->sendTextMessage(s);
    //{
    //    QString err = "ReplyWithTextFromObject argument is not object";
    //    qDebug() << err;
    //    sendError(err);
    //}
    bReplied = true;
}

void AWebSocketServer::replyWithBinary_File(const QString & fileName)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply from file:" << fileName;

    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) )
    {
        QString err = "Cannot open file: ";
        err += fileName;
        qDebug() << err;
        sendError(err);
    }
    else
    {
        QByteArray ba = file.readAll();
        Client->sendBinaryMessage(ba);
        Client->sendTextMessage("{ \"binary\" : \"file\" }");
    }
    bReplied = true;
}

void AWebSocketServer::replyWithBinary_JSON(const QVariantMap & object)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply from object as JSON";

    QJsonObject js = QJsonObject::fromVariantMap(object);
    QJsonDocument doc(js);
    Client->sendBinaryMessage(doc.toJson());
    Client->sendTextMessage("{ \"binary\" : \"json\" }");
    //{
    //    qDebug() << "Reply from object failed";
    //    sendError("ReplyWithBinary_JSON argument is not object");
    //}
    bReplied = true;
}

void AWebSocketServer::replyWithQByteArray(const QByteArray & ba)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply: QByteArray";

    Client->sendBinaryMessage(ba);
    Client->sendTextMessage("{ \"binary\" : \"qbytearray\" }");
    bReplied = true;
}

void AWebSocketServer::onNewConnection()
{
    if (bDebug) qDebug() << "New connection attempt";
    QWebSocket * pSocket = Server->nextPendingConnection();

    emit reportToGUI(QString("--- Connection request from ") + pSocket->peerAddress().toString());

    if (Client)
    {
        //deny - exclusive connections!
        if (bDebug) qDebug() << "Connection denied: another client is already connected";
        emit reportToGUI("--X Denied: another session is already active");
        pSocket->sendTextMessage("{ \"result\":false, \"error\" : \"Another client is already connected\" }");
        pSocket->close();
    }
    else
    {
        if (bDebug) qDebug() << "Connection established with" << pSocket->peerAddress().toString();
        Client = pSocket;

        emit reportToGUI("--> Connection established");

        connect(pSocket, &QWebSocket::textMessageReceived,   this, &AWebSocketServer::onTextMessageReceived);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &AWebSocketServer::onBinaryMessageReceived);
        connect(pSocket, &QWebSocket::disconnected,          this, &AWebSocketServer::onSocketDisconnected);

        sendOK();
    }
}

#include "ascripthub.h"
#include "ajscriptmanager.h"
#include <QApplication>
#include <QThread>
void AWebSocketServer::onTextMessageReceived(const QString &message)
{    
    bReplied = false;

    if (bDebug) qDebug() << "Text message received:\n--->\n"<<message << "\n<---";

    //emit reportToGUI("    Text message received");

    if (message.isEmpty())
    {
        if (Client)
            Client->sendTextMessage("PONG"); //used for watchdogs
        return;
    }

    //emit textMessageReceived(message);

    qDebug() << "-->Evaluating as JavaScript";

    AScriptHub & ScriptHub = AScriptHub::getInstance();
    AJScriptManager & ScriptManager = ScriptHub.getJScriptManager();

    bool ok = ScriptManager.evaluate(message);
    if (!ok)
    {
        qDebug() << "-->Failed to start script evaluation (worker is busy)";
        sendError( QString("Failed to start script evaluation, worker is busy") );
        return;
    }

    do
    {
        QApplication::processEvents();
        QThread::usleep(100);
        //if (AbortRequestTmp) ScriptManager.abort();
    }
    while ( !ScriptManager.isAborted() && !ScriptManager.isFinished());
    //qDebug() << ScriptManager.isAborted() << ScriptManager.isFinished();

    if (ScriptManager.isError())
    {
        qDebug() << "-->Evaluation error:" << ScriptManager.getErrorDescription() << "in line" << ScriptManager.getErrorLineNumber();
        sendError( QString("Script error -> %0 in line %1").arg(ScriptManager.getErrorDescription()).arg(ScriptManager.getErrorLineNumber()) );
    }
    else if (ScriptManager.isAborted())
    {
        qDebug() << "-->Evaluation aborted";
        sendError( QString("Script eval aborted") );
    }
    else
    {
        qDebug() << "-->Evaluation success";
        if ( !isReplied() )
        {
            qDebug() << "-->Generating reply";
            QVariant res = ScriptManager.getResult();
            qDebug() << "-->Result:" << res;
            replyWithText("{ \"result\" : true, \"evaluation\" : \"" + res.toString() + "\" }");
        }
        else qDebug() << "-->Not need to reply, script already generated one";
    }
}

void AWebSocketServer::onBinaryMessageReceived(const QByteArray & message)
{
    ReceivedBinary = message;

    //emit reportToGUI("    Binary message received");

    if (bDebug) qDebug() << "Binary message received. Length =" << message.length();

    sendOK();
}

void AWebSocketServer::onSocketDisconnected()
{
    if (bDebug) qDebug() << "Client disconnected";

    emit reportToGUI("<-- Client disconnected");

    if (Client)
    {
        Client->close();
        delete Client;
    }
    Client = nullptr;

    emit clientDisconnected();
}

void AWebSocketServer::sendOK()
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage("{ \"result\" : true }");
}

void AWebSocketServer::sendError(const QString &error)
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage("{ \"result\" : false, \"error\" : \"" + error + "\" }");
}

void AWebSocketServer::disconnectClient()
{
    onSocketDisconnected();
}

QString AWebSocketServer::getUrl() const
{
    return Server->serverUrl().toString();
}

int AWebSocketServer::getPort() const
{
    return Server->serverPort();
}
