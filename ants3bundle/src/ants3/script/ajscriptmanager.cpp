#include "ajscriptmanager.h"
#include "ajscriptworker.h"
#include "ascriptinterface.h"

#include <QThread>
#include <QDebug>

AJScriptManager::AJScriptManager(QObject *parent) :
    AVirtualScriptManager(parent)
{
    //qDebug() << "Creating script manager" << QThread::currentThreadId();
    start();
}

AJScriptManager::~AJScriptManager()
{
    qDebug() << "Destr for JScriptManager";
    Worker->abort();
    Worker->exit();
    Thread->exit();
    delete Worker;
    delete Thread;
}

void AJScriptManager::registerInterface(AScriptInterface * interface, QString name)
{
    interface->Lang = EScriptLanguage::JavaScript;
    emit doRegisterInterface(interface, name);
}

const std::vector<AScriptInterface*> & AJScriptManager::getInterfaces() const
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
//    connect(Worker, &AJScriptWorker::stopped,     Worker, &AJScriptWorker::deleteLater);
    connect(Thread, &QThread::finished,           Thread, &QThread::deleteLater);

    connect(this, &AJScriptManager::doRegisterInterface, Worker, &AJScriptWorker::onRegisterInterface);

    Thread->start();
}

void AJScriptManager::evalFinished(bool flag)
{
    bFinished = true;
    emit finished(flag);
}

bool AJScriptManager::evaluate(const QString &script)
{
    //qDebug() << "Request to evaluate script:\n" << script;
    if (Worker->isBusy()) return false;

    bAborted = false;
    bFinished = false;

    emit doEval(script);
    return true;
}

bool AJScriptManager::isRunning() const
{
    return Worker->isBusy();
}

void AJScriptManager::abort() // to abort script use AJScriptHub::abort(message)
{
    bAborted = true;
    Worker->abort();
}

QVariant AJScriptManager::getResult()
{
    return Worker->getResult().toVariant();
}

bool AJScriptManager::isError() const
{
    return Worker->isError();
}

int AJScriptManager::getErrorLineNumber()
{
    return Worker->getErrorLineNumber();
}

bool AJScriptManager::testMinimizationFunction()
{
    return Worker->isCallable(MiniFunctionName);
}

double AJScriptManager::runMinimizationFunction(const double * p)
{
    return Worker->runMinimizationFunction(MiniFunctionName, p, MiniNumVariables); // !!!*** store function as JSValue
}

bool AJScriptManager::isCallable(const QString & functionName)
{
    return Worker->isCallable(functionName);
}

bool AJScriptManager::callFunctionNoArguments(const QString & functionName)
{
    return Worker->callFunctionNoArguments(functionName);
}

QString AJScriptManager::getErrorDescription() const
{
    return Worker->getErrorDescription();
}

void AJScriptManager::collectGarbage()
{
    Worker->collectGarbage();
}
