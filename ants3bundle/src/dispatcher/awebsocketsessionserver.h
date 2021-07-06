#ifndef AWEBSOCKETSESSIONSERVER_H
#define AWEBSOCKETSESSIONSERVER_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
//#include <QAbstractSocket>

class QWebSocketServer;
class QWebSocket;
class QHostAddress;

class AWebSocketSessionServer : public QObject
{
    Q_OBJECT
public:
    explicit AWebSocketSessionServer(const QString & tmpDataDir, QObject *parent = 0);
    ~AWebSocketSessionServer();

    QString getUrl() const;
    int     getPort() const;

    bool    startListen(QHostAddress ip, quint16 port);
    void    stopListen();
    bool    isRunning();

    void    sendProgress(int eventsDone);
    void    sendError(const QString& error);
    void    sendWorkFinished(const QString& error);

    bool    isBinaryEmpty() const;
    void    clearBinary();

private slots:
    void    onNewConnection();
    void    onTextMessageReceived(const QString &message);
    void    onBinaryMessageReceived(const QByteArray &message);
    void    onBinaryFrameReceived(const QByteArray &frame, bool isLastFrame);
    //void  onError(QAbstractSocket::SocketError error);
    //void  onStateChanged(QAbstractSocket::SocketState state);
    void    onSocketDisconnected();

signals:
    void    textMessageReceived(const QString & message);
    void    restartIdleTimer();
    void    clientDisconnected();
    void    closed();
    void    requestAbort(QString reason);

    void    remoteCommandReceived(QJsonObject config);

private:
    QString            TmpDataDir;

    QWebSocketServer * Server = nullptr;
    QWebSocket       * Client = nullptr;

    //bool             bCompressBinary = false;
    //int              CompressionLevel = -1;

    QByteArray         ReceivedBinary;

    int                FileProgress  = 0;
    int                NumFrames = 0;

private:
    bool assureCanReply();
    void sendOK();
    void sendText(const QString & text);
    void sendFileMoveProgress(int percents);
};

#endif // AWEBSOCKETSESSIONSERVER_H
