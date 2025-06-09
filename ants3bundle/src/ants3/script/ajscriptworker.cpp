#include "ajscriptworker.h"
#include "ascriptinterface.h"

#include <QJSEngine>
#include <QDebug>

AJScriptWorker::~AJScriptWorker()
{
    //qDebug() << "Destr for JavaScriptWorker";
    delete Engine;

    // do not delete script interfaces, it is automatic!
}

void AJScriptWorker::onRegisterInterface(AScriptInterface * interface, QString name)
{
    interface->Name = name;
    QJSValue sv = Engine->newQObject(interface);
    Engine->globalObject().setProperty(name, sv);
    Interfaces.push_back(interface);

    if (name == "core")
    {
        QString str = "function print()\n"
                      "{\n"
                      "   var str = ''\n"
                      "   for (var ii = 0; ii < arguments.length; ii++)\n"
                      "     str += core.toStr(arguments[ii]) + ' '\n"
                      "   core.print(str)\n"
                      "}";
        Engine->evaluate(str);

        str = "function printHtml()\n"
                      "{\n"
                      "   var str = ''\n"
                      "   for (var ii = 0; ii < arguments.length; ii++)\n"
                      "     str += core.toStr(arguments[ii]) + ' '\n"
                      "   core.printHtml(str)\n"
                      "}";
        Engine->evaluate(str);
    }
}

void AJScriptWorker::abort()
{
    for (AScriptInterface * inter : Interfaces) inter->abortRun();

    Engine->setInterrupted(true);
}

bool AJScriptWorker::isError() const
{
    return Result.isError();
}

QString AJScriptWorker::getErrorDescription() const
{
    if (bBusy) return "";

    bool bError = Result.isError();
    if (!bError) return "";

    return Result.property("message").toString();
}

int AJScriptWorker::getErrorLineNumber()
{
    if (bBusy) return -1;

    bool bError = Result.isError();
    if (!bError) return -1;

    return Result.property("lineNumber").toInt();
}

bool AJScriptWorker::isCallable(const QString & name) const
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

bool AJScriptWorker::callFunctionNoArguments(const QString & functionName)
{
    QJSValue prop = Engine->globalObject().property(functionName);
    return !prop.call().isError();
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

    //qDebug() << "Script eval result:\n" << Result.toString();

    for (AScriptInterface * inter : Interfaces) inter->afterRun(); // !!!*** error control!

    bool ok = !Result.isError();

    bBusy = false;
    emit evalFinished(ok);
}

void AJScriptWorker::exit()
{
    //qDebug() << "Exit triggered for JS worker!";

    Engine->setInterrupted(true);
    do {} while (!Engine->isInterrupted());

    emit stopped();
}

void AJScriptWorker::onRequestGarbageCollection()
{
    if (bBusy) return;

    bBusy = true;
    Engine->collectGarbage();
    bBusy = false;
}
