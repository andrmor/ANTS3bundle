#include "aminipython_si.h"
#include "apythonscriptmanager.h"
#include "ascripthub.h"

#include <QDebug>

#include "Math/Functor.h"

double AFunctorPython::operator()(const double *p)
{
    APythonScriptManager & psm = AScriptHub::getInstance().getPythonManager();
    if (psm.isAborted()) return 1e30;

        QString str;
        for (int i=0; i<psm.MiniNumVariables; i++)
        {
            if (i != 0) str += ", ";
            str += QString::number(p[i]);
        }
        qDebug() << "Functor call with parameters:"<<str;

    const double result = psm.runMinimizationFunction(p);

        qDebug() << "Minimization parameter value obtained:"<<result;

    return result;

    /*
    const int numArguments = psm.MiniNumVariables;
    PythonQtObjectPtr tupleArgs;
    tupleArgs.setNewRef( PyTuple_New(numArguments) );
    for (int i=0; i<numArguments; i++)
        PyTuple_SetItem(tupleArgs, i, PyFloat_FromDouble(p[i]));
    PythonQtObjectPtr pyth_val;
    pyth_val.setNewRef( PyObject_Call(psm.MinimizationFunctor, tupleArgs, NULL) );

    double result = 1e30;
    bool bOK;
    if (pyth_val)
        result = PythonQtConv::PyObjGetDouble(pyth_val, false, bOK);
    return result;
    */
    return 0;
}

AMiniPython_SI::~AMiniPython_SI()
{
    delete Functor;
}

bool AMiniPython_SI::wasAborted() const
{
    APythonScriptManager & psm = AScriptHub::getInstance().getPythonManager();
    return psm.isAborted();
}

void AMiniPython_SI::setFunctorName(const QString & name)
{
    APythonScriptManager & psm = AScriptHub::getInstance().getPythonManager();
    psm.MiniFunctionName = name;
}

void AMiniPython_SI::configureNumVariables(int num)
{
    APythonScriptManager & psm = AScriptHub::getInstance().getPythonManager();
    psm.MiniNumVariables = num;
}

ROOT::Math::Functor * AMiniPython_SI::configureFunctor()
{
    APythonScriptManager & psm = AScriptHub::getInstance().getPythonManager();
    bool ok = psm.testMinimizationFunction();
    if (!ok) return nullptr;

    Functor = new AFunctorPython();
    return new ROOT::Math::Functor(*Functor, psm.MiniNumVariables);

/*
    delete baseFunctor; baseFunctor = nullptr;

    APythonScriptManager* psm = static_cast<APythonScriptManager*>(ScriptManager);

    if (psm->GlobalDict.object())
    {
        psm->MinimizationFunctor.setNewRef( PyDict_GetItemString(psm->GlobalDict, ScriptManager->MiniFunctionName.toLatin1().data()) );

        if (psm->MinimizationFunctor && PyCallable_Check(psm->MinimizationFunctor))
        {
            AFunctor_PythonScript * f = new AFunctor_PythonScript(psm);
            baseFunctor = f;
            return new ROOT::Math::Functor(*f, psm->MiniNumVariables);
        }
    }

    psm->MinimizationFunctor = nullptr;
    return nullptr;
    */
}

