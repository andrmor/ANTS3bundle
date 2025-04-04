#include "awebserver_si.h"
#include "awebsocketserver.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>

AWebServer_SI::AWebServer_SI() :
    AScriptInterface(), Server(AWebSocketServer::getInstance())
{
    QObject::connect(&Server, &AWebSocketServer::requestAbort, this, &AWebServer_SI::abort);

    Description = "Interface to the web socket server (see MainWindow->Settings->Servers)\n"
                  "Can be used by a remote web socket client after establising connection to the server.";

    Help["sendText"] = "Send text to the connected client";
    Help["sendFile"] = "Send file to the input binary buffer of the connected client";
    Help["sendObject"] = "Send script object the input binary buffer of the connected client";

    Help["isBufferEmpty"] = "Check if the input binary buffer (can be used by the client to send binary data) is empty.";
    Help["clearBuffer"] = "Clear the input binary buffer. The buffer can be used by the client to send binary data";

    Help["getBufferAsObject"] = "Load script object from input binary buffer";
    Help["saveBufferToFile"] = "Save content of the input binary buffer to file";
}

void AWebServer_SI::sendText(QString message)
{
    //Server.replyWithText(message);  // cannot be used directly: thread conflict!
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
        Server.replyWithBinary_File(fileName);
    } );
}

void AWebServer_SI::sendObject(QVariantMap object)
{
    //Server.replyWithBinaryObject(object);
    QTimer::singleShot(0, &Server, [this, object]()
    {
        Server.replyWithBinary_JSON(object);
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
