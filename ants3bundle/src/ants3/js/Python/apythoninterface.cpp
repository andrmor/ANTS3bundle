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

static PyObject * initModule() // requires strName to be filled with the pointer to a persistent name c-string!
{
    //qDebug() << "init called!";

    PyModuleDef * md = new PyModuleDef{PyModuleDef_HEAD_INIT, strName->data(), NULL, -1, NULL, NULL, NULL,NULL,NULL};
    ModDefinitions.push_back(md);

    return PyModule_Create(md);
}


APythonInterface::APythonInterface()
{
    const wchar_t * program = Py_DecodeLocale(QString("Ants3Python").toLatin1().data(), NULL);
    Py_SetProgramName(program);
}

APythonInterface::~APythonInterface()
{
    Py_FinalizeEx();
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
        bool ok = pyObjectToVariantList(po, vl);
        if (!ok) return false;
        var = vl;
        return true;
    }
    if (PyDict_Check(po))
    {
        QVariantMap vm;
        bool ok = pyObjectToVariantMap(po, vm);
        if (!ok) return false;
        var = vm;
        return true;
    }

    if (PyList_Check(po))
    {
        QVariantList vl;
        bool ok = pyObjectToVariantList(po, vl);
        if (!ok) return false;
        var = vl;
        return true;
    }
    if (PySet_Check(po))
    {
        QVariantList vl;
        bool ok = pyObjectToVariantList(po, vl);
        if (!ok) return false;
        var = vl;
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        list.clear(); list.reserve(size); for (int i = 0; i < size; i++) list << QVariant();
#else
        list.resize(size);
#endif
        for (int i = 0; i < size; i++)
        {
            PyObject * el = PyTuple_GetItem(po, i); // borrowed
            bool ok = pyObjectToVariant(el, list[i]);
            if (!ok) break;
        }
        return true;
    }

    res = PyList_Check(po);
    if (res)
    {
        const int size = PyList_Size(po);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        list.clear(); list.reserve(size); for (int i = 0; i < size; i++) list << QVariant();
#else
        list.resize(size);
#endif
        for (int i = 0; i < size; i++)
        {
            PyObject * el = PyList_GetItem(po, i); // borrowed
            bool ok = pyObjectToVariant(el, list[i]);
            if (!ok) break;
        }
        return true;
    }

    // last resort: check if it is possible to iterate over items
    list.clear();
    PyObject * iterator = PyObject_GetIter(po); // new ref
    if (!iterator) return false;
    PyObject * item = nullptr;
    bool ok = false;
    while ((item = PyIter_Next(iterator)))  // new ref
    {
        QVariant var;
        ok = pyObjectToVariant(item, var);
        Py_DECREF(item);
        if (!ok) break;
        list.push_back(var);
    }
    Py_DECREF(iterator);
    return ok;
}

bool APythonInterface::pyObjectToVariantMap(PyObject * po, QVariantMap & map)
{
    int res = PyDict_Check(po);
    if (res)
    {
        PyObject * keys = PyDict_Keys(po);   // new ref
        PyObject * vals = PyDict_Values(po); // new ref

        const int size = PyList_Size(keys);
        //qDebug() << "++++++++++++++++++++++++" << size;
        for (int i = 0; i < size; i++)
        {
            QString  s(PyUnicode_AsUTF8( PyList_GetItem(keys, i) ));     // borrowed
            QVariant val;
            bool ok = pyObjectToVariant( PyList_GetItem(vals, i), val);  // borrowed
            if (!ok)
            {
                Py_DecRef(keys);
                Py_DecRef(vals);
                return false;
            }
            map[s] = val;
        }
        Py_DecRef(keys);
        Py_DecRef(vals);
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
    PyObject * po = PyTuple_GetItem(args, iArg);  // borrowed
    AArgDataHolder & h = ArgDataHolders[iArg];

    int asType = met.parameterType(iArg); // QMetaType asType = met.parameterMetaType(iArg);

    if (asType == QMetaType::Bool) // if (asType == QMetaType(QMetaType::Bool))
    {
        int res = PyObject_IsTrue(po);
        if (res != -1)
        {
            h.Bool = res;
            arg = Q_ARG(bool, h.Bool);
            return true;
        }
    }
    else if (asType == QMetaType::Int) // else if (asType == QMetaType(QMetaType::Int))
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
    else if (asType == QMetaType::Double) // else if (asType == QMetaType(QMetaType::Double))
    {
        int res = PyFloat_Check(po);
        if (res)
        {
            h.Double = PyFloat_AsDouble(po);
            arg = Q_ARG(double, h.Double);
            return true;
        }
    }
    else if (asType == QMetaType::QString) // else if (asType == QMetaType(QMetaType::QString))
    {
        int res = PyUnicode_Check(po);
        if (res)
        {
            h.String = PyUnicode_AsUTF8(po);
            arg = Q_ARG(QString, h.String);
            return true;
        }
    }
    else if (asType == QMetaType::QVariant) // else if (asType == QMetaType(QMetaType::QVariant))
    {
        bool ok = APythonInterface::pyObjectToVariant(po, h.Variant);
        if (ok)
        {
            arg = Q_ARG(QVariant, h.Variant);
            return true;
        }
    }
    else if (asType == QMetaType::QVariantList) //  else if (asType == QMetaType(QMetaType::QVariantList))
    {
        bool ok = APythonInterface::pyObjectToVariantList(po, h.List);
        if (ok)
        {
            arg = Q_ARG(QVariantList, h.List);
            return true;
        }
    }
    else if (asType == QMetaType::QVariantMap) // else if (asType == QMetaType(QMetaType::QVariantMap))
    {
        bool ok = APythonInterface::pyObjectToVariantMap(po, h.Map);
        if (ok)
        {
            arg = Q_ARG(QVariantMap, h.Map);
            return true;
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    PyErr_SetString(PyExc_TypeError, QString("Method '%1' argument %2 conversion error to %3").arg(met.name().data()).arg(iArg).arg(QMetaType(asType).name().data()).toLatin1().data());
#else
    PyErr_SetString(PyExc_TypeError, QString("Method '%1' argument %2 conversion error to %3").arg(met.name()).arg(iArg).arg(asType.name()).toLatin1().data());
#endif
    return false;
}

PyObject* APythonInterface::variantToPyObject(const QVariant & var)
{
    const int mtype = var.type(); // const QMetaType mtype = var.metaType();
    if      (mtype == QMetaType::Void)         Py_RETURN_NONE; // return Py_NewRef(Py_None);
    else if (mtype == QMetaType::Bool)         return PyBool_FromLong(var.toBool()); // else if (mtype == QMetaType(QMetaType::Bool))
    else if (mtype == QMetaType::Int)          return PyLong_FromLong(var.toInt());
    else if (mtype == QMetaType::Double)       return PyFloat_FromDouble(var.toDouble());
    else if (mtype == QMetaType::QString)      return PyUnicode_FromString(var.toString().toLatin1().data());
    else if (mtype == QMetaType::QVariantList) return listToTuple(var.toList());
    else if (mtype == QMetaType::QVariantMap)  return mapToDict(var.toMap());
    return nullptr;
}

PyObject * APythonInterface::listToTuple(const QVariantList & list)
{
    const size_t size = list.size();
    PyObject * tu = PyTuple_New(size); // new ref
    for (size_t i = 0; i < size; i++)
    {
        PyObject * el = variantToPyObject(list[i]);
        PyTuple_SetItem(tu, i, el);
    }
    return tu;
}

PyObject* APythonInterface::mapToDict(const QVariantMap & map)
{
    PyObject * dic = PyDict_New(); // new ref
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

    const int mtype = met.returnType(); // const QMetaType mtype = met.returnMetaType();
    qDebug() << "==============>" << QMetaType(mtype).name(); // qDebug() << "==============>" << mtype.name();
    QGenericReturnArgument ret;
    if      (mtype == QMetaType::Void)         ; // keep default
    else if (mtype == QMetaType::Bool)         ret = Q_RETURN_ARG(bool,         retBool); //  else if (mtype == QMetaType(QMetaType::Bool))
    else if (mtype == QMetaType::Int)          ret = Q_RETURN_ARG(int,          retInt);
    else if (mtype == QMetaType::Double)       ret = Q_RETURN_ARG(double,       retDouble);
    else if (mtype == QMetaType::QString)      ret = Q_RETURN_ARG(QString,      retString);
    else if (mtype == QMetaType::QVariant)     ret = Q_RETURN_ARG(QVariant,     retVariant);
    else if (mtype == QMetaType::QVariantList) ret = Q_RETURN_ARG(QVariantList, retList);
    else if (mtype == QMetaType::QVariantMap)  ret = Q_RETURN_ARG(QVariantMap,  retMap);
    else
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported return argument of type %2").arg(met.name().data()).arg(QMetaType(mtype).name().data())).toLatin1().data());
#else
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported return argument of type %2").arg(met.name()).arg(mtype.name())).toLatin1().data());
#endif
        return nullptr;
    }

    const int numArgs = met.parameterCount();
    if (PyTuple_Size(args) < numArgs)
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires at least %2 argument(s)").arg(met.name().data()).arg(numArgs)).toLatin1().data());
#else
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires at least %2 argument(s)").arg(met.name()).arg(numArgs)).toLatin1().data());
#endif
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

    if      (mtype == QMetaType::Void)         Py_RETURN_NONE; // return Py_NewRef(Py_None);
    else if (mtype == QMetaType::Bool)         return PyBool_FromLong(retBool); // else if (mtype == QMetaType(QMetaType::Bool))
    else if (mtype == QMetaType::Int)          return PyLong_FromLong(retInt);
    else if (mtype == QMetaType::Double)       return PyFloat_FromDouble(retDouble);
    else if (mtype == QMetaType::QString)      return PyUnicode_FromString(retString.toLatin1().data());
    else if (mtype == QMetaType::QVariant)     return APythonInterface::variantToPyObject(retVariant);
    else if (mtype == QMetaType::QVariantList) return APythonInterface::listToTuple(retList);
    else if (mtype == QMetaType::QVariantMap)  return APythonInterface::mapToDict(retMap);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    PyErr_SetString(PyExc_TypeError, QString("Unexpected mismatch in return type of method '%1'").arg(met.name().data()).toLatin1().data());
#else
    PyErr_SetString(PyExc_TypeError, QString("Unexpected mismatch in return type of method '%1'").arg(met.name()).toLatin1().data());
#endif
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
    PyErr_Clear();

    PyObject * mainModule = getMainModule();

    PyObject * dict = PyModule_GetDict(mainModule);
    if (!dict)
    {
        qCritical() << "Cannot get dictionary of the main module!";
        exit(111);
    }

    ErrorDescription.clear();
    ErrorLineNumber = -1;

    qDebug() << "Running script:" << script;
    PyObject * outObj = PyRun_String(script.toLatin1().data(), Py_file_input, dict, dict);

    if (!outObj)
    {
        handleError();
        return false;
    }

    Py_DECREF(outObj);
    return true;
}

void APythonInterface::abort()
{
    qDebug() << "--> abort of Python Interface...";
    PyErr_SetInterrupt();
}

PyObject * APythonInterface::getMainModule()
{
    PyObject * md = PyImport_GetModuleDict();    // borrowed
    return PyDict_GetItemString(md, "__main__"); // borrowed
}

void APythonInterface::handleError()
{
    qDebug() << "-------------ERROR-----------";
    PyObject * pyErr = PyErr_Occurred();
    if (!pyErr)
    {
        ErrorDescription = "Error reported, by PyErr not generated!";
        return;
    }

    PyObject * ptype;
    PyObject * pvalue;
    PyObject * ptraceback;
    //PyErr_GetExcInfo(&ptype, &pvalue, &ptraceback);
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);

    if (PyErr_GivenExceptionMatches(pyErr, PyExc_SyntaxError))
    {
        qDebug() << "...syntax-related error";  //e.g. exception SyntaxError(message, details)

        PyObject * first = PyTuple_GetItem(pvalue, 0); // borrowed
        //qDebug() << "QQQQQQQQQQQQQQQ:" << PyUnicode_AsUTF8(PyObject_Str(first));
        ErrorDescription = PyUnicode_AsUTF8(PyObject_Str(first));

        PyObject * second = PyTuple_GetItem(pvalue, 1); // borrowed
        //qDebug() << PyLong_AsLong( PyTuple_GetItem(second, 1) );
        ErrorLineNumber = PyLong_AsLong( PyTuple_GetItem(second, 1) );
        //qDebug() << "...Storing info:" << ErrorDescription << ErrorLineNumber;
        return;
    }

    // Not (yet) implemented errors types
    PyObject* repr = PyObject_Repr(pvalue);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char * bytes = PyBytes_AS_STRING(str);

    ErrorDescription = QString(bytes);
    qDebug() << ">>>>>>>" << ErrorDescription;

    Py_XDECREF(repr);
    Py_XDECREF(str);


    {
        PyObject* repr = PyObject_Repr(ptype);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);

        //printf("REPR: %s\n", bytes);
        qDebug() << ">>>>>>>" << bytes;

        Py_XDECREF(repr);
        Py_XDECREF(str);
    }
    {
        PyObject* repr = PyObject_Repr(pvalue);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);

        //printf("REPR: %s\n", bytes);
        qDebug() << ">>>>>>>" << bytes;

        Py_XDECREF(repr);
        Py_XDECREF(str);
    }
    {
        PyObject* repr = PyObject_Repr(ptraceback);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);

        //printf("REPR: %s\n", bytes);
        qDebug() << ">>>>>>>" << bytes;

        Py_XDECREF(repr);
        Py_XDECREF(str);
    }

    //const char *errMsg = PyUnicode_AsUTF8(PyObject_Str(pvalue));
    //qDebug() << "\n\n" << errMsg;

}
