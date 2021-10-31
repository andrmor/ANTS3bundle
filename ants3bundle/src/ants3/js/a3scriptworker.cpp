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

#include "ademomanager.h"
#include "afarm_si.h"
#include "aphotonsim_si.h"
void A3ScriptWorker::initialize()
{
    Engine = new QJSEngine();

    /*
    // proper approach is to have a SI -> will be enforced later
    ADemoManager & DemoMan = ADemoManager::getInstance();
    QJSValue sv = Engine->newQObject(&DemoMan);
    Engine->globalObject().setProperty("demo", sv);

    A3FarmSI * farm = new A3FarmSI(this);
    QJSValue svf = Engine->newQObject(farm);
    Engine->globalObject().setProperty("farm", svf);

    APhotonSimSI * lsim = new APhotonSimSI(this);
    QJSValue svls = Engine->newQObject(lsim);
    Engine->globalObject().setProperty("lsim", svls);
    */
}

void A3ScriptWorker::evaluate(const QString & script)
{
    if (bBusy) return;

    Engine->setInterrupted(false);

    bBusy = true;
    Result = Engine->evaluate(script);
    bBusy = false;

    qDebug() << "Script eval result:\n" << Result.toString();

    bool ok = !Result.isError();
    emit evalFinished(ok);
}

void A3ScriptWorker::exit()
{
    Engine->setInterrupted(true);
    do {} while (!Engine->isInterrupted());

//    emit stopped();
}
