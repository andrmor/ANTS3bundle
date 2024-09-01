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

bool APythonScriptManager::isCallable(const QString & name) const
{
    return Worker->isCallable(name);
}

bool APythonScriptManager::callFunctionNoArguments(const QString & name)
{
    return Worker->callFunctionNoArguments(name);
}

QString APythonScriptManager::getVersion()
{
    int ver = PY_VERSION_HEX;

// https://docs.python.org/3/c-api/apiabiversion.html
// Bytes   Bits (big endian order) Meaning             Value for 3.4.1a2
// 1       1-8                     PY_MAJOR_VERSION    0x03
// 2       9-16                    PY_MINOR_VERSION    0x04
// 3       17-24                   PY_MICRO_VERSION    0x01
// 4       25-28                   PY_RELEASE_LEVEL    0xA
//         29-32                   PY_RELEASE_SERIAL   0x2

    int major = ver / 0x1000000;
    int minor = (ver - major*0x1000000) / 0x10000;
    int extra = ver - major*0x1000000 - minor*0x10000;

    //return QString::number(ver, 16);
    return QString::number(major) + "." + QString::number(minor) + "." + QString::number(extra, 16);
}

void APythonScriptManager::checkSignals()
{
    Worker->checkSignals();
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
    //qDebug() << "Busy?" << Worker->isBusy();
    if (Worker->isBusy()) return false;

    bAborted = false;

    emit doEval(script);

    return true;
}

void APythonScriptManager::abort()
{
    Worker->abort();
    bAborted = true;
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
