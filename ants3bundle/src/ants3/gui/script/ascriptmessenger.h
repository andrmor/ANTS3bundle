#ifndef ASCRIPTMESSENGER_H
#define ASCRIPTMESSENGER_H

#include "escriptlanguage.h"

#include <QObject>
#include <QString>
#include <QMutex>

class AStopWatch;
class QTimer;

class AScriptMessenger : public QObject
{
    Q_OBJECT
public:
    AScriptMessenger(EScriptLanguage language, QObject * parent = nullptr);

    void output(QString txt, bool html);

    void flush();
    void clear();

private slots:
    void onTimer();

private:
    EScriptLanguage Language;

    AStopWatch    * StopWatch = nullptr;
    QTimer        * Timer = nullptr;

    std::vector<std::pair<bool,QString>> Buffer;

    const int       TimerInterval_Milliseconds      = 1;
    const double    IntervalForDirectOutput_seconds = 0.001;

    mutable QMutex  BufferMutex;
};

#endif // ASCRIPTMESSENGER_H
