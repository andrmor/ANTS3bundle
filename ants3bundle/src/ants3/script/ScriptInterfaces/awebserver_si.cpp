#include "awebserver_si.h"
#include "awebsocketserver.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

AWebServer_SI::AWebServer_SI() :
    AScriptInterface(), Server(AWebSocketServer::getInstance())
{
    QObject::connect(&Server, &AWebSocketServer::requestAbort, this, &AWebServer_SI::abort);
}

#include <QTimer>
void AWebServer_SI::sendText(QString message)
{
    //Server.replyWithText(message);
    QTimer::singleShot(0, &Server, [this, message]()
    {
        Server.replyWithText(message);
    } );
}

void AWebServer_SI::sendFile(QString fileName)
{
    //Server.replyWithBinaryFile(fileName);
    QTimer::singleShot(0, &Server, [this, fileName]()
    {
        Server.replyWithBinaryFile(fileName);
    } );
}

void AWebServer_SI::sendObject(QVariantMap object)
{
    //Server.replyWithBinaryObject(object);
    QTimer::singleShot(0, &Server, [this, object]()
    {
        Server.replyWithBinaryObject(object);
    } );
}

void AWebServer_SI::sendObjectAsJSON(QVariantMap object)
{
    //Server.replyWithBinaryObject_asJSON(object);
    QTimer::singleShot(0, &Server, [this, object]()
    {
        Server.replyWithBinaryObject_asJSON(object);
    } );
}

bool AWebServer_SI::isBufferEmpty()
{
    return Server.isBinaryEmpty();
}

void AWebServer_SI::clearBuffer()
{
    Server.clearBinary();
}

QVariantMap AWebServer_SI::getBufferAsObject() const
{
    const QByteArray& ba = Server.getBinary();
    QJsonDocument doc =  QJsonDocument::fromJson(ba);
    QJsonObject json = doc.object();

    QVariantMap vm = json.toVariantMap();
    return vm;
}

bool AWebServer_SI::saveBufferToFile(QString fileName)
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

void AWebServer_SI::sendProgressReport(int percents)
{
    Server.replyProgress(percents);
}

void AWebServer_SI::setAcceptExternalProgressReport(bool flag)
{
    Server.setCanRetranslateProgress(flag);
}
