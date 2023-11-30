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

    LineBreak = (HTML ? "<br>" : "\n");
}

void AScriptMessenger::output(QString txt)
{
    if (Buffer.isEmpty())
    {
        if (StopWatch->getSecondsFromStart() > IntervalForDirectOutput_seconds)
        {
            (HTML ? AScriptHub::getInstance().outputHtml(txt, Language) : AScriptHub::getInstance().outputText(txt, Language));
            StopWatch->start();
            return;
        }
        // else starting new "queue"

        // locking the buffer and store txt in the buffer
        {
            QMutexLocker locker(&BufferMutex);
            if (!Buffer.isEmpty()) Buffer += LineBreak;
            Buffer += txt;
        }
        Timer->start();
    }
    else
    {
        // no need to pause timer?
        QMutexLocker locker(&BufferMutex);
        Buffer += LineBreak;
        Buffer += txt;
    }
}

void AScriptMessenger::flush()
{
    Timer->stop();
    QMutexLocker locker(&BufferMutex);
    (HTML ? AScriptHub::getInstance().outputHtml(Buffer, Language) : AScriptHub::getInstance().outputText(Buffer, Language));
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
    // locking the buffer to send the message
    {
        QMutexLocker locker(&BufferMutex);
        (HTML ? AScriptHub::getInstance().outputHtml(Buffer, Language) : AScriptHub::getInstance().outputText(Buffer, Language));
        Buffer.clear();
    }

    StopWatch->start();
}
