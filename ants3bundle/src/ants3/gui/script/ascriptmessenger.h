#ifndef ASCRIPTMESSENGER_H
#define ASCRIPTMESSENGER_H

#include <QObject>
#include <QString>
#include <QMutex>

class AStopWatch;
class QTimer;

class AScriptMessenger : public QObject
{
    Q_OBJECT
public:
    explicit AScriptMessenger(QObject * parent = nullptr);

    void output(QString txt);

signals:
    void requestOutput(QString txt);

private slots:
    void onTimer();

private:
    AStopWatch   * StopWatch = nullptr;
    QTimer       * Timer = nullptr;
    QString        Buffer;

    const int      TimerInterval_Milliseconds      = 1;
    const double   IntervalForDirectOutput_seconds = 0.001;

    mutable QMutex BufferMutex;
};

#endif // ASCRIPTMESSENGER_H
