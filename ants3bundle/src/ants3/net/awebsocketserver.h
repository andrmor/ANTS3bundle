#ifndef AWEBSOCKETSERVER_H
#define AWEBSOCKETSERVER_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>

class QWebSocketServer;
class QWebSocket;
class QHostAddress;

// Note there is a similar AWebSocketSessionServer class in Dispatcher,
// which is dedicated for doistributing ANTS3 work.
// This class (AWebSocketServer) is dedicated to the ANTS3 web server only
class AWebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit AWebSocketServer(QObject * parent = nullptr);
    ~AWebSocketServer();

    bool startListen(QHostAddress ip, quint16 port);
    void stopListen();
    bool isRunning();

    void replyWithText(const QString & message);
    void replyWithTextFromObject(const QVariantMap & object);
    void replyWithBinaryFile(const QString & fileName);
    void replyWithBinaryObject(const QVariantMap & object);
    void replyWithBinaryObject_asJSON(const QVariantMap & object);
    void replyWithQByteArray(const QByteArray & ba);

    void replyProgress(int percents);
    void setCanRetranslateProgress(bool flag) {bRetranslateProgress = flag;}

    bool isReplied() const {return bReplied;}
    bool isBinaryEmpty() const {return ReceivedBinary.isEmpty();}
    void clearBinary() {ReceivedBinary.clear();}

    const QByteArray & getBinary() const {return ReceivedBinary;}

    QString getUrl() const;
    int     getPort() const;

    void sendOK();
    void sendError(const QString& error);

    void disconnectClient();

public slots:
    void onProgressChanged(int percents);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onSocketDisconnected();

signals:
    void textMessageReceived(const QString &message);
    void clientDisconnected();
    void closed();
    void reportToGUI(const QString & text);
    void requestAbort(const QString reason);

private:
    QWebSocketServer * Server = nullptr;
    QWebSocket       * Client = nullptr;

    bool bDebug = true;

    QByteArray ReceivedBinary;

    bool bReplied = false;
    bool bRetranslateProgress = false;

private:
    bool assureCanReply();
};

#endif // AWEBSOCKETSESSIONSERVER_H
