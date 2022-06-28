//#define PY_SSIZE_T_CLEAN

#include "apythoninterface.h"

#include <QObject>
#include <QDebug>
#include <QMetaMethod>
#include <QMetaType>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

struct AModuleData
{
    std::string   StrName;
    QObject     * Object;
};
static std::vector<AModuleData*> ModData;          // filled directly
static std::string * strName;                      // used to transfer the module name to function pointer during init phase
static std::vector<PyModuleDef*> ModDefinitions;   // filled indirectly

APythonInterface::APythonInterface()
{
    const wchar_t * program = Py_DecodeLocale(QString("Ants3Python").toLatin1().data(), NULL);
    Py_SetProgramName(program);
}

APythonInterface::~APythonInterface()
{
    Py_FinalizeEx();
}

static PyObject* initModule() // requires strName to be filled with the pointer to a persistent name c-string!
{
    qDebug() << "init called!";

    PyModuleDef * md = new PyModuleDef{PyModuleDef_HEAD_INIT, strName->data(), NULL, -1, NULL, NULL, NULL,NULL,NULL};
    ModDefinitions.push_back(md);

    return PyModule_Create(md);
}

bool APythonInterface::registerUnit(QObject * unit, const QString & unitName)  // just the first phase, continues in initialize()
{
    AModuleData * data = new AModuleData();
    ModData.push_back(data);

    data->Object  = unit;
    data->StrName = unitName.toLatin1().data();

    PyImport_AppendInittab(data->StrName.data(), initModule);
    return true;
}

bool APythonInterface::pyObjectToVariant(PyObject * po, QVariant & var)
{
    if (PyBool_Check(po))
    {
        bool res = PyObject_IsTrue(po);
        var = res;
        return true;
    }
    if (PyLong_Check(po))
    {
        int i = PyLong_AsLong(po);
        var = i;
        return true;
    }
    if (PyFloat_Check(po))
    {
        double d = PyFloat_AsDouble(po);
        var = d;
        return true;
    }
    if (PyUnicode_Check(po))
    {
        QString s(PyUnicode_AsUTF8(po));
        var = s;
        return true;
    }
    if (PyTuple_Check(po))
    {
        QVariantList vl;
        pyObjectToVariantList(po, vl);
        var = vl;
        return true;
    }
    if (PyDict_Check(po))
    {
        QVariantMap vm;
        pyObjectToVariantMap(po, vm);
        var = vm;
        return true;
    }

    return false;
}

bool APythonInterface::pyObjectToVariantList(PyObject * po, QVariantList & list)
{
    int res = PyTuple_Check(po);
    if (res)
    {
        const int size = PyTuple_Size(po);
        list.resize(size);
        for (int i = 0; i < size; i++)
        {
            PyObject * el = PyTuple_GetItem(po, i);
            bool ok = pyObjectToVariant(el, list[i]);
            if (!ok) break;
        }
        return true;
    }

    return false;
}

bool APythonInterface::pyObjectToVariantMap(PyObject * po, QVariantMap & map)
{
    int res = PyDict_Check(po);
    if (res)
    {
        PyObject * keys = PyDict_Keys(po);
        PyObject * vals = PyDict_Values(po);

        const int size = PyList_Size(keys);
        qDebug() << "++++++++++++++++++++++++" << size;
        for (int i = 0; i < size; i++)
        {
            QString  s(PyUnicode_AsUTF8( PyList_GetItem(keys, i) ));
            QVariant val;
            bool ok = pyObjectToVariant( PyList_GetItem(vals, i), val);
            if (!ok) return false;
            map[s] = val;
        }
        return true;
    }

    return false;
}

// infrastructure for Python->Qt argument conversion
struct AArgDataHolder
{
    bool         Bool;
    int          Int;
    double       Double;
    QString      String;
    QVariant     Variant;
    QVariantList List;
    QVariantMap  Map;
};
static std::array<AArgDataHolder,10> ArgDataHolders;
static bool parseArg(int iArg, PyObject * args, QMetaMethod & met, QGenericArgument & arg)
{
    PyObject * po = PyTuple_GetItem(args, iArg);
    AArgDataHolder & h = ArgDataHolders[iArg];

    QMetaType asType = met.parameterMetaType(iArg);

    if (asType == QMetaType(QMetaType::Bool))
    {
        int res = PyObject_IsTrue(po);
        if (res != -1)
        {
            h.Bool = res;
            arg = Q_ARG(bool, h.Bool);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::Int))
    {
        int res = PyLong_Check(po);
        if (res)
        {
            h.Int = PyLong_AsLong(po);
            arg = Q_ARG(int, h.Int);
            return true;
        }
        res = PyFloat_Check(po);
        if (res)
        {
            h.Int = PyFloat_AsDouble(po);
            arg = Q_ARG(int, h.Int);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::Double))
    {
        int res = PyFloat_Check(po);
        if (res)
        {
            h.Double = PyFloat_AsDouble(po);
            arg = Q_ARG(double, h.Double);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::QString))
    {
        int res = PyUnicode_Check(po);
        if (res)
        {
            h.String = PyUnicode_AsUTF8(po);
            arg = Q_ARG(QString, h.String);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::QVariant))
    {
        bool ok = APythonInterface::pyObjectToVariant(po, h.Variant);
        if (ok)
        {
            arg = Q_ARG(QVariant, h.Variant);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::QVariantList))
    {
        bool ok = APythonInterface::pyObjectToVariantList(po, h.List);
        if (ok)
        {
            arg = Q_ARG(QVariantList, h.List);
            return true;
        }
    }
    else if (asType == QMetaType(QMetaType::QVariantMap))
    {
        bool ok = APythonInterface::pyObjectToVariantMap(po, h.Map);
        if (ok)
        {
            arg = Q_ARG(QVariantMap, h.Map);
            return true;
        }
    }

    PyErr_SetString(PyExc_TypeError, QString("Method '%1' argument %2 conversion error to %3").arg(met.name()).arg(iArg).arg(asType.name()).toLatin1().data());
    return false;
}

PyObject* APythonInterface::variantToPyObject(const QVariant & var)
{
    const QMetaType mtype = var.metaType();
    if      (mtype == QMetaType(QMetaType::Void))         return Py_NewRef(Py_None);
    else if (mtype == QMetaType(QMetaType::Bool))         return PyBool_FromLong(var.toBool());
    else if (mtype == QMetaType(QMetaType::Int))          return PyLong_FromLong(var.toInt());
    else if (mtype == QMetaType(QMetaType::Double))       return PyFloat_FromDouble(var.toDouble());
    else if (mtype == QMetaType(QMetaType::QString))      return PyUnicode_FromString(var.toString().toLatin1().data());
    else if (mtype == QMetaType(QMetaType::QVariantList)) return listToTuple(var.toList());
    else if (mtype == QMetaType(QMetaType::QVariantMap))  return mapToDict(var.toMap());
    return nullptr;
}

PyObject* APythonInterface::listToTuple(const QVariantList & list)
{
    const size_t size = list.size();
    PyObject * tu = PyTuple_New(size);
    for (size_t i = 0; i < size; i++)
    {
        PyObject * el = variantToPyObject(list[i]);
        PyTuple_SetItem(tu, i, el);
    }
    return tu;
}

PyObject* APythonInterface::mapToDict(const QVariantMap & map)
{
    PyObject * dic = PyDict_New();
    QMap<QString, QVariant>::const_iterator it = map.constBegin();
    while (it != map.constEnd())
    {
        //qDebug() << it.key() << ": " << it.value();
        PyDict_SetItemString(dic, it.key().toLatin1().data(), variantToPyObject(it.value()));
        ++it;
    }
    return dic;
}

bool         retBool;
int          retInt;
double       retDouble;
QString      retString;
QVariant     retVariant;
QVariantList retList;
QVariantMap  retMap;
static PyObject* baseFunction(PyObject *caller, PyObject *args)
{
    qDebug() << "Base function called!";
    qDebug() << "caller-->" << caller->ob_type->tp_name;
    int iModule;
    int iMethod;
    PyArg_ParseTuple(caller, "ii", &iModule, &iMethod);
    qDebug() << iModule << iMethod;

    QObject * obj = ModData[iModule]->Object;
    QMetaMethod met = obj->metaObject()->method(iMethod);
    /*
    int retVal;
    met.invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(int, retVal));
    return PyLong_FromLong(retVal);
    */

    const QMetaType mtype = met.returnMetaType();
    qDebug() << "==============>" << mtype.name();
    QGenericReturnArgument ret;
    if      (mtype == QMetaType(QMetaType::Void))         ; // keep default
    else if (mtype == QMetaType(QMetaType::Bool))         ret = Q_RETURN_ARG(bool,         retBool);
    else if (mtype == QMetaType(QMetaType::Int))          ret = Q_RETURN_ARG(int,          retInt);
    else if (mtype == QMetaType(QMetaType::Double))       ret = Q_RETURN_ARG(double,       retDouble);
    else if (mtype == QMetaType(QMetaType::QString))      ret = Q_RETURN_ARG(QString,      retString);
    else if (mtype == QMetaType(QMetaType::QVariant))     ret = Q_RETURN_ARG(QVariant,     retVariant);
    else if (mtype == QMetaType(QMetaType::QVariantList)) ret = Q_RETURN_ARG(QVariantList, retList);
    else if (mtype == QMetaType(QMetaType::QVariantMap))  ret = Q_RETURN_ARG(QVariantMap,  retMap);
    else
    {
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported return argument of type %2").arg(met.name()).arg(mtype.name())).toLatin1().data());
        return nullptr;
    }

    const int numArgs = met.parameterCount();
    if (PyTuple_Size(args) < numArgs)
    {
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires at least %2 argument(s)").arg(met.name()).arg(numArgs)).toLatin1().data());
        return nullptr;
    }

    std::array<QGenericArgument,10> ar;
    std::fill(ar.begin(), ar.end(), QGenericArgument());
    for (int i = 0; i < numArgs; i++)
    {
        bool ok = parseArg(i, args, met, ar[i]);
        if (!ok) return nullptr;
    }
    if (!ret.name()) met.invoke(obj, Qt::DirectConnection,      ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]);
    else             met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]); // ugly, but it's Qt :)

    /*
    switch (numArgs)
    {
    case 0:
        {
            if (!ret.name()) met.invoke(obj, Qt::DirectConnection);
            else             met.invoke(obj, Qt::DirectConnection, ret);
            break;
        }
    case 1:
        {
            QGenericArgument arg0;
            bool ok = parseArg(0, args, met, arg0);
            if (!ok) return nullptr;
            if (!ret.name()) met.invoke(obj, Qt::DirectConnection, arg0);
            else             met.invoke(obj, Qt::DirectConnection, ret, arg0);
            break;
        }
    case 2:
        {
            QGenericArgument arg0, arg1;
            bool ok = parseArg(0, args, met, arg0);
            if (!ok) return nullptr;
            ok = parseArg(1, args, met, arg1);
            if (!ok) return nullptr;
            if (!ret.name()) met.invoke(obj, Qt::DirectConnection, arg0, arg1);
            else             met.invoke(obj, Qt::DirectConnection, ret, arg0, arg1);
            break;
        }
    }
    */

    if      (mtype == QMetaType(QMetaType::Void))         return Py_NewRef(Py_None);
    else if (mtype == QMetaType(QMetaType::Bool))         return PyBool_FromLong(retBool);
    else if (mtype == QMetaType(QMetaType::Int))          return PyLong_FromLong(retInt);
    else if (mtype == QMetaType(QMetaType::Double))       return PyFloat_FromDouble(retDouble);
    else if (mtype == QMetaType(QMetaType::QString))      return PyUnicode_FromString(retString.toLatin1().data());
    else if (mtype == QMetaType(QMetaType::QVariant))     return APythonInterface::variantToPyObject(retVariant);
    else if (mtype == QMetaType(QMetaType::QVariantList)) return APythonInterface::listToTuple(retList);
    else if (mtype == QMetaType(QMetaType::QVariantMap))  return APythonInterface::mapToDict(retMap);

    PyErr_SetString(PyExc_TypeError, QString("Unexpected mismatch in return type of method '%1'").arg(met.name()).toLatin1().data());
    return nullptr;
}

static PyMethodDef baseMethodDef = {"dummy",
                                    &baseFunction,
                                    METH_VARARGS,
                                    "Base method for interfacing QObjects"};

void APythonInterface::initialize()
{
    Py_Initialize();

    PyObject * mainModule = PyImport_GetModuleDict();

    for (size_t iUnit = 0; iUnit < ModData.size(); iUnit++)
    {
        qDebug() << "-->" << iUnit;
        strName = &ModData[iUnit]->StrName;
        evalScript( QString("import %1").arg(strName->data()) );

        PyObject * module = PyDict_GetItemString(mainModule, strName->data());
        qDebug() << "  module:" << module;

        const int numMethods = ModData[iUnit]->Object->metaObject()->methodCount();
        qDebug() << "  num meta methods" << numMethods;
        for (int iMet = 0; iMet < numMethods; iMet++)
        {
            QMetaMethod meta = ModData[iUnit]->Object->metaObject()->method(iMet);
            if ((meta.methodType() == QMetaMethod::Slot) && meta.access() == QMetaMethod::Public)
            {
                if (meta.name() == "deleteLater") continue;
                qDebug() << "  ->" << meta.name() << " with meta index:" << iMet;

                PyObject* caller = PyTuple_Pack(2, PyLong_FromSize_t(iUnit), PyLong_FromLong(iMet));
                PyObject * pyFunc = PyCFunction_New(&baseMethodDef, caller);
                if (pyFunc == NULL) qDebug() << "PyCFunction is null!";
                int res = PyModule_AddObject(module, meta.name(), pyFunc); //store method names?
                if (res < 0) qDebug() << "Failed to create object for this method";
            }
        }
    }
    qDebug() << "initialized finished\n";
}

bool APythonInterface::evalScript(const QString & script)
{
    PyObject * mainModule = getMainModule();

    PyObject * dict = PyModule_GetDict(mainModule);
    if (!dict)
    {
        qCritical() << "Cannot get dictionary of the main module!";
        exit(111);
    }

    qDebug() << "Running script:" << script;
    PyObject * outObj = PyRun_String(script.toLatin1().data(), Py_file_input, dict, dict);

    if (!outObj)
    {
        qDebug() << "Error!";
        handleError();
        return false;
    }

    Py_DECREF(outObj);
    return true;
}

PyObject * APythonInterface::getMainModule()
{
    PyObject * md = PyImport_GetModuleDict();    // borrowed
    return PyDict_GetItemString(md, "__main__"); // borrowed
}
