#ifndef APYTHONINTERFACE_H
#define APYTHONINTERFACE_H

#define PY_SSIZE_T_CLEAN
#include "Python.h"  // must be first include!

#include <vector>

#include <QString>
#include <QVariant>

class QObject;
struct APyModuleData;

class APythonInterface
{
public:
    APythonInterface();
    ~APythonInterface();

    bool registerUnit(QObject * unit, const QString & unitName);
    void initialize();

    bool evalScript(const QString & script);

    void abort();

    QString ErrorDescription;
    int     ErrorLineNumber = -1;

    PyObject * getMainModule();

protected:
    void handleError();

public:
    static PyObject* variantToPyObject(const QVariant & var);
    static PyObject* listToTuple(const QVariantList & list);
    static PyObject* mapToDict(const QVariantMap & map);
    static bool pyObjectToVariant(PyObject * po, QVariant & var);
    static bool pyObjectToVariantList(PyObject *po, QVariantList & list);
    static bool pyObjectToVariantMap(PyObject * po, QVariantMap & map);

    //-- for minimizer --
    PyObject* MinimizationFunctor = nullptr;
    //----

};

#endif // APYTHONINTERFACE_H
