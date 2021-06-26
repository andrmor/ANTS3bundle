#include "awebsocketsession.h"

#include <QWebSocket>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QThread>
#include "ajsontools.h"

AWebSocketSession::AWebSocketSession() : QObject()
{
    socket = new QWebSocket();
    QObject::connect(socket, &QWebSocket::connected, this, &AWebSocketSession::onConnect);
    QObject::connect(socket, &QWebSocket::disconnected, this, &AWebSocketSession::onDisconnect);
    QObject::connect(socket, &QWebSocket::textMessageReceived, this, &AWebSocketSession::onTextMessageReceived);
    QObject::connect(socket, &QWebSocket::binaryMessageReceived, this, &AWebSocketSession::onBinaryMessageReceived);

    //QObject::connect(socket, &QWebSocket::stateChanged, this, &AWebSocketSession::onStateChanged);
}

AWebSocketSession::~AWebSocketSession()
{
    socket->deleteLater();
}

bool AWebSocketSession::Connect(const QString &Url, bool WaitForAnswer)
{
    fExternalAbort = false;
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
        socket->open( QUrl(Url) );
        do
        {
            QThread::msleep(sleepDuration);
            qApp->processEvents();
            if (fExternalAbort)
            {
                socket->abort();
                State = Aborted;
                Error = "Aborted!";
                return false;
            }
            if (timer.elapsed() > timeout)
            {
                socket->abort();
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
        peerPort = socket->peerPort();
        return true;
    }
    else
    {
        socket->close();
        State = Idle;
        return false;
    }
}

void AWebSocketSession::Disconnect()
{
    QElapsedTimer timer;
    timer.start();

    socket->close();
    while (socket->state() != QAbstractSocket::UnconnectedState)
    {
        if (fExternalAbort) break;
        QThread::msleep(10);
        qApp->processEvents();
        if (timer.elapsed() > timeoutDisc )
        {
            socket->abort();
            break;
        }
    }

    State = Idle;
}

int AWebSocketSession::Ping()
{
    if (State == Connected)
    {
        bool bOK = SendText("");
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

bool AWebSocketSession::ConfirmSendPossible()
{
    Error.clear();
    TextReply.clear();

    if (State != Connected  ||  socket->state() != QAbstractSocket::ConnectedState)
    {
        Error = "Not connected to server";
        return false;
    }

    return true;
}

void AWebSocketSession::onSendMessageRequest(QString txt)
{
    SendText(txt, false);
}

bool AWebSocketSession::waitForReply()
{
    bWaitForAnswer = true;
    QElapsedTimer timer;
    timer.start();

    do
    {
        QThread::msleep(sleepDuration);
        qApp->processEvents();
        if (fExternalAbort)
        {
            qDebug() << "DEBUG:WS->||| External abort!";
            socket->abort();
            State = Aborted;
            Error = "Aborted!";
            return false;
        }
        if (timer.elapsed() > timeout)
        {
            qDebug() << "DEBUG:WS->||| Timeout on waiting for reply";
            socket->abort();
            State = Idle;
            Error = "Timeout!";
            return false;
        }
    }
    while (bWaitForAnswer);
    TimeMs = timer.elapsed();

    return true;
}

bool AWebSocketSession::SendText(const QString &message, bool bWaitReply)
{
    if ( !ConfirmSendPossible() ) return false;

    socket->sendTextMessage(message);

    if (bWaitReply) return waitForReply();
    else return true;
}

bool AWebSocketSession::SendJson(const QJsonObject &json, bool bWaitReply)
{
    if ( !ConfirmSendPossible() ) return false;

    QJsonDocument doc(json);
    QByteArray ba = doc.toBinaryData();

    socket->sendBinaryMessage(ba);

    if (bWaitReply) return waitForReply();
    else return true;
}

bool AWebSocketSession::SendQByteArray(const QByteArray &ba)
{
    if ( !ConfirmSendPossible() ) return false;

    socket->sendBinaryMessage(ba);

    return waitForReply();
}

bool AWebSocketSession::SendFile(const QString &fileName, const QString & remoteFileName)
{
    if ( !ConfirmSendPossible() ) return false;

    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) )
    {
        Error = "Cannot open file: ";
        Error += fileName;
        return false;
    }
    QByteArray ba = file.readAll();

    socket->sendBinaryMessage(ba);
    qDebug() << "DEBUG:WS->File buffer has been sent, listening for the reply";
    bool ok = waitForReply();
    if (!ok) return false;

    QJsonObject js;
    js["File"] = remoteFileName;
    return SendText(jstools::jsonToString(js));
}

bool AWebSocketSession::RequestFile(const QString &RemoteFileName, const QString &SaveAs)
{
    if ( !ConfirmSendPossible() ) return false;

    BinaryReply.clear();

    QJsonObject js;
    js["GetFile"] = RemoteFileName;
    QString message = jstools::jsonToString(js);
    qDebug() << "DEBUG:WS->Sending file request:" << message;

    bool ok = SendText(message, true);
    if (!ok)
    {

    }

    QFile file(SaveAs);
    if ( !file.open(QIODevice::WriteOnly) )
    {
        Error = "Cannot open file: ";
        Error += SaveAs;
        //return false;
    }
    file.write(BinaryReply);
    return true;
}

bool AWebSocketSession::ResumeWaitForAnswer()
{
    if ( !ConfirmSendPossible() ) return false;
    return waitForReply();
}

void AWebSocketSession::ClearReply()
{
    TextReply.clear();
    BinaryReply.clear();
}

void AWebSocketSession::ExternalAbort()
{
    //State = Aborted;
    //socket->abort();
    fExternalAbort = true;
}

void AWebSocketSession::onConnect()
{
    qDebug() << "DEBUG:WS->Connected to server";
    if (bWaitForAnswer) qDebug() << "DEBUG:WS->Waiting for confirmation";
    else State = Connected;
}

void AWebSocketSession::onDisconnect()
{
    if (State == Aborted)
    {
        qDebug() << "DEBUG:WS->Disconnect on abort";
        State = Idle;
    }
    else if (bWaitForAnswer)
    {
        qDebug() << "DEBUG:WS->Disconnected while attempting to establish connection";
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
    qDebug() << "DEBUG:WS->Text message received:" << message;
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
