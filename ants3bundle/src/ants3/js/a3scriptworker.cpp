#include "a3scriptworker.h"
#include "a3scriptres.h"

#include <QJSEngine>
#include <QDebug>

A3ScriptWorker::~A3ScriptWorker()
{
    qDebug() << "Destr for ScriptWorker";
    delete Engine;
}

void A3ScriptWorker::abort()
{
    //Engine->setInterrupted(true);
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

#include "a3particlesimmanager.h"
#include "a3farmsi.h"
void A3ScriptWorker::initialize()
{
    Engine = new QJSEngine();

    QJSValue sv = Engine->newQObject(ScrRes.ParticleSim);
    Engine->globalObject().setProperty("simp", sv);

    A3FarmSI * farm = new A3FarmSI(this);
    QJSValue svf = Engine->newQObject(farm);
    Engine->globalObject().setProperty("farm", svf);
}

void A3ScriptWorker::evaluate(const QString & script)
{
    if (bBusy) return;

    //Engine->setInterrupted(false);

    bBusy = true;
    Result = Engine->evaluate(script);
    bBusy = false;

    qDebug() << "Script eval result:\n" << Result.toString();

    bool ok = !Result.isError();
    emit evalFinished(ok);
}

void A3ScriptWorker::exit()
{
    //Engine->setInterrupted(true);
    //do {} while (!Engine->isInterrupted());
    emit stopped();
}
