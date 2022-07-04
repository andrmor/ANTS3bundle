#include "ajscriptworker.h"
#include "ascriptinterface.h"

#include <QJSEngine>
#include <QDebug>

AJScriptWorker::~AJScriptWorker()
{
    qDebug() << "Destr for ScriptWorker";
    delete Engine;

    // do not delete script interfaces, it is automatic!
}

void AJScriptWorker::onRegisterInterface(AScriptInterface * interface, QString name)
{
    QJSValue sv = Engine->newQObject(interface);
    Engine->globalObject().setProperty(name, sv);
    interface->Name = name;
    Interfaces.push_back(interface);
}

void AJScriptWorker::abort()
{
    for (AScriptInterface * inter : Interfaces) inter->abortRun();

    Engine->setInterrupted(true);
}

void AJScriptWorker::collectGarbage()
{
    Engine->collectGarbage();
}

bool AJScriptWorker::isError() const
{
    return Result.isError();
}

bool AJScriptWorker::getError(QString & errorString, int & lineNumber, QString & errorFileName)
{
    if (bBusy) return false;

    bool bError = Result.isError();
    if (!bError) return false;

    errorString   = Result.property("message").toString();
    lineNumber    = Result.property("lineNumber").toInt();
    errorFileName = Result.property("fileName").toString();

    return true;
}

int AJScriptWorker::getErrorLineNumber()
{
    if (bBusy) return -1;

    bool bError = Result.isError();
    if (!bError) return -1;

    return Result.property("lineNumber").toInt();
}

bool AJScriptWorker::testMinimizationFunction(const QString & name) const
{
    QJSValue prop = Engine->globalObject().property(name);
    return prop.isCallable();
}

double AJScriptWorker::runMinimizationFunction(const QString & name, const double * p, int numParameters)
{
    QJSValue prop = Engine->globalObject().property(name);

    QJSValueList input;
    for (int i=0; i<numParameters; i++) input << p[i];

    return prop.call(input).toNumber();
}

void AJScriptWorker::initialize()
{
    Engine = new QJSEngine();
}

void AJScriptWorker::evaluate(const QString & script)
{
    if (bBusy) return;

    for (AScriptInterface * inter : Interfaces) inter->beforeRun(); // !!!*** error control!

    bBusy = true;
    Engine->setInterrupted(false);
    Result = Engine->evaluate(script);
    bBusy = false;

    qDebug() << "Script eval result:\n" << Result.toString();

    for (AScriptInterface * inter : Interfaces) inter->afterRun(); // !!!*** error control!

    bool ok = !Result.isError();
    emit evalFinished(ok);
}

void AJScriptWorker::exit()
{
    Engine->setInterrupted(true);
    do {} while (!Engine->isInterrupted());

//    emit stopped();
}
