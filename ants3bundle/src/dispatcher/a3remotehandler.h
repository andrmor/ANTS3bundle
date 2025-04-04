#ifndef A3REMOTEHANDLER_H
#define A3REMOTEHANDLER_H

#include "a3processhandler.h"
#include "a3workdistrconfig.h"

#include <QString>

#include <vector>

class A3WSClient;
class QThread;

class A3RemoteHandler : public A3WorkerHandler
{
    Q_OBJECT

public:
    A3RemoteHandler(const A3WorkNodeConfig & Node, const QString & Command, const QString & ExchangeDir, const std::vector<QString> & CommonFiles);

    bool start() override;
    void abort() override;
    bool isRunning() override;
    void sendMessage(QString txt) override; // not implemented

private slots:
    void onRemoteWorkFinished(QString message);
    void onProgressReceived(int eventsDone);

signals:
    void sendTextRequest(QString txt);

protected:
    A3WorkNodeConfig     Node;
    QString              Command;
    QString              ExchangeDir;
    std::vector<QString> CommonFiles;

    QThread            * Thread = nullptr;
    A3WSClient         * Client = nullptr;

    bool                 bRunning = true;

public:
    QString              Reply;
};

#endif // A3REMOTEHANDLER_H
