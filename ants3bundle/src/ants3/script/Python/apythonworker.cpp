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
    qDebug() << "Python Worker abort triggered";
    PyInterface->abort();

    for (AScriptInterface * inter : Interfaces)
        inter->abortRun();

    bBusy = false;
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

bool APythonWorker::testMinimizationFunction(const QString & name) const
{
    Py_XDECREF( PyInterface->MinimizationFunctor );

    //qDebug() << "--> Test:" << name;

    PyObject* mainModule = PyInterface->getMainModule();
    PyObject* dict = PyModule_GetDict(mainModule);

    //qDebug() << "Dict:" << dict;

    PyInterface->MinimizationFunctor = PyDict_GetItemString(dict, name.toLatin1().data());
    //qDebug() << "---->" << PyInterface->MinimizationFunctor;

    if (PyInterface->MinimizationFunctor && PyCallable_Check(PyInterface->MinimizationFunctor))
    {
        //qDebug() << "Callable!";
        return true;
    }
    //qDebug() << "NOT callable";
    PyInterface->MinimizationFunctor = nullptr;
    return false;
}

double APythonWorker::runMinimizationFunction(const double *p, int numParameters)
{
    //qDebug() << "  Still callable?" << PyCallable_Check(PyInterface->MinimizationFunctor) << "num_P:" << numParameters;

    PyObject* tupleArgs = PyTuple_New(numParameters);
    for (int i = 0; i < numParameters; i++)
    {
        //qDebug() << "    ->" << i << p[i];
        PyTuple_SetItem(tupleArgs, i, PyFloat_FromDouble(p[i]));
    }

    PyObject * pyth_val = PyObject_Call(PyInterface->MinimizationFunctor, tupleArgs, NULL);
    //qDebug() << "return obj:" << pyth_val;
    Py_DecRef(tupleArgs);

    if (!pyth_val)
    {
        qDebug() << "Python mini: received NULL from functor call";
        return 0;
    }
    const double result = PyFloat_AsDouble(pyth_val);
    Py_DecRef(pyth_val);
    //qDebug() << "GOT:" << result;
    return result;
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
    qDebug() << "Python Worker exit triggered";
    emit stopped();
    //if (bBusy) PyInterface->abort();
}
