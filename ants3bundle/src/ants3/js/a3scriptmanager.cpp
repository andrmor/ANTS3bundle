#include "a3scriptmanager.h"
#include "a3scriptworker.h"
#include "a3scriptres.h"

#include <QThread>
#include <QDebug>

A3ScriptManager::A3ScriptManager(A3ScriptRes & ScrRes, QObject *parent) :
    QObject(parent), ScrRes(ScrRes)
{
    //qDebug() << "Creating script manager" << QThread::currentThreadId();
    start();
}

A3ScriptManager::~A3ScriptManager()
{
    qDebug() << "Destr for ScriptManager";
    Worker->abort();
    emit doExit();
}

void A3ScriptManager::start()
{
    //qDebug() << "Starting script worlker"<< QThread::currentThreadId();
    Thread = new QThread();
    Worker = new A3ScriptWorker(ScrRes);
    Worker->moveToThread(Thread);

    connect(Thread, &QThread::started,             Worker, &A3ScriptWorker::initialize);
    connect(this,   &A3ScriptManager::doEval,      Worker, &A3ScriptWorker::evaluate);
    connect(this,   &A3ScriptManager::doExit,      Worker, &A3ScriptWorker::exit);
    connect(Worker, &A3ScriptWorker::evalFinished, this,   &A3ScriptManager::evalFinished);

    connect(Worker, &A3ScriptWorker::stopped,     Thread, &QThread::quit);
    connect(Worker, &A3ScriptWorker::stopped,     Worker, &A3ScriptWorker::deleteLater);
    connect(Thread, &QThread::finished,           Thread, &QThread::deleteLater);

    Thread->start();
}

void A3ScriptManager::evalFinished(bool flag)
{
    emit finished(flag);
}

bool A3ScriptManager::evaluate(const QString &script)
{
    qDebug() << "Request to evaluate script:\n" << script;
    if (Worker->isBusy()) return false;

    emit doEval(script);
    return true;
}

bool A3ScriptManager::isRunning() const
{
    return Worker->isBusy();
}

QJSValue A3ScriptManager::getResult()
{
    return Worker->getResult();
}

int A3ScriptManager::getErrorLineNumber()
{
    return Worker->getErrorLineNumber();
}

bool A3ScriptManager::getError(QString & errorString, int & lineNumber, QString & errorFileName)
{
    return Worker->getError(errorString, lineNumber, errorFileName);
}
