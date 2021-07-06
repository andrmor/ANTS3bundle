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
    Server(new QWebSocketServer("Dispatcher", QWebSocketServer::NonSecureMode, this))
{
    connect(Server, &QWebSocketServer::newConnection, this, &AWebSocketSessionServer::onNewConnection);
    connect(Server, &QWebSocketServer::closed,        this, &AWebSocketSessionServer::closed);
}

AWebSocketSessionServer::~AWebSocketSessionServer()
{
    Server->close();
}

bool AWebSocketSessionServer::startListen(QHostAddress ip, quint16 port)
{
    if ( !Server->listen(ip, port) ) //QHostAddress::LocalHost QHostAddress::Any QHostAddress::AnyIPv4
    {
        QString s = QString("WebSocket was unable to start listening on IP ") + ip.toString() + " and port " + QString::number(port);
        qCritical() << s;
        return false;
    }

    qDebug() << "Dispatcher is listening:" << getUrl();
    return true;
}

QString AWebSocketSessionServer::getUrl() const
{
    return Server->serverUrl().toString();
}

int AWebSocketSessionServer::getPort() const
{
    return Server->serverPort();
}

void AWebSocketSessionServer::stopListen()
{
    Server->close();
}

bool AWebSocketSessionServer::isRunning()
{
    return Server->isListening();
}

bool AWebSocketSessionServer::isBinaryEmpty() const
{
    return ReceivedBinary.isEmpty();
}

void AWebSocketSessionServer::clearBinary()
{
    ReceivedBinary.clear();
}

bool AWebSocketSessionServer::assureCanReply()
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState) return true;

    QString err = "AWebSocketSessionServer: cannot reply - connection not established";
    qDebug() << err;
    emit requestAbort(err);
    return false;
}

void AWebSocketSessionServer::sendText(const QString & text)
{
    if ( !assureCanReply() ) return;

    qDebug() << "Sending text:" << text;
    Client->sendTextMessage(text);
}

void AWebSocketSessionServer::sendFileMoveProgress(int percents)
{
    if ( !assureCanReply() ) return;

    QString s = QString::number(percents);
    qDebug() << "Sending file move progress: " << s;
    Client->sendTextMessage("{ \"progress\" : " + s + " }");
}

void AWebSocketSessionServer::onNewConnection()
{
    qDebug() << "New connection attempt";
    QWebSocket * pSocket = Server->nextPendingConnection();

    qDebug() << QString("--- Connection request from ") + pSocket->peerAddress().toString();

    if (Client)
    {
        //deny - exclusive connections!
        qDebug() << "Connection denied: another client is already connected";
        pSocket->sendTextMessage("{ \"result\":false, \"error\" : \"Another client is already connected\" }");
        pSocket->close();
    }
    else
    {
        qDebug() << "Connection established with" << pSocket->peerAddress().toString();
        Client = pSocket;

        pSocket->setReadBufferSize(1000000);

        qDebug() << "--> Connection established";

        connect(pSocket, &QWebSocket::textMessageReceived,   this, &AWebSocketSessionServer::onTextMessageReceived);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &AWebSocketSessionServer::onBinaryMessageReceived);
        connect(pSocket, &QWebSocket::binaryFrameReceived,   this, &AWebSocketSessionServer::onBinaryFrameReceived);
        connect(pSocket, &QWebSocket::disconnected,          this, &AWebSocketSessionServer::onSocketDisconnected);

        //connect(pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
        //connect(pSocket, &QWebSocket::stateChanged, this, &AWebSocketSessionServer::onStateChanged);

        sendOK();
    }
}

void AWebSocketSessionServer::onTextMessageReceived(const QString & message)
{    
    NumFrames = 0; FileProgress = 0;
    qDebug() << "Text message received:\n--->\n"<<message << "\n<---";

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
            Client->sendBinaryMessage(ba);
            qDebug() << "Done, sending confirmation...";
            Client->sendTextMessage("{ \"binary\" : \"file\" }");
            qDebug() << "Done!";
        }
    }
    else if (message.isEmpty())
    {
        if (Client) Client->sendTextMessage("PONG"); //used for watchdogs
    }

    //
    //else emit textMessageReceived(message);
}

void AWebSocketSessionServer::onBinaryMessageReceived(const QByteArray &message)
{
    ReceivedBinary = message;
    NumFrames = 0; FileProgress = 0;

    //emit reportToGUI("    Binary message received");
    emit restartIdleTimer();

    qDebug() << "Binary message received. Length =" << message.length();

    sendOK();
}

void AWebSocketSessionServer::onBinaryFrameReceived(const QByteArray &, bool)
{
    //qDebug() << "Binary frame received. last?" << frame.size()<< isLastFrame;
    emit restartIdleTimer();
    NumFrames++;
    if (NumFrames % 10 == 0)
    {
        sendFileMoveProgress(FileProgress);
        FileProgress += 25; // not actual progress!
        if (FileProgress > 100) FileProgress = 0;
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
    qDebug() << "Client disconnected";

    if (Client) Client->deleteLater();
    Client = nullptr;

    emit clientDisconnected();
}

void AWebSocketSessionServer::sendOK()
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage("{ \"result\" : true }");
}

void AWebSocketSessionServer::sendProgress(int eventsDone)
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage( QString("$$>%0<$$\n").arg(eventsDone) );
}

void AWebSocketSessionServer::sendWorkFinished(const QString &error)
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage("{ \"status\" : \"finished\", \"error\" : \"" + error + "\" }");
}

void AWebSocketSessionServer::sendError(const QString &error)
{
    if (Client && Client->state() == QAbstractSocket::ConnectedState)
        Client->sendTextMessage("{ \"result\" : false, \"error\" : \"" + error + "\" }");
}
