#include "ajscriptmanager.h"
#include "ajscriptworker.h"
#include "ascriptinterface.h"

#include <QThread>
#include <QDebug>

AJScriptManager::AJScriptManager(QObject *parent) :
    QObject(parent)
{
    //qDebug() << "Creating script manager" << QThread::currentThreadId();
    start();
}

AJScriptManager::~AJScriptManager()
{
    qDebug() << "Destr for ScriptManager";
    Worker->abort();
    //emit doExit();
    Worker->exit();
    Thread->exit();
    delete Worker;
    delete Thread;
}

#include <QTimer>
void AJScriptManager::registerInterface(AScriptInterface * interface, QString name)
{
    emit doRegisterInterface(interface, name);
}

const std::vector<AScriptInterface *> &AJScriptManager::getInterfaces() const
{
    return Worker->getInterfaces();
}

void AJScriptManager::start()
{
    //qDebug() << "Starting script worlker"<< QThread::currentThreadId();
    Thread = new QThread();
    Worker = new AJScriptWorker();
    Worker->moveToThread(Thread);

    connect(Thread, &QThread::started,             Worker, &AJScriptWorker::initialize);
    connect(this,   &AJScriptManager::doEval,      Worker, &AJScriptWorker::evaluate);
    connect(this,   &AJScriptManager::doExit,      Worker, &AJScriptWorker::exit);
    connect(Worker, &AJScriptWorker::evalFinished, this,   &AJScriptManager::evalFinished);

    connect(Worker, &AJScriptWorker::stopped,     Thread, &QThread::quit);
    connect(Worker, &AJScriptWorker::stopped,     Worker, &AJScriptWorker::deleteLater);
    connect(Thread, &QThread::finished,           Thread, &QThread::deleteLater);

    connect(this, &AJScriptManager::doRegisterInterface, Worker, &AJScriptWorker::onRegisterInterface);

    Thread->start();
}

void AJScriptManager::evalFinished(bool flag)
{
    emit finished(flag);
}

bool AJScriptManager::evaluate(const QString &script)
{
    qDebug() << "Request to evaluate script:\n" << script;
    if (Worker->isBusy()) return false;

    emit doEval(script);
    return true;
}

bool AJScriptManager::isRunning() const
{
    return Worker->isBusy();
}

void AJScriptManager::abort()
{
    Worker->abort();
}

QJSValue AJScriptManager::getResult()
{
    return Worker->getResult();
}

int AJScriptManager::getErrorLineNumber()
{
    return Worker->getErrorLineNumber();
}

bool AJScriptManager::getError(QString & errorString, int & lineNumber, QString & errorFileName)
{
    return Worker->getError(errorString, lineNumber, errorFileName);
}
