#ifndef A3DISPATCHER_H
#define A3DISPATCHER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

#include <vector>

class AWebSocketSessionServer;
class QFile;
class QTextStream;
class QTimer;
class A3WorkerHandler;
class A3WorkNodeConfig;
class A3WorkDistrConfig;
class A3FarmNodeRecord;

class A3Dispatcher : public QObject
{
    Q_OBJECT

public:
    A3Dispatcher(const QString & ip, quint16 port, int maxProcesses, QObject * parent = nullptr); // port = 0 -> WebSocket server not started
    ~A3Dispatcher();

public slots:
    void start();

    void executeLocalCommand(QJsonObject json);
    void abortExecution() {bAbortRequested = true;}

signals:
    void workFinished(QJsonObject result);
    void reportProgress(double eventsDone);

protected:
    QString       IP;
    quint16       WebSocketPort = 0;
    int           MaxNumberProcesses = 4;

    AWebSocketSessionServer * WebSocketServer = nullptr;

    QFile       * LogFile       = nullptr;
    QTextStream * LogStream     = nullptr;
    QTimer      * ProgressTimer = nullptr;

    std::vector<A3WorkerHandler*> Handlers;

    QString StandaloneDir;

    bool    bAbortRequested = false;

    void    localReplyFinished();
    void    localReplyError(const QString & ErrorMessage);
    void    localReplyAborted();
    void    log(const QString & text);
    bool    startWorkHere(const QString & executable, const QString & exchangeDir, const A3WorkNodeConfig & localNode);
    void    waitForWorkFinished();
    void    clearHandlers();
    void    checkFarmStatus(const A3WorkDistrConfig & wdc);

private slots:
    void onReportProgressTimer();

#ifdef WEBSOCKETS
protected:
    QString startWorkFarm(const A3WorkDistrConfig & wdc); //returns error, otherwise ""
private slots:
    void    onRemoteCommandReceived(QJsonObject json);    // only server use
#endif
};

#endif // A3DISPATCHER_H
