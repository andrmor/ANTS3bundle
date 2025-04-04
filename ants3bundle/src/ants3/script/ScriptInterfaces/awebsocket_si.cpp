#include "awebsocket_si.h"
#include "awebsocketsession.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

QJsonObject strToObject(const QString & s)
{
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
    return doc.object();
}

AWebSocket_SI::AWebSocket_SI() :
    AScriptInterface()
{
    Description = "Interface to the web socket client\n"""
                  "If connecting to ANTS3 web socket server (see MainWindow->Settings->Servers),\n"
                  "can be used together with the webserver script interface for two-directional communication";

    Help["connect"] = "Connect to a web socket server";
    Help["disconnect"] = "Break the establihsed connection";

    Help["sendText"] = "Send text message, returns the reply text received from the server";
    Help["sendObject"] = "Send script object to the input binary buffer of the server";
    Help["sendFile"] = "Send file content to the input binary buffer of the server";

    Help["resumeWaitForAnswer"] = "Resumes waiting for the text reply from the server, and returns the received text messeage";

    Help["getBinaryReplyAsObject"] = "Get content of the binary input buffer as script object";
    Help["saveBinaryReplyToFile"] = "Save content of the binary input buffer to a file";

    Help["setTimeout"] = "Set timeout (in milliseconds) in wait for text reply from the server. Default value is 3000";

}

AWebSocket_SI::~AWebSocket_SI()
{
    if (Socket) Socket->deleteLater();
}

void AWebSocket_SI::abortRun()
{
    if (Socket) Socket->externalAbort();
}

void AWebSocket_SI::setTimeout(int milliseconds)
{
    TimeOut = milliseconds;

    if (Socket) Socket->setTimeout(milliseconds);
}

QString AWebSocket_SI::connect(QString url, bool getAnswerOnConnection)
{
    if (!Socket)
    {
        Socket = new AWebSocketSession();
        Socket->setTimeout(TimeOut);
    }

    bool bOK = Socket->connect(url, getAnswerOnConnection);
    if (bOK)
    {
        return Socket->getTextReply();
    }
    else
    {
        abort(Socket->getError());
        return "";
    }
}

void AWebSocket_SI::disconnect()
{
    if (Socket) Socket->disconnect();
}

QString AWebSocket_SI::sendText(QString message)
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = Socket->sendText(message);
    if (bOK)
        return Socket->getTextReply();
    else
    {
        abort(Socket->getError());
        return "";
    }
}

QString AWebSocket_SI::sendObject(QVariantMap object)
{
    QJsonObject js = QJsonObject::fromVariantMap(object);
    return sendQJsonObject(js);
}

QString AWebSocket_SI::sendQJsonObject(const QJsonObject & json)
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = Socket->sendJsonObject(json);
    if (bOK)
        return Socket->getTextReply();
    else
    {
        abort(Socket->getError());
        return "";
    }
}

/*
QString AWebSocket_SI::sendQByteArray(const QByteArray &ba)
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = Socket->sendQByteArray(ba);
    if (bOK)
        return Socket->getTextReply();
    else
    {
        abort(Socket->getError());
        return "";
    }
}
*/

QString AWebSocket_SI::sendFile(QString fileName)
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = Socket->sendFile(fileName, fileName);
    if (bOK)
        return Socket->getTextReply();
    else
    {
        abort(Socket->getError());
        return "";
    }
}

QString AWebSocket_SI::resumeWaitForAnswer()
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = Socket->resumeWaitForAnswer();
    if (bOK)
        return Socket->getTextReply();
    else
    {
        abort(Socket->getError());
        return "";
    }
}

QVariantMap AWebSocket_SI::getBinaryReplyAsObject()
{
    QVariantMap vm;
    if (!Socket)
    {
        abort("Web socket was not connected");
        return vm;
    }
    const QByteArray & ba = Socket->getBinaryReply();
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    QJsonObject json = doc.object();

    vm = json.toVariantMap();
    return vm;
}

bool AWebSocket_SI::saveBinaryReplyToFile(QString fileName)
{
    if (!Socket)
    {
        abort("Web socket was not connected");
        return "";
    }
    const QByteArray& ba = Socket->getBinaryReply();
    qDebug() << "ByteArray to save size:"<<ba.size();

    QFile saveFile(fileName);
    if ( !saveFile.open(QIODevice::WriteOnly) )
    {
        abort( QString("Server: Cannot save binary to file: ") + fileName );
        return false;
    }
    saveFile.write(ba);
    saveFile.close();
    return true;
}
