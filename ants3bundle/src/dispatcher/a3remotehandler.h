#ifndef A3REMOTEHANDLER_H
#define A3REMOTEHANDLER_H

#include "a3processhandler.h"
#include "a3workdistrconfig.h"

#include <QVector>
#include <QString>

class AWebSocketSession;

class A3RemoteHandler : public A3WorkerHandler
{
    Q_OBJECT

public:
    A3RemoteHandler(const A3WorkNodeConfig & Node, const QString & Command, const QString & ExchangeDir, const QVector<QString> & CommonFiles);

    bool start() override;
    void abort() override;
    bool isRunning() override;
    void sendMessage(QString txt) override;

    bool isRequestingTransfer() override {return bRequestTransfer;}
    void transferOutputFiles() override;

private slots:
    void onRemoteWorkFinished(QString message);
    void onProgressReceived(int eventsDone);

signals:
    void sendTextRequest(QString txt);

protected:
    A3WorkNodeConfig Node;
    QString          Command;
    QString          ExchangeDir;
    QVector<QString> CommonFiles;

    AWebSocketSession * Session = nullptr;

    bool             bRequestTransfer = false;
    bool             bRunning = true;
};

#endif // A3REMOTEHANDLER_H
