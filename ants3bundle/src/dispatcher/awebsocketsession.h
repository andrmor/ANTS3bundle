#ifndef AWEBSOCKETSESSION_H
#define AWEBSOCKETSESSION_H

#include <QObject>
#include <QVariant>
#include <QByteArray>
//#include <QAbstractSocket>

class QWebSocket;
class QJsonObject;

class AWebSocketSession : public QObject
{
    Q_OBJECT

public:
    enum  ServerState {Idle = 0, Connecting, ConnectionFailed, Connected, Aborted};

    AWebSocketSession(QObject * parent = nullptr);
    ~AWebSocketSession();

    bool  connect(const QString& Url, bool WaitForAnswer = true);
    void  disconnect();
    void  externalAbort();
    int   ping();

    bool  sendText(const QString & message, bool bWaitReply = true);
    bool  sendJsonObject(const QJsonObject & json);
    bool  sendFile(const QString & fileName, const QString & remoteFileName);

    bool  requestFile(const QString & RemoteFileName, const QString & SaveAs);

    bool  resumeWaitForAnswer();

    void  clearReply();

    const QString    & getError() const {return Error;}
    const QString    & getTextReply() const {return TextReply;}
    const QByteArray & getBinaryReply() const {return BinaryReply;}
    bool               isBinaryReplyEmpty() const {return BinaryReply.isEmpty();}

    void setTimeout(int timeout) {Timeout = timeout;}

public slots:
    void  onSendMessageRequest(QString txt);

private slots:
    void  onConnect();
    void  onDisconnect();
    void  onTextMessageReceived(const QString& message);
    void  onBinaryMessageReceived(const QByteArray &message);

    //void  onStateChanged(QAbstractSocket::SocketState state);

signals:
    void remoteWorkFinished(QString message);
    void progressReceived(int eventsDone);

private:
    QWebSocket * Socket        = nullptr;
    int          Timeout       = 3000;  // in ms
    int          TimeoutDisc   = 3000;  // in ms
    ulong        SleepDuration = 50;    // in ms

    ServerState  State         = Idle;
    QString      Error;
    quint16      PeerPort = 0;

    bool         bWaitForAnswer = false;
    int          TimeMs         = 0;
    bool         bExternalAbort = false;

    QString      TextReply;
    QByteArray   BinaryReply;

private:    
    bool confirmSendPossible();
    bool waitForReply();
};

#endif // AWEBSOCKETSESSION_H
