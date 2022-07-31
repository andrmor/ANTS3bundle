#include "apythoninterface.h" // MUST be first

#include "apythonscriptmanager.h"
#include "ascriptinterface.h"
#include "apythonworker.h"

#include <QThread>
#include <QDebug>

APythonScriptManager::APythonScriptManager(QObject *parent)
    : AVirtualScriptManager{parent}
{
    //PyInterface = new APythonInterface();

    //qDebug() << "Starting script worlker"<< QThread::currentThreadId();
    Thread = new QThread();
    Worker = new APythonWorker();
    Worker->moveToThread(Thread);

    connect(Thread, &QThread::started,             Worker, &APythonWorker::initialize);
    connect(this,   &APythonScriptManager::doEval, Worker, &APythonWorker::evaluate);
    connect(this,   &APythonScriptManager::doExit, Worker, &APythonWorker::exit);
    connect(Worker, &APythonWorker::evalFinished,  this,   &APythonScriptManager::evalFinished);

    connect(Worker, &APythonWorker::stopped,       Thread, &QThread::quit);
    connect(Worker, &APythonWorker::stopped,       Worker, &APythonWorker::deleteLater);
    connect(Thread, &QThread::finished,            Thread, &QThread::deleteLater);

    connect(this, &APythonScriptManager::doRegisterInterface, Worker, &APythonWorker::onRegisterInterface);
    connect(this, &APythonScriptManager::doFinalizeInit,      Worker, &APythonWorker::onFinalizeInit);

    Thread->start();
}

APythonScriptManager::~APythonScriptManager()
{
    // !!!*** to do: crashes on exit i script is running (same with JS)
    qDebug() << "Destr for PythonManager";
    Worker->abort();
    emit doExit();
    //Worker->exit();
    //Thread->exit();
    //delete Worker;
    //delete Thread;
}

void APythonScriptManager::registerInterface(AScriptInterface * interface, QString name)
{
    interface->Lang = EScriptLanguage::Python;
    emit doRegisterInterface(interface, name);
}

void APythonScriptManager::finalizeInit()
{
    emit doFinalizeInit();
}

bool APythonScriptManager::testMinimizationFunction()
{
    return Worker->testMinimizationFunction(MiniFunctionName);
}

double APythonScriptManager::runMinimizationFunction(const double *p)
{
    return Worker->runMinimizationFunction(p, MiniNumVariables);
}

void APythonScriptManager::evalFinished(bool flag)
{
    emit finished(flag);
}

const std::vector<AScriptInterface *> & APythonScriptManager::getInterfaces() const
{
    return Worker->getInterfaces();
}

bool APythonScriptManager::evaluate(const QString & script)
{
    //qDebug() << "Request to evaluate script:\n" << script;
    if (Worker->isBusy()) return false;

    bAborted = false;

    emit doEval(script);

    return true;
}

void APythonScriptManager::abort()
{
    bAborted = true;
    Worker->abort();
}

bool APythonScriptManager::isRunning() const
{
    return Worker->isBusy();
}

QVariant APythonScriptManager::getResult()
{
    return "undefined";
}

bool APythonScriptManager::isError() const
{
    return Worker->isError();
}

QString APythonScriptManager::getErrorDescription() const
{
    return Worker->getErrorDescription();
}

int APythonScriptManager::getErrorLineNumber()
{
    return Worker->getErrorLineNumber();
}
