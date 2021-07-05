#include "awebsocketsessionserver.h"
#include "ajsontools.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QNetworkInterface>
#include <QFile>
#include <QHostAddress>

AWebSocketSessionServer::AWebSocketSessionServer(const QString & tmpDataDir, QObject *parent) :
    QObject(parent), TmpDataDir(tmpDataDir),
    server(new QWebSocketServer("Dispatcher", QWebSocketServer::NonSecureMode, this))
{
    connect(server, &QWebSocketServer::newConnection, this, &AWebSocketSessionServer::onNewConnection);
    connect(server, &QWebSocketServer::closed, this, &AWebSocketSessionServer::closed);
}

AWebSocketSessionServer::~AWebSocketSessionServer()
{
    server->close();
}

bool AWebSocketSessionServer::StartListen(QHostAddress ip, quint16 port)
{
    if ( !server->listen(ip, port) ) //QHostAddress::LocalHost QHostAddress::Any QHostAddress::AnyIPv4
    {
        QString s = QString("WebSocket was unable to start listening on IP ") + ip.toString() + " and port " + QString::number(port);
        qCritical() << s;
        return false;
    }

    if (bDebug)
    {
        //qDebug() << "--Port:" << server->serverPort();
        qDebug() << "Dispatcher is listening:" << GetUrl();
    }
    return true;
}

void AWebSocketSessionServer::StopListen()
{
    server->close();
}

bool AWebSocketSessionServer::IsRunning()
{
    return server->isListening();
}

bool AWebSocketSessionServer::assureCanReply()
{
    if (client && client->state() == QAbstractSocket::ConnectedState) return true;

    QString err = "AWebSocketSessionServer: cannot reply - connection not established";
    qDebug() << err;
    emit requestAbort(err);
    return false;
}

void AWebSocketSessionServer::ReplyWithText(const QString &message)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Reply text message:"<<message;

    client->sendTextMessage(message);
    bReplied = true;
}

void AWebSocketSessionServer::ReplyWithTextFromObject(const QVariant &object)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Reply with object as text";

    if (object.type() == QVariant::Map)
    {
        QVariantMap vm = object.toMap();
        QJsonObject js = QJsonObject::fromVariantMap(vm);
        QJsonDocument doc(js);
        QString s(doc.toJson(QJsonDocument::Compact));
        client->sendTextMessage(s);
    }
    else
    {
        QString err = "ReplyWithTextFromObject argument is not object";
        qDebug() << err;
        sendError(err);
    }
    bReplied = true;
}

void AWebSocketSessionServer::SendBackFile(const QString &fileName, const QString &remoteFileName)
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
        client->sendBinaryMessage(ba);
        QJsonObject js;
        js["File"] = remoteFileName;
        client->sendTextMessage(jstools::jsonToString(js));
    }
    bReplied = true;
}

void AWebSocketSessionServer::ReplyWithBinaryFile(const QString &fileName)
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
        client->sendBinaryMessage(ba);
        client->sendTextMessage("{ \"binary\" : \"file\" }");
    }
    bReplied = true;
}

void AWebSocketSessionServer::ReplyWithBinaryObject(const QVariant &object)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply from object";

    if (object.type() == QVariant::Map)
    {
        QVariantMap vm = object.toMap();
        QJsonObject js = QJsonObject::fromVariantMap(vm);
        QJsonDocument doc(js);
        client->sendBinaryMessage(doc.toJson(QJsonDocument::Compact));
        client->sendTextMessage("{ \"binary\" : \"object\" }");
    }
    else
    {
        QString err = "Error: Reply with object failed";
        qDebug() << err;
        sendError("ReplyWithBinaryObject argument is not object");
    }
    bReplied = true;
}

void AWebSocketSessionServer::ReplyWithBinaryObject_asJSON(const QVariant &object)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply from object as JSON";

    if (object.type() == QVariant::Map)
    {
        QVariantMap vm = object.toMap();
        QJsonObject js = QJsonObject::fromVariantMap(vm);
        QJsonDocument doc(js);
        client->sendBinaryMessage(doc.toJson());
        client->sendTextMessage("{ \"binary\" : \"json\" }");
    }
    else
    {
        qDebug() << "Reply from object failed";
        sendError("ReplyWithBinaryObject_asJSON argument is not object");
    }
    bReplied = true;
}

void AWebSocketSessionServer::ReplyWithQByteArray(const QByteArray &ba)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Binary reply: QByteArray";

    client->sendBinaryMessage(ba);
    client->sendTextMessage("{ \"binary\" : \"qbytearray\" }");
    bReplied = true;
}

void AWebSocketSessionServer::ReplyProgress(int percents)
{
    if ( !assureCanReply() ) return;

    QString s = QString::number(percents);
    qDebug() << "Sending progress: " << s;

    client->sendTextMessage("{ \"progress\" : " + s + " }");
}

void AWebSocketSessionServer::onNewConnection()
{
    if (bDebug) qDebug() << "New connection attempt";
    QWebSocket *pSocket = server->nextPendingConnection();

    if (bDebug) qDebug() << QString("--- Connection request from ") + pSocket->peerAddress().toString();

    if (client)
    {
        //deny - exclusive connections!
        if (bDebug) qDebug() << "Connection denied: another client is already connected";
        pSocket->sendTextMessage("{ \"result\":false, \"error\" : \"Another client is already connected\" }");
        pSocket->close();
    }
    else
    {
        if (bDebug) qDebug() << "Connection established with" << pSocket->peerAddress().toString();
        client = pSocket;

        pSocket->setReadBufferSize(1000000);

        if (bDebug) qDebug() << "--> Connection established";

        connect(pSocket, &QWebSocket::textMessageReceived,   this, &AWebSocketSessionServer::onTextMessageReceived);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &AWebSocketSessionServer::onBinaryMessageReceived);
        connect(pSocket, &QWebSocket::binaryFrameReceived,   this, &AWebSocketSessionServer::onBinaryFrameReceived);
        connect(pSocket, &QWebSocket::disconnected,          this, &AWebSocketSessionServer::onSocketDisconnected);

        //connect(pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
        //connect(pSocket, &QWebSocket::stateChanged, this, &AWebSocketSessionServer::onStateChanged);

        sendOK();
    }
}

#include "ajsontools.h"
void AWebSocketSessionServer::onTextMessageReceived(const QString &message)
{    
    bReplied = false;

    NumFrames = 0; Progress = 0;
    if (bDebug) qDebug() << "Text message received:\n--->\n"<<message << "\n<---";

    QJsonObject json = jstools::strToJson(message);
    if (json.contains("File"))
    {
        if (ReceivedBinary.isEmpty()) sendError("Binmary is missing");
        else
        {
            QFile file(TmpDataDir + '/' + json["File"].toString());
            if (!file.open(QFile::WriteOnly))
            {
                sendError("cannot save file");
            }
            file.write(ReceivedBinary);
            file.close();
            sendOK();
        }
    }
    else if (json.contains("Command"))
    {
        emit remoteCommandReceived(json);
    }
    else if (json.contains("GetFile"))
    {
        QString fileName = TmpDataDir + '/' + json["GetFile"].toString();
        qDebug() << "Request recieved to send file:" << fileName;
        //sendOK();
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
            qDebug() << "Sending file as a binary message...";
            client->sendBinaryMessage(ba);
            qDebug() << "Done, sending confirmation...";
            client->sendTextMessage("{ \"binary\" : \"file\" }");
            qDebug() << "Done!";
        }
    }
    else if (message.isEmpty())
    {
        if (client) client->sendTextMessage("PONG"); //used for watchdogs
    }
    else
       emit textMessageReceived(message);
}

void AWebSocketSessionServer::onBinaryMessageReceived(const QByteArray &message)
{
    ReceivedBinary = message;
    NumFrames = 0; Progress = 0;

    //emit reportToGUI("    Binary message received");
    emit restartIdleTimer();

    if (bDebug) qDebug() << "Binary message received. Length =" << message.length();

    sendOK();
}

void AWebSocketSessionServer::onBinaryFrameReceived(const QByteArray &frame, bool isLastFrame)
{
    //qDebug() << "Binary frame received. last?" << frame.size()<< isLastFrame;
    emit restartIdleTimer();
    NumFrames++;
    if (NumFrames % 10 == 0)
    {
        ReplyProgress(Progress);
        Progress += 25;
        if (Progress > 100) Progress = 0;
    }
}

/*
void AWebSocketSessionServer::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "ERROR!!!!";
}
void AWebSocketSessionServer::onStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "STATECHANGE -   " << state;
}
*/

void AWebSocketSessionServer::onSocketDisconnected()
{
    if (bDebug) qDebug() << "Client disconnected";

    if (client) client->deleteLater();
    client = nullptr;

    /*
    if (client)
    {
        client->close();
        delete client;
    }
    client = 0;
    */

    emit clientDisconnected();
}

void AWebSocketSessionServer::sendOK()
{
    if (client && client->state() == QAbstractSocket::ConnectedState)
        client->sendTextMessage("{ \"result\" : true }");
}

void AWebSocketSessionServer::sendProgress(int eventsDone)
{
    if (client && client->state() == QAbstractSocket::ConnectedState)
        client->sendTextMessage( QString("$$>%0<$$\n").arg(eventsDone) );
}

void AWebSocketSessionServer::sendWorkFinished(const QString &error)
{
    if (client && client->state() == QAbstractSocket::ConnectedState)
        client->sendTextMessage("{ \"status\" : \"finished\", \"error\" : \"" + error + "\" }");
}

void AWebSocketSessionServer::sendError(const QString &error)
{
    if (client && client->state() == QAbstractSocket::ConnectedState)
        client->sendTextMessage("{ \"result\" : false, \"error\" : \"" + error + "\" }");
}

void AWebSocketSessionServer::DisconnectClient()
{
    onSocketDisconnected();
}

const QString AWebSocketSessionServer::GetUrl() const
{
    return server->serverUrl().toString();
}

int AWebSocketSessionServer::GetPort() const
{
    return server->serverPort();
}
