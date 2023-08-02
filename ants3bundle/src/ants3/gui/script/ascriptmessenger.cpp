#include "ascriptmessenger.h"
#include "astopwatch.h"
#include "ascripthub.h"

#include <QTimer>

#include <chrono>

AScriptMessenger::AScriptMessenger(EScriptLanguage language, bool html, QObject * parent) :
    QObject{parent},
    Language(language), HTML(html),
    StopWatch(new AStopWatch), Timer(new QTimer())
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
            AScriptHub::getInstance().outputText(txt, Language);
            return;
        }
        // else starting new "queue"

        // locking the buffer
        {
            QMutexLocker locker(&BufferMutex);
            if (!Buffer.isEmpty()) Buffer += '\n';
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

void AScriptMessenger::flush()
{
    Timer->stop();
    QMutexLocker locker(&BufferMutex);
    AScriptHub::getInstance().outputText(Buffer, Language);
    Buffer.clear();
}

void AScriptMessenger::clear()
{
    Timer->stop();
    QMutexLocker locker(&BufferMutex);
    Buffer.clear();
}

void AScriptMessenger::onTimer()
{
    // locking the buffer
    {
        QMutexLocker locker(&BufferMutex);
        AScriptHub::getInstance().outputText(Buffer, Language);
        Buffer.clear();
    }

    StopWatch->start();
}
