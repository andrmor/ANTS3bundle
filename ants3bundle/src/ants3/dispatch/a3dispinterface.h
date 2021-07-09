#ifndef A3DISPINTERFACE_H
#define A3DISPINTERFACE_H

#include "a3farmnoderecord.h"

#include <QObject>
#include <QJsonObject>
#include <QString>

#include <vector>
#include <mutex>

class A3Dispatcher;
class A3WorkDistrConfig;

class A3DispInterface : public QObject
{
    Q_OBJECT

public:
    A3DispInterface(QObject * parent = nullptr);
    ~A3DispInterface();

    QString prepareRunPlan(std::vector<A3FarmNodeRecord> & runPlan, int numEvents, int overrideLocalCores = -1); //returns error, otherwise ""

    QString performTask(const A3WorkDistrConfig & Request);
    void waitForReply();

public slots:
    void stop();

private slots:
    void onProgressReceived(double progress);
    void onWorkFinsihed(QJsonObject result);

signals:
    void sendCommand(QJsonObject text);
    void updateProgress(double progress);

protected:
    A3Dispatcher * Dispatcher = nullptr;

    int NumEvents;

    QJsonObject Reply;

    std::mutex ReplyMutex;
};

#endif // A3DISPINTERFACE_H
