#include "ascriptmessenger.h"
#include "astopwatch.h"

#include <QTimer>

#include <chrono>

AScriptMessenger::AScriptMessenger(QObject * parent)
    : QObject{parent}, StopWatch(new AStopWatch), Timer(new QTimer())
{
    connect(Timer, &QTimer::timeout, this, &AScriptMessenger::onTimer);
    Timer->setSingleShot(true);
    Timer->setInterval(TimerInterval_Milliseconds);
}

void AScriptMessenger::output(QString txt)
{
    if (Buffer.isEmpty())
    {
        if (StopWatch->getSecondsFromStart() > IntervalForDirectOutput_seconds)
        {
            emit requestOutput(txt);
            return;
        }
        // else starting new "queue"

        // locking the buffer
        {
            QMutexLocker locker(&BufferMutex);
            Buffer += '\n';
            Buffer += txt;
        }
        Timer->start();
    }
    else
    {
        // no need to pause timer?
        QMutexLocker locker(&BufferMutex);
        Buffer += '\n';
        Buffer += txt;
    }
}

void AScriptMessenger::onTimer()
{
    // locking the buffer
    {
        QMutexLocker locker(&BufferMutex);
        emit requestOutput(Buffer);
        Buffer.clear();
    }

    StopWatch->start();
}
