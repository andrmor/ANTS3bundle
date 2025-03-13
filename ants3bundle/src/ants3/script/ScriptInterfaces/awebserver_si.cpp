#include "awebserver_si.h"
#include "awebsocketserver.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

AWebServerInterface::AWebServerInterface(AWebSocketServer & Server) :
    AScriptInterface(), Server(Server)
{
    QObject::connect(&Server, &AWebSocketServer::requestAbort, this, &AWebServerInterface::abort);
}

void AWebServerInterface::sendText(QString message)
{
    Server.replyWithText(message);
}

void AWebServerInterface::sendFile(QString fileName)
{
    Server.replyWithBinaryFile(fileName);
}

void AWebServerInterface::sendObject(QVariantMap object)
{
    Server.replyWithBinaryObject(object);
}

void AWebServerInterface::sendObjectAsJSON(QVariantMap object)
{
    Server.replyWithBinaryObject_asJSON(object);
}

bool AWebServerInterface::isBufferEmpty()
{
    return Server.isBinaryEmpty();
}

void AWebServerInterface::clearBuffer()
{
    Server.clearBinary();
}

QVariantMap AWebServerInterface::getBufferAsObject() const
{
    const QByteArray& ba = Server.getBinary();
    QJsonDocument doc =  QJsonDocument::fromJson(ba);
    QJsonObject json = doc.object();

    QVariantMap vm = json.toVariantMap();
    return vm;
}

bool AWebServerInterface::saveBufferToFile(QString fileName)
{
    const QByteArray& ba = Server.getBinary();
    qDebug() << "Preparing to save, ba size = " << ba.size();

    QFile saveFile(fileName);
    if ( !saveFile.open(QIODevice::WriteOnly) )
    {
        abort( QString("Cannot save binary reply to file: ") + fileName );
        return false;
    }
    saveFile.write(ba);
    saveFile.close();
    return true;
}

void AWebServerInterface::sendProgressReport(int percents)
{
    Server.replyProgress(percents);
}

void AWebServerInterface::setAcceptExternalProgressReport(bool flag)
{
    Server.setCanRetranslateProgress(flag);
}
