#ifndef A3DISPINTERFACE_H
#define A3DISPINTERFACE_H

#include "a3farmnoderecord.h"

#include <QObject>
#include <QString>

#include <vector>

class A3ProcessHandler;
class A3WorkDistrConfig;

class A3DispInterface : public QObject
{
    Q_OBJECT

public:
    A3DispInterface(QObject * parent = nullptr);
    ~A3DispInterface();

    QString prepareRunPlan(std::vector<A3FarmNodeRecord> & runPlan, int numEvents, int overrideLocalCores = -1); //returns error, otherwise ""

    QString performTask(const A3WorkDistrConfig & Request);
    QString waitForReply();

public slots:
    void start();
    void onSendMessage(QString text);
    void stop();

private slots:
    void receivedMessage(QString text);
    void onProgressReceived(double progress);

signals:
    void sendMessage(QString text);
    void updateProgress(double progress);
    void exitProcess();

protected:
    A3ProcessHandler * Handler = nullptr;

    int LocalCores = 4;

    int NumEvents;

    QString Reply;
};

#endif // A3DISPINTERFACE_H
