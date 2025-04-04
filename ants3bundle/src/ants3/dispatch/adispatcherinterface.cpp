#include "adispatcherinterface.h"
#include "a3dispatcher.h"
#include "a3workdistrconfig.h"
#include "afarmhub.h"

#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <cmath>

ADispatcherInterface & ADispatcherInterface::getInstance()
{
    static ADispatcherInterface instance;
    return instance;
}

ADispatcherInterface::ADispatcherInterface() : QObject(nullptr)
{
    Dispatcher = new A3Dispatcher("127.0.0.1", 0, 4);
    connect(this,       &ADispatcherInterface::sendCommand, Dispatcher, &A3Dispatcher::executeLocalCommand,        Qt::QueuedConnection);
    connect(Dispatcher, &A3Dispatcher::workFinished,        this,       &ADispatcherInterface::onWorkFinsihed,     Qt::QueuedConnection);
    connect(Dispatcher, &A3Dispatcher::reportProgress,      this,       &ADispatcherInterface::onProgressReceived, Qt::QueuedConnection);
}

void ADispatcherInterface::aboutToQuit()
{
    qDebug() << "AboutToExit for DispInterface";
    //emit sendMessage("$$EXIT\n");
    Dispatcher->deleteLater();
    Dispatcher = nullptr;
}

ADispatcherInterface::~ADispatcherInterface()
{
    qDebug() << "Destr for DispInterface";
}

QString ADispatcherInterface::fillRunPlan(std::vector<AFarmNodeRecord> & runPlan, int numEvents, int overrideLocalProcesses)
{
    runPlan.clear();

    int numLocalsProcesses = overrideLocalProcesses;
    const AFarmHub & FarmHub = AFarmHub::getConstInstance();
    if (overrideLocalProcesses < 0)
    {
        numLocalsProcesses = 0;
        const AFarmHub & FarmHub = AFarmHub::getConstInstance();
        if (FarmHub.UseLocal)
            numLocalsProcesses = FarmHub.LocalProcesses;
    }

    std::vector<AFarmNodeRecord> tmpPlan;
    int    num      = 0;
    double totSpeed = 0;

    if (numLocalsProcesses > 0)
    {
        tmpPlan.push_back( AFarmNodeRecord("", 0, numLocalsProcesses) );
        num      += numLocalsProcesses;
        totSpeed += 1.0 * numLocalsProcesses;
    }

    if (FarmHub.UseFarm)
    {
        const std::vector<AFarmNodeRecord*> & FarmNodes = FarmHub.getNodes();
        for (const AFarmNodeRecord * FarmNode : FarmNodes)
            if (FarmNode->Enabled)
            {
                tmpPlan.push_back( *FarmNode );
                num      += FarmNode->Processes;
                totSpeed += FarmNode->SpeedFactor * FarmNode->Processes;
            }
    }

    if (num      == 0) return "No local/remote processes were allocated";
    if (totSpeed == 0) return "Total allocated speed factor is zero";

    //dividing work
    double eventsPerUnitSpeed = numEvents / totSpeed;
    int remainingEvents = numEvents;
    double lastDelta = 0;
    for (AFarmNodeRecord & r : tmpPlan)
    {
        if (remainingEvents == 0) break;

        r.Split = std::vector<int>(r.Processes, 0);
        double perCore = r.SpeedFactor * eventsPerUnitSpeed;
        for (int & num : r.Split)
        {
            double toDo = perCore + lastDelta;
            num = std::ceil(toDo);

            if (num <= 0) num = 1;
            if (num > remainingEvents) num = remainingEvents;

            lastDelta = toDo - num;

            remainingEvents -= num;
            if (remainingEvents == 0) break;
        }
        runPlan.push_back(r);
    }

    if (remainingEvents != 0)
    {
        qDebug() << "Undistributed events:"<< remainingEvents << "-- assuming it can be only one!";
        runPlan.front().Split.front() += remainingEvents;
    }

    // debug
    qDebug() << "Obtained run plan over local/farm nodes:";
    int iNode = 0;
    for (AFarmNodeRecord & r : runPlan)
        qDebug() << iNode++ << ">--->" << (r.Address.isEmpty() ? "Local" : r.Address) << " Number of events:"<< r.Split;

    return "";
}

QJsonObject ADispatcherInterface::performTask(const A3WorkDistrConfig & Request)
{
    clearOutputFiles(Request);

    bAbortRequested = false;
    Reply = QJsonObject();
    NumEvents = Request.NumEvents;

    QJsonObject rjs;
    Request.writeToJson(rjs);
    emit sendCommand(rjs); //cannot send message directly (different threads if called from script!)

    qDebug() << "Waiting for reply from dispatcher...";
    waitForReply();

    qDebug() << "...work completed, dispatcher reply:\n" << Reply;
    return Reply;
}

void ADispatcherInterface::waitForReply()
{
    // !!!*** TODO: filter reply! error?

    while (Reply.isEmpty())
    {
        QThread::msleep(50);
        qApp->processEvents();
    }
    return;
}

void ADispatcherInterface::abortTask()
{
    bAbortRequested = true;
    Dispatcher->abortExecution();
}

bool ADispatcherInterface::isAborted() const
{
    return bAbortRequested;
}

void ADispatcherInterface::onProgressReceived(double progress)
{
    double val = (NumEvents == 0 ? 0 : progress / NumEvents);
    emit updateProgress(val); // to GUI if present
}

void ADispatcherInterface::onWorkFinsihed(QJsonObject result)
{
    const std::lock_guard<std::mutex> lock(ReplyMutex);
    Reply = result;
}

void ADispatcherInterface::clearOutputFiles(const A3WorkDistrConfig & Request)
{
    for (const A3WorkNodeConfig & node : Request.Nodes)
        for (const A3NodeWorkerConfig & wc : node.Workers)
            for (const QString & fn : wc.OutputFiles)
                QFile::remove(Request.ExchangeDir + '/' + fn);
}
