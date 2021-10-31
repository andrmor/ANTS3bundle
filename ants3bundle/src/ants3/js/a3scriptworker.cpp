#include "a3scriptworker.h"
#include "ascriptinterface.h"

#include <QJSEngine>
#include <QDebug>

A3ScriptWorker::~A3ScriptWorker()
{
    qDebug() << "Destr for ScriptWorker";
    delete Engine;

    // do not delete script interfaces, it is automatic!
}

void A3ScriptWorker::onRegisterInterface(AScriptInterface * interface, QString name)
{
    QJSValue sv = Engine->newQObject(interface);
    Engine->globalObject().setProperty(name, sv);
    interface->Name = name;
    Interfaces.push_back(interface);
}

void A3ScriptWorker::abort()
{
    Engine->setInterrupted(true);

    // interrupt all script interfaces!
}

bool A3ScriptWorker::getError(QString & errorString, int & lineNumber, QString & errorFileName)
{
    if (bBusy) return false;

    bool bError = Result.isError();
    if (!bError) return false;

    errorString   = Result.property("message").toString();
    lineNumber    = Result.property("lineNumber").toInt();
    errorFileName = Result.property("fileName").toString();

    return true;
}

int A3ScriptWorker::getErrorLineNumber()
{
    if (bBusy) return -1;

    bool bError = Result.isError();
    if (!bError) return -1;

    return Result.property("lineNumber").toInt();
}

void A3ScriptWorker::initialize()
{
    Engine = new QJSEngine();
}

void A3ScriptWorker::evaluate(const QString & script)
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

void A3ScriptWorker::exit()
{
    Engine->setInterrupted(true);
    do {} while (!Engine->isInterrupted());

//    emit stopped();
}
