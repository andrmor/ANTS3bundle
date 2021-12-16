#ifndef A3PROCESSHANDLER_H
#define A3PROCESSHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QProcess;

class A3WorkerHandler : public QObject
{
    Q_OBJECT
public:
    virtual bool start() = 0;
    virtual void abort() = 0;
    virtual bool isRunning() = 0;
    virtual void sendMessage(QString txt) = 0; // !!!*** no need anymore!

signals:
    void receivedMessage(QString text);  // !!!*** no need anymore!

public:
    QString ErrorString;
    double  EventsDone = 0;
};

class A3ProcessHandler : public A3WorkerHandler
{
    Q_OBJECT

public:
    A3ProcessHandler(const QString & program, const QStringList & args);
    ~A3ProcessHandler();

    bool start() override;
    void abort() override;
    bool isRunning() override;
    void sendMessage(QString txt) override;

public slots:
    void doExit();

private slots:
    void onReadReady();

signals:
    void updateProgress(double value);

private:
    QString       Program;
    QStringList   Args;
    QProcess    * Process = nullptr;
    //QString output;

protected:
    void killProcess();

};

#endif // A3PROCESSHANDLER_H
