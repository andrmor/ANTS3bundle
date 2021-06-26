#ifndef AWEBSOCKETSESSIONSERVER_H
#define AWEBSOCKETSESSIONSERVER_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>

#include <QAbstractSocket>

class QWebSocketServer;
class QWebSocket;
class QHostAddress;

class AWebSocketSessionServer : public QObject
{
    Q_OBJECT
public:
    explicit AWebSocketSessionServer(const QString & tmpDataDir, QObject *parent = 0);
    ~AWebSocketSessionServer();

    bool StartListen(QHostAddress ip, quint16 port);
    void StopListen();
    bool IsRunning();

    void SendBackFile(const QString & fileName, const QString & remoteFileName);

    void ReplyWithText(const QString& message);
    void ReplyWithTextFromObject(const QVariant& object);
    void ReplyWithBinaryFile(const QString& fileName);
    void ReplyWithBinaryObject(const QVariant& object);
    void ReplyWithBinaryObject_asJSON(const QVariant& object);
    void ReplyWithQByteArray(const QByteArray & ba);

    void ReplyProgress(int percents);  // used during sending large files
    //void SetCanRetranslateProgress(bool flag) {bRetranslateProgress = flag;}

    bool isReplied() const {return bReplied;}
    bool isBinaryEmpty() const {return ReceivedBinary.isEmpty();}
    void clearBinary() {ReceivedBinary.clear();}
    const QByteArray& getBinary() const {return ReceivedBinary;}

    const QString GetUrl() const;
    int GetPort() const;

    void sendOK();
    void sendProgress(int eventsDone);
    void sendWorkFinished(const QString& error);
    void sendError(const QString& error);

    void DisconnectClient();

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onBinaryFrameReceived(const QByteArray &frame, bool isLastFrame);
    //void onError(QAbstractSocket::SocketError error);
    //void onStateChanged(QAbstractSocket::SocketState state);
    void onSocketDisconnected();

signals:
    void textMessageReceived(const QString &message);
    void restartIdleTimer();
    void clientDisconnected();
    void closed();
    void requestAbort(const QString reason);

    void remoteCommandReceived(QJsonObject config);

private:
    QString            TmpDataDir;

    QWebSocketServer * server = nullptr;
    QWebSocket       * client = nullptr;


    bool               bDebug = true;
    //bool bCompressBinary = false;
    //int CompressionLevel = -1;

    QByteArray         ReceivedBinary;

    bool               bReplied = false;
    //bool               bRetranslateProgress = false;

    int                Progress  = 0;
    int                NumFrames = 0;

private:
    bool assureCanReply();
};

#endif // AWEBSOCKETSESSIONSERVER_H
