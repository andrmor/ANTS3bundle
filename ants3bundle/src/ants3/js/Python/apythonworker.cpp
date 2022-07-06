#include "apythoninterface.h" // must be first!

#include "apythonworker.h"
#include "ascriptinterface.h"

#include <QDebug>

APythonWorker::APythonWorker(QObject *parent)
    : QObject{parent} {}

APythonWorker::~APythonWorker()
{
    qDebug() << "Destr for PythonWorker";
    delete PyInterface;

    // JS does not need the following step due to QObject paranting, this does:
    for (AScriptInterface * inter : Interfaces)
        delete inter;
}

void APythonWorker::abort()
{
    PyInterface->abort();

    for (AScriptInterface * inter : Interfaces)
        inter->abortRun();
}

bool APythonWorker::isError() const
{
    return !PyInterface->ErrorDescription.isEmpty();
}

QString APythonWorker::getErrorDescription() const
{
    return PyInterface->ErrorDescription;
}

int APythonWorker::getErrorLineNumber() const
{
    return PyInterface->ErrorLineNumber;
}

void APythonWorker::initialize()
{
    PyInterface = new APythonInterface();
}

void APythonWorker::onRegisterInterface(AScriptInterface * interface, QString name)
{
    interface->Name = name;
    PyInterface->registerUnit(interface, name);
    Interfaces.push_back(interface);
}

void APythonWorker::onFinalizeInit()
{
    PyInterface->initialize();
}

void APythonWorker::evaluate(const QString &script)
{
    if (bBusy) return;

    for (AScriptInterface * inter : Interfaces) inter->beforeRun(); // !!!*** error control!

    bBusy = true;
    bool ok = PyInterface->evalScript(script);
    bBusy = false;

    //qDebug() << "Script eval finished:\n" << ok;

    for (AScriptInterface * inter : Interfaces) inter->afterRun(); // !!!*** error control!

    emit evalFinished(ok);
}

void APythonWorker::exit()
{
    // !!!***
}
