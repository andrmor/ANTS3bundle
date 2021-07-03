#ifndef A3DISPATCHER_H
#define A3DISPATCHER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

#include <vector>

class QSocketNotifier;
class AWebSocketSessionServer;
class QFile;
class QTextStream;
class QTimer;
class A3WorkerHandler;
class A3WorkNodeConfig;
class A3WorkDistrConfig;

class A3Dispatcher : public QObject
{
    Q_OBJECT

public:
    A3Dispatcher(quint16 port, QObject * parent = nullptr); // port = 0 -> persistent web socket server not started
    ~A3Dispatcher();

public slots:
    void start();

private slots:
    void onLocalCommandReceived();
    void onRemoteCommandReceived(QJsonObject json);

    void onReportProgressTimer();

protected:
    quint16 PortPersistentWS = 0;

    QSocketNotifier         * Notifier        = nullptr;
    AWebSocketSessionServer * WebSocketServer = nullptr;

    QFile       * OutFile             = nullptr;
    QTextStream * LogStream           = nullptr;
    QTimer      * ProgressReportTimer = nullptr;

    std::vector<A3WorkerHandler*> Handlers;

    QString StandaloneDir;

    void    localReplyFinished();
    void    localReplyError(const QString & ErrorMessage);
    void    log(const QString & text);
    void    processLocalCommand(const QString & message);
    bool    startLocalWork(const QString & executable, const QString & exchangeDir, const A3WorkNodeConfig & localNode);
    QString startRemoteWork(const A3WorkDistrConfig & wdc); //returns error, otherwise ""
    void    waitForWorkFinished();
    void    clearHandlers();
};

#endif // A3DISPATCHER_H
