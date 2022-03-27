#include "aminijs_si.h"
#include "ajscripthub.h"
#include "ajscriptmanager.h"

#include <QDebug>

#include "Math/Functor.h"

double AFunctorJS::operator()(const double * p)
{
    AJScriptManager & ScriptManager = AJScriptHub::manager();
    if (ScriptManager.isAborted()) return 1e30;

        QString str;
        for (int i=0; i<ScriptManager.MiniNumVariables; i++)
        str += QString::number(p[i]) + "  ";
        qDebug() << "Functor call with parameters:"<<str;

    const double result = ScriptManager.runMinimizationFunction(p);

        qDebug() << "Minimization parameter value obtained:"<<result;

    return result;
}

// ---

AMiniJS_SI::~AMiniJS_SI()
{
    delete Functor;
}

void AMiniJS_SI::setFunctorName(const QString & name)
{
    AJScriptManager & SM = AJScriptHub::manager();
    SM.MiniFunctionName = name;
}

void AMiniJS_SI::configureNumVariables(int num)
{
    AJScriptManager & SM = AJScriptHub::manager();
    SM.MiniNumVariables = num;
}

ROOT::Math::Functor * AMiniJS_SI::configureFunctor()
{
    AJScriptManager & ScriptManager = AJScriptHub::manager();

    bool ok = ScriptManager.testMinimizationFunction();
    if (!ok) return nullptr;

    Functor = new AFunctorJS();
    return new ROOT::Math::Functor(*Functor, ScriptManager.MiniNumVariables);
}
