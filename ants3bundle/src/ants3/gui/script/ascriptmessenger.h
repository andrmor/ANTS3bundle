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
    AScriptMessenger(EScriptLanguage language, bool html, QObject * parent = nullptr);

    void output(QString txt);

    void flush();

signals:
    void requestOutput(QString txt);

private slots:
    void onTimer();

private:
    EScriptLanguage Language;
    bool            HTML;

    AStopWatch    * StopWatch = nullptr;
    QTimer        * Timer = nullptr;
    QString         Buffer;

    const int       TimerInterval_Milliseconds      = 1;
    const double    IntervalForDirectOutput_seconds = 0.001;

    mutable QMutex  BufferMutex;

};

#endif // ASCRIPTMESSENGER_H
