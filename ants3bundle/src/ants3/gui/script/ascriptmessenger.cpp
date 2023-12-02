#include "ascriptmessenger.h"
#include "astopwatch.h"
#include "ascripthub.h"

#include <QTimer>

#include <chrono>

AScriptMessenger::AScriptMessenger(EScriptLanguage language, QObject * parent) :
    QObject{parent},
    Language(language),
    StopWatch(new AStopWatch), Timer(new QTimer())
{
    connect(Timer, &QTimer::timeout, this, &AScriptMessenger::onTimer);
    Timer->setSingleShot(true);
    Timer->setInterval(TimerInterval_Milliseconds);
    StopWatch->start();
}

void AScriptMessenger::output(QString txt, bool html)
{
    if (Buffer.empty())
    {
        if (StopWatch->getSecondsFromStart() > IntervalForDirectOutput_seconds)
        {
            (html ? AScriptHub::getInstance().outputHtml(txt, Language) : AScriptHub::getInstance().outputText(txt, Language));
            StopWatch->start();  // protext by mutex if expand to multiple instances of the caller
            return;
        }
        // else starting new "queue"

        // locking the buffer and store txt in the buffer
        {
            QMutexLocker locker(&BufferMutex);
            Buffer.push_back({html, txt});
            Timer->start();
        }
    }
    else
    {
        QMutexLocker locker(&BufferMutex);
        Buffer.push_back({html, txt});
    }
}

void AScriptMessenger::flush()
{
    QMutexLocker locker(&BufferMutex);
    Timer->stop();
    AScriptHub::getInstance().outputFromBuffer(Buffer, Language);
    Buffer.clear();
}

void AScriptMessenger::clear()
{
    QMutexLocker locker(&BufferMutex);
    Timer->stop();
    Buffer.clear();
    StopWatch->start();
}

void AScriptMessenger::onTimer()
{
    QMutexLocker locker(&BufferMutex);
    AScriptHub::getInstance().outputFromBuffer(Buffer, Language);
    Buffer.clear();
    StopWatch->start();
}
