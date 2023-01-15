#ifndef ADISPATCHERINTERFACE_H
#define ADISPATCHERINTERFACE_H

#include "afarmnoderecord.h"

#include <QObject>
#include <QJsonObject>
#include <QString>

#include <vector>
#include <mutex>

class A3Dispatcher;
class A3WorkDistrConfig;

class ADispatcherInterface : public QObject
{
    Q_OBJECT

public:
    static ADispatcherInterface & getInstance();

private:
    ADispatcherInterface();
    ~ADispatcherInterface();

    ADispatcherInterface(const ADispatcherInterface&)            = delete;
    ADispatcherInterface(ADispatcherInterface&&)                 = delete;
    ADispatcherInterface& operator=(const ADispatcherInterface&) = delete;
    ADispatcherInterface& operator=(ADispatcherInterface&&)      = delete;

public:
    QString     fillRunPlan(std::vector<AFarmNodeRecord> & runPlan, int numEvents, int overrideLocalCores = -1); //returns error, otherwise ""

    QJsonObject performTask(const A3WorkDistrConfig & Request);
    void        waitForReply();

    void        abortTask();
    bool        isAborted() const;

public slots:
    void        aboutToQuit();

private slots:
    void        onProgressReceived(double progress);
    void        onWorkFinsihed(QJsonObject result);

signals:
    void        sendCommand(QJsonObject command);
    void        updateProgress(double progress);

protected:
    A3Dispatcher * Dispatcher = nullptr;

    int            NumEvents;
    QJsonObject    Reply;
    std::mutex     ReplyMutex;

    bool           bAbortRequested = false;

private:
    void clearOutputFiles(const A3WorkDistrConfig & Request);
};

#endif // ADISPATCHERINTERFACE_H
