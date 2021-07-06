#include "a3dispinterface.h"
#include "a3processhandler.h"
#include "a3global.h"
#include "a3workdistrconfig.h"
#include "ajsontools.h"

#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>

#include <cmath>

A3DispInterface::A3DispInterface(QObject *parent) :
    QObject(parent)
{
    connect(this, &A3DispInterface::sendMessage, this, &A3DispInterface::onSendMessage, Qt::QueuedConnection);
}

void A3DispInterface::stop()
{
    qDebug() << "AboutToExit for DispInterface";
    emit sendMessage("$$EXIT\n");
    if (Handler) Handler->doExit(); //calls deleteLater on Handler!
    Handler = nullptr;
}

A3DispInterface::~A3DispInterface()
{
    qDebug() << "Destr for DispInterface";
}

QString A3DispInterface::prepareRunPlan(std::vector<A3FarmNodeRecord> &runPlan, int numEvents, int overrideLocalCores)
{
    runPlan.clear();

    A3Global & GlobSet = A3Global::getInstance();
    int numLocals = (overrideLocalCores == -1 ? GlobSet.LocalCores : overrideLocalCores);

    QVector<A3FarmNodeRecord> tmpPlan;
    int    num      = 0;
    double totSpeed = 0;

    if (numLocals > 0)
    {
        tmpPlan << A3FarmNodeRecord("", 0, numLocals);
        num      += numLocals;
        totSpeed += 1.0 * numLocals;
    }

    std::vector<A3FarmNodeRecord> & FarmNodes = GlobSet.FarmNodes;
    for (const A3FarmNodeRecord & r : FarmNodes)
        if (r.Enabled)
        {
            tmpPlan << r;
            num      += r.Cores;
            totSpeed += r.SpeedFactor * r.Cores;
        }
    if (num      == 0) return "There are no cores to run";
    if (totSpeed == 0) return "Total allocated speed factor is zero";

    //dividing work
    double eventsPerUnitSpeed = numEvents / totSpeed;
    int remainingEvents = numEvents;
    double lastDelta = 0;
    for (A3FarmNodeRecord & r : tmpPlan)
    {
        if (remainingEvents == 0) break;

        r.Split = std::vector<int>(r.Cores, 0);
        double perCore = r.SpeedFactor * eventsPerUnitSpeed;
        for (int & num : r.Split)
        {
            double toDo = perCore + lastDelta;
            num = std::round(toDo);
            lastDelta = num - toDo;

            if (num > remainingEvents) num = remainingEvents;
            remainingEvents -= num;
            if (remainingEvents == 0) break;
        }
        runPlan.push_back(r);
    }

    if (remainingEvents != 0)
    {
        qDebug() << "! Undistributed events:"<< remainingEvents << "-- assuming it can be only one";
        runPlan.front().Split.front() += remainingEvents;
    }

    return "";
}

QString A3DispInterface::performTask(const A3WorkDistrConfig &Request)
{
    Reply.clear();
    NumEvents = Request.NumEvents;

    QJsonObject rjs;
    Request.writeToJson(rjs);
    QString message = jstools::jsonToString(rjs);
    qDebug() << "Sending request to dispatcher:\n" << message;
    Handler->sendMessage(message); //cannot send message directly (different threads!)

    qDebug() << "Waiting for reply from dispatcher...";
    waitForReply();

    qDebug() << "...work completed, dispatcher reply:\n" << Reply;
    return Reply;
}

QString A3DispInterface::waitForReply()
{
    // TODO: filter reply! need "status: finished" or error

    while (Reply.isEmpty())
    {
        QThread::usleep(100);
        qApp->processEvents();
    }
    return Reply;
}

void A3DispInterface::start()
{
    A3Global & GlobSet = A3Global::getInstance();
    Handler = new A3ProcessHandler(GlobSet.ExecutableDir + '/' + GlobSet.DispatcherExecutable, {"0"}); // 0 -> WebSocket server not started
    connect(Handler, &A3ProcessHandler::receivedMessage, this,    &A3DispInterface::receivedMessage);
    connect(Handler, &A3ProcessHandler::updateProgress,  this,    &A3DispInterface::onProgressReceived);
    Handler->start();
}

void A3DispInterface::onSendMessage(QString text)
{
    //qDebug() << Handler << "Sending message to dispatcher:\n" << text;
    Reply.clear();
    if (Handler) Handler->sendMessage(text);
}

void A3DispInterface::receivedMessage(QString text)
{
    qDebug() << "Received message from dispatcher:\n" << text;
    Reply = text;
}

void A3DispInterface::onProgressReceived(double progress)
{
    double val = (NumEvents == 0 ? 0 : progress / NumEvents);
    emit updateProgress(val); // to GUI if present
}
