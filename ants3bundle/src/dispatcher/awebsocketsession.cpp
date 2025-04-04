#include "awebsocketsession.h"
#include "ajsontools.h"
#include "afiletools.h"

#include <QWebSocket>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QThread>

AWebSocketSession::AWebSocketSession(QObject *parent) : QObject(parent)
{
    Socket = new QWebSocket();
    QObject::connect(Socket, &QWebSocket::connected,             this, &AWebSocketSession::onConnect);
    QObject::connect(Socket, &QWebSocket::disconnected,          this, &AWebSocketSession::onDisconnect);
    QObject::connect(Socket, &QWebSocket::textMessageReceived,   this, &AWebSocketSession::onTextMessageReceived);
    QObject::connect(Socket, &QWebSocket::binaryMessageReceived, this, &AWebSocketSession::onBinaryMessageReceived);

    //QObject::connect(socket, &QWebSocket::stateChanged, this, &AWebSocketSession::onStateChanged);
}

AWebSocketSession::~AWebSocketSession()
{
    //qDebug() << "DEBUG:Dest for WebSocketSession";
    delete Socket;
    //socket->deleteLater();
}

bool AWebSocketSession::connect(const QString &Url, bool WaitForAnswer)
{
    bExternalAbort = false;
    Error.clear();
    TextReply.clear();
    BinaryReply.clear();
    bWaitForAnswer = WaitForAnswer;

    if ( !(State == Idle || State == Aborted) )
    {
        Error = "Cannot connect: not idle";
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    //waiting for connection
    do
    {
        State = Connecting;
        Error.clear();
        Socket->open( QUrl(Url) );
        do
        {
            QThread::msleep(SleepDuration);
            qApp->processEvents();
            if (bExternalAbort)
            {
                Socket->abort();
                State = Aborted;
                Error = "Aborted!";
                return false;
            }
            if (timer.elapsed() > Timeout)
            {
                Socket->abort();
                State = Idle;
                Error = "Connection timeout!";
                return false;
            }
        }
        while (State == Connecting);
    }
    while (State == ConnectionFailed);  //on server start-up, it refuses connections -> wait for fully operational

    if (Error.isEmpty())
    {
        PeerPort = Socket->peerPort();
        return true;
    }
    else
    {
        Socket->close();
        State = Idle;
        return false;
    }
}

void AWebSocketSession::disconnect()
{
    QElapsedTimer timer;
    timer.start();

    Socket->close();
    while (Socket->state() != QAbstractSocket::UnconnectedState)
    {
        if (bExternalAbort) break;
        QThread::msleep(10);
        qApp->processEvents();
        if (timer.elapsed() > TimeoutDisc )
        {
            Socket->abort();
            break;
        }
    }

    State = Idle;
}

int AWebSocketSession::ping()
{
    if (State == Connected)
    {
        bool bOK = sendText("");
        if (bOK)
            return TimeMs;
        else
            return false;
    }
    else
    {
        Error = "Not connected";
        return -1;
    }
}

bool AWebSocketSession::confirmSendPossible()
{
    Error.clear();
    TextReply.clear();

    if (State != Connected  ||  Socket->state() != QAbstractSocket::ConnectedState)
    {
        Error = "Not connected to server";
        return false;
    }

    return true;
}

void AWebSocketSession::onSendMessageRequest(QString txt)
{
    sendText(txt, false);
}

bool AWebSocketSession::waitForReply()
{
    bWaitForAnswer = true;
    QElapsedTimer timer;
    timer.start();

    do
    {
        QThread::msleep(SleepDuration);
        qApp->processEvents();
        if (bExternalAbort)
        {
            qDebug() << "DEBUG:WS->||| External abort!";
            Socket->abort();
            State = Aborted;
            Error = "Aborted!";
            return false;
        }
        if (timer.elapsed() > Timeout)
        {
            qDebug() << "DEBUG:WS->||| Timeout on waiting for reply";
            Socket->abort();
            State = Idle;
            Error = "Timeout!";
            return false;
        }
    }
    while (bWaitForAnswer);
    TimeMs = timer.elapsed();

    return true;
}

bool AWebSocketSession::sendText(const QString &message, bool bWaitReply)
{
    if ( !confirmSendPossible() ) return false;

    Socket->sendTextMessage(message);

    if (bWaitReply) return waitForReply();
    else return true;
}

bool AWebSocketSession::sendJsonObject(const QJsonObject & json)
{
    if ( !confirmSendPossible() ) return false;

    QJsonDocument doc(json);
    QByteArray ba = doc.toJson();

    Socket->sendBinaryMessage(ba);

    return waitForReply();
}

bool AWebSocketSession::sendFile(const QString & fileName, const QString & remoteFileName)
{
    if ( !confirmSendPossible() ) return false;

    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) )
    {
        Error = "Cannot open file: ";
        Error += fileName;
        return false;
    }
    QByteArray ba = file.readAll();

    Socket->sendBinaryMessage(ba);
    //qDebug() << "DEBUG:WS->File buffer has been sent, listening for the reply";
    bool ok = waitForReply();
    if (!ok) return false;

    QJsonObject js;
    js["File"] = remoteFileName;
    return sendText(jstools::jsonToString(js));
}

bool AWebSocketSession::requestFile(const QString &RemoteFileName, const QString &SaveAs)
{
    if ( !confirmSendPossible() ) return false;

    BinaryReply.clear();

    QJsonObject js;
    js["GetFile"] = RemoteFileName;
    QString message = jstools::jsonToString(js);
    //qDebug() << "DEBUG:WS->Sending file request:" << message;

    //bWaitForBinary = true;
    bool ok = sendText(message, true);
    if (!ok)
    {
        qDebug() << "DEBUG:WS->fail to send/receive answer on file request";
    }

//    ok = waitForBinaryReply(); //cannot set bWaitForNinary ->answer could come before wait is started
    qDebug() << "DEBUG:WS->Binary recieve status:" << ok;

    QFile file(SaveAs);
    if ( !file.open(QIODevice::WriteOnly) )
    {
        Error = "Cannot open file: ";
        Error += SaveAs;
        return false;
    }
    file.write(BinaryReply);
    file.close();

    //QString txt;
    //ftools::loadTextFromFile(txt, SaveAs);
    //qDebug() << "DEBUG:" << txt;

    return true;
}

bool AWebSocketSession::resumeWaitForAnswer()
{
    if ( !confirmSendPossible() ) return false;
    return waitForReply();
}

void AWebSocketSession::clearReply()
{
    TextReply.clear();
    BinaryReply.clear();
}

void AWebSocketSession::externalAbort()
{
    //State = Aborted;
    //socket->abort();
    bExternalAbort = true;
}

void AWebSocketSession::onConnect()
{
    //qDebug() << "DEBUG:WS->Connected to server";
    if (bWaitForAnswer)
    {
        //qDebug() << "DEBUG:WS->Waiting for confirmation";
    }
    else State = Connected;
}

void AWebSocketSession::onDisconnect()
{
    if (State == Aborted)
    {
        //qDebug() << "DEBUG:WS->Disconnect on abort";
        State = Idle;
    }
    else if (bWaitForAnswer)
    {
        //qDebug() << "DEBUG:WS->Disconnected while attempting to establish connection";
        if (State == Connecting)
            Error = "Server disconnected before confirming connection";
        else
            Error = "Server disconnected before reply";
        State = ConnectionFailed;
    }    
    else
    {
        //qDebug() << "DEBUG:WS->Clinet disconnected";
        State = Idle; //paranoic
    }
}

void AWebSocketSession::onTextMessageReceived(const QString &message)
{
    //qDebug() << "DEBUG:WS->Text message received:" << message;
    TextReply = message;
    bWaitForAnswer = false;

    if (message.startsWith("$$>") && message.endsWith("<$$\n"))
    {
        QString txt = message;
        txt.remove("$$>");
        txt.remove("<$$\n");
        int eventsDone = txt.toInt();
        //qDebug() << "DEBUG:WS->PROGRESS"<<eventsDone;
        emit progressReceived(eventsDone);
    }
    else if (message.contains("status"))
    {
        //qDebug() << "DEBUG:WS->Remote work finished!";
        emit remoteWorkFinished(message);
    }
    else if (State == Connecting)
    {
        if (message.startsWith("Error"))
        {
            State = ConnectionFailed;
            Error = "Connection failed";
            if ( message.contains("another client is already connected") )
                Error += ": another client is connected to the server";
        }
        else
            State = Connected;
    }
}

void AWebSocketSession::onBinaryMessageReceived(const QByteArray &message)
{
    qDebug() << "DEBUG:WS->Binary message received. Size = " << message.length();
    TextReply = "#binary";
    BinaryReply = message;
    bWaitForAnswer = false;
}

/*
void AWebSocketSession::onStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "DEBUG:WS->State changed!"<< state;
}
*/
