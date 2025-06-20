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
    //qDebug() << "Destr for PythonInterface";
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
//  !!!*** max number of parameters is 20 !!!!
static std::array<AArgDataHolder,20> ArgDataHolders;
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
bool parseArg(int iArg, PyObject * args, QMetaMethod & met, QGenericArgument & arg)
#else
bool parseArg(int iArg, PyObject * args, QMetaMethod & met, QMetaMethodArgument & arg)
#endif
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
        res = PyLong_Check(po);
        if (res)
        {
            h.Double = PyLong_AsLong(po);
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
    //PyErr_SetString(PyExc_TypeError, QString("Method '%1' argument %2 conversion error to %3").arg(met.name()).arg(iArg).arg(asType.name()).toLatin1().data());
    PyErr_SetString(PyExc_TypeError, QString("Method '%1' argument %2 conversion error to %3").arg(met.name()).arg(iArg).arg(QMetaType(asType).name()).toLatin1().data());
#endif
    return false;
}

PyObject* APythonInterface::variantToPyObject(const QVariant & var) // returns new ref
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const int mtype = var.type();
#else
    const int mtype = var.metaType().id(); // const QMetaType mtype = var.metaType();
    //qDebug() << var.typeName();
#endif
    if      (mtype == QMetaType::Void)         Py_RETURN_NONE; // return Py_NewRef(Py_None);
    else if (mtype == QMetaType::Bool)         return PyBool_FromLong(var.toBool()); // else if (mtype == QMetaType(QMetaType::Bool))
    else if (mtype == QMetaType::Int)          return PyLong_FromLong(var.toInt());
    else if (mtype == QMetaType::LongLong)     return PyLong_FromLong(var.toInt());
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
        PyTuple_SetItem(tu, i, el);  // steals ref
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
        PyObject * newVal = variantToPyObject(it.value());
        PyDict_SetItemString(dic, it.key().toLatin1().data(), newVal); // does NOT steal ref
        Py_DecRef(newVal);
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
template <size_t arraySize, size_t... indices>
constexpr auto create_parameter_pack(const std::array<QMetaMethodArgument, arraySize> & array, std::index_sequence<indices...>)
{
    return std::make_tuple(array[indices]...);
}

template <size_t numberOfArguments, size_t arraySize>
constexpr auto extract_elements_from_array(const std::array<QMetaMethodArgument, arraySize> & array)
{
    return create_parameter_pack(array, std::make_index_sequence<numberOfArguments>{});
}

template<size_t num>
bool invokeMetaMethod(QObject * obj, QMetaMethod * met, std::array<QMetaMethodArgument, 20> & methodArgumentArray)
{
    auto pack = extract_elements_from_array<num, 20>(methodArgumentArray);

    const int mtype = met->returnType(); // const QMetaType mtype = met.returnMetaType();
    //qDebug() << "return argument type:" << QMetaType(mtype).name(); // qDebug() << "==============>" << mtype.name();
    if      (mtype == QMetaType::Void)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, args...); }, pack);
    else if (mtype == QMetaType::Bool)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(bool, retBool), args...); }, pack);
    else if (mtype == QMetaType::Int)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(int, retInt), args...); }, pack);
    else if (mtype == QMetaType::Double)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(double, retDouble), args...); }, pack);
    else if (mtype == QMetaType::QString)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QString, retString), args...); }, pack);
    else if (mtype == QMetaType::QVariant)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QVariant, retVariant), args...); }, pack);
    else if (mtype == QMetaType::QVariantList)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QVariantList, retList), args...); }, pack);
    else if (mtype == QMetaType::QVariantMap)
        std::apply([obj, met](auto &&... args) { met->invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QVariantMap, retMap), args...); }, pack);
    else
    {
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported return argument of type %2").arg(met->name()).arg(QMetaType(mtype).name())).toLatin1().data());
        return false;
    }
    return true;
}

static PyObject* baseFunction_Qt67(PyObject * caller, PyObject * args)
{
    //qDebug() << "Base function called!";
    //qDebug() << "caller-->" << caller->ob_type->tp_name << "caller size:" << PyTuple_Size(caller);
    const int iModule = PyLong_AsLong(PyTuple_GetItem(caller, 0));
    QObject * obj = ModData[iModule]->Object;
    //qDebug() << "script interface object:" << obj;

    // optimized for non-overloaded methods
    int iMethod = PyLong_AsLong(PyTuple_GetItem(caller, 1));
    QMetaMethod met = obj->metaObject()->method(iMethod);
    //qDebug() << "called method name:" << met.name();
    int numArgs = met.parameterCount();
    //qDebug() << "num arguments:" << numArgs;
    if (PyTuple_Size(args) != numArgs)
    {
        const int numMethods = PyTuple_Size(caller) - 1;
        //qDebug() << "There are" << numMethods << "method versions";
        if (numMethods == 1)
        {
            PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires %2 argument(s)").arg(met.name()).arg(numArgs)).toLatin1().data());
            return nullptr;
        }
        else
        {
            //check overloaded
            bool bFound = false;
            for (int methodIndex = 1; methodIndex < numMethods; methodIndex++)
            {
                iMethod = PyLong_AsLong(PyTuple_GetItem(caller, 1 + methodIndex)); // 1 is for the iModule!
                met = obj->metaObject()->method(iMethod);
                numArgs = met.parameterCount();
                if (PyTuple_Size(args) == numArgs)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                PyErr_SetString(PyExc_TypeError, (QString("Non of the overloaded versions of method '%1' takes %2 argument(s)").arg(met.name()).arg(PyTuple_Size(args))).toLatin1().data());
                return nullptr;
            }
        }
    }

    std::array<QMetaMethodArgument, 20> argumentArray;
    for (int i = 0; i < numArgs; i++)
    {
        bool ok = parseArg(i, args, met, argumentArray[i]);
        //qDebug() << "Argument #" << i << "ok?--->" << ok;
        if (!ok) return nullptr;
    }

    //qDebug() << "invoking...";
    //qDebug() << "-------Num args" << numArgs << "Retuns void?" << returnsVoid;

    bool ok;
    switch (numArgs)
    {
    //case 0  : met.invoke(obj, Qt::DirectConnection); break;
    case 0  : ok = invokeMetaMethod<0>(obj, &met, argumentArray); break;
    case 1  : ok = invokeMetaMethod<1>(obj, &met, argumentArray); break;
    case 2  : ok = invokeMetaMethod<2>(obj, &met, argumentArray); break;
    case 3  : ok = invokeMetaMethod<3>(obj, &met, argumentArray); break;
    case 4  : ok = invokeMetaMethod<4>(obj, &met, argumentArray); break;
    case 5  : ok = invokeMetaMethod<5>(obj, &met, argumentArray); break;
    case 6  : ok = invokeMetaMethod<6>(obj, &met, argumentArray); break;
    case 7  : ok = invokeMetaMethod<7>(obj, &met, argumentArray); break;
    case 8  : ok = invokeMetaMethod<8>(obj, &met, argumentArray); break;
    case 9  : ok = invokeMetaMethod<9>(obj, &met, argumentArray); break;
    case 10 : ok = invokeMetaMethod<10>(obj, &met, argumentArray); break;
    case 11 : ok = invokeMetaMethod<11>(obj, &met, argumentArray); break;
    case 12 : ok = invokeMetaMethod<12>(obj, &met, argumentArray); break;
    case 13 : ok = invokeMetaMethod<13>(obj, &met, argumentArray); break;
    case 14 : ok = invokeMetaMethod<14>(obj, &met, argumentArray); break;
    case 15 : ok = invokeMetaMethod<15>(obj, &met, argumentArray); break;
    case 16 : ok = invokeMetaMethod<16>(obj, &met, argumentArray); break;
    case 17 : ok = invokeMetaMethod<17>(obj, &met, argumentArray); break;
    case 18 : ok = invokeMetaMethod<18>(obj, &met, argumentArray); break;
    case 19 : ok = invokeMetaMethod<19>(obj, &met, argumentArray); break;
    case 20 : ok = invokeMetaMethod<20>(obj, &met, argumentArray); break;
    default:
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported number of arguments: maximum is 20").arg(met.name())).toLatin1().data());
        qCritical("Cannot be more than 20 arguments in Pythom interface call"); exit(333);
    }
    if (!ok) return nullptr;

    //qDebug() << "converting and returning result...";
    const int mtype = met.returnType(); // const QMetaType mtype = met.returnMetaType();
    if      (mtype == QMetaType::Void)         Py_RETURN_NONE; // return Py_NewRef(Py_None);
    else if (mtype == QMetaType::Bool)         return PyBool_FromLong(retBool); // else if (mtype == QMetaType(QMetaType::Bool))
    else if (mtype == QMetaType::Int)          return PyLong_FromLong(retInt);
    else if (mtype == QMetaType::Double)       return PyFloat_FromDouble(retDouble);
    else if (mtype == QMetaType::QString)      return PyUnicode_FromString(retString.toLatin1().data());
    else if (mtype == QMetaType::QVariant)     return APythonInterface::variantToPyObject(retVariant);
    else if (mtype == QMetaType::QVariantList) return APythonInterface::listToTuple(retList);
    else if (mtype == QMetaType::QVariantMap)  return APythonInterface::mapToDict(retMap);

    PyErr_SetString(PyExc_TypeError, QString("Unexpected mismatch in return type of method '%1'").arg(met.name()).toLatin1().data());
    return nullptr;
}
#endif

static PyObject* baseFunction(PyObject * caller, PyObject *args)
{
    // too drastic change in Qt 6.7, making a separate method
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    return baseFunction_Qt67(caller, args);
#endif

    //qDebug() << "Base function called!";
    //qDebug() << "caller-->" << caller->ob_type->tp_name << "caller size:" << PyTuple_Size(caller);
    const int iModule = PyLong_AsLong(PyTuple_GetItem(caller, 0));
    QObject * obj = ModData[iModule]->Object;
    //qDebug() << "script interface object:" << obj;

    // optimized for non-overloaded methods
    int iMethod = PyLong_AsLong(PyTuple_GetItem(caller, 1));
    QMetaMethod met = obj->metaObject()->method(iMethod);
    //qDebug() << "called method name:" << met.name();
    int numArgs = met.parameterCount();
    //qDebug() << "num arguments:" << numArgs;
    if (PyTuple_Size(args) != numArgs)
    {
        const int numMethods = PyTuple_Size(caller) - 1;
        //qDebug() << "There are" << numMethods << "method versions";
        if (numMethods == 1)
        {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires %2 argument(s)").arg(met.name().data()).arg(numArgs)).toLatin1().data());
#else
            PyErr_SetString(PyExc_TypeError, (QString("Method '%1' requires %2 argument(s)").arg(met.name()).arg(numArgs)).toLatin1().data());
#endif
            return nullptr;
        }
        else
        {
            //check overloaded
            bool bFound = false;
            for (int methodIndex = 1; methodIndex < numMethods; methodIndex++)
            {
                iMethod = PyLong_AsLong(PyTuple_GetItem(caller, 1 + methodIndex)); // 1 is for the iModule!
                met = obj->metaObject()->method(iMethod);
                numArgs = met.parameterCount();
                if (PyTuple_Size(args) == numArgs)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                PyErr_SetString(PyExc_TypeError, (QString("Non of the overloaded versions of method '%1' takes %2 argument(s)").arg(met.name().data()).arg(PyTuple_Size(args))).toLatin1().data());
#else
                PyErr_SetString(PyExc_TypeError, (QString("Non of the overloaded versions of method '%1' takes %2 argument(s)").arg(met.name()).arg(PyTuple_Size(args))).toLatin1().data());
#endif
                return nullptr;
            }
        }
    }

    const int mtype = met.returnType(); // const QMetaType mtype = met.returnMetaType();
    //qDebug() << "return argument type:" << QMetaType(mtype).name(); // qDebug() << "==============>" << mtype.name();
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    QGenericReturnArgument ret;
#else
    QMetaMethodReturnArgument ret;
#endif
    bool returnsVoid = false;
    if      (mtype == QMetaType::Void)         returnsVoid = true; // keep default ret
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
        PyErr_SetString(PyExc_TypeError, (QString("Method '%1' has unsupported return argument of type %2").arg(met.name()).arg(QMetaType(mtype).name())).toLatin1().data());
#endif
        return nullptr;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    std::array<QGenericArgument,20> ar;
    std::fill(ar.begin(), ar.end(), QGenericArgument());    // !!!*** why this?
#else
    std::vector<QMetaMethodArgument> ar(numArgs, QMetaMethodArgument{});
    //std::fill(ar.begin(), ar.end(), QMetaMethodArgument{}); // !!!*** why this?
#endif
    for (int i = 0; i < numArgs; i++)
    {
        bool ok = parseArg(i, args, met, ar[i]);
        //qDebug() << "Argument #" << i << "ok?--->" << ok;
        if (!ok) return nullptr;
    }

    //qDebug() << "invoking...";
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    if (!ret.name()) met.invoke(obj, Qt::DirectConnection,      ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]);
    else             met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]); // !!!*** is there a better way?
#else
    //if (!ret.name) met.invoke(obj, Qt::DirectConnection,      ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]);
    //else           met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]);

    //if (!ret.name) // cannot use anymore, default QMetaMethodReturnArgument can have non-empty name after default constructor
    if (returnsVoid)
    {
        // at least until(and including) Qt 6.5.1 there is still no invoke with std::vector of arguments
        // see ArgDataHolders - it's max size is limited above to 20!
        switch (numArgs)
        {
        case 0  : met.invoke(obj, Qt::DirectConnection); break;
        case 1  : met.invoke(obj, Qt::DirectConnection, ar[0]); break;
        case 2  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1]); break;
        case 3  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2]); break;
        case 4  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3]); break;
        case 5  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4]); break;
        case 6  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5]); break;
        case 7  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6]); break;
        case 8  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7]); break;
        case 9  : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8]); break;
        case 10 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]); break;
        case 11 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10]); break;
        case 12 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11]); break;
        case 13 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12]); break;
        case 14 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13]); break;
        case 15 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14]); break;
        case 16 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14], ar[15]); break;
        case 17 : met.invoke(obj, Qt::DirectConnection, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14], ar[15], ar[16]); break;
        default: qCritical("Num arguments is > 17"); exit(333);
        }
    }
    else
    {
        //qDebug() << "---" << met.name() << ret.name;
        switch (numArgs)
        {
        case 0  : met.invoke(obj, Qt::DirectConnection, ret); break;
        case 1  : met.invoke(obj, Qt::DirectConnection, ret, ar[0]); break;
        case 2  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1]); break;
        case 3  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2]); break;
        case 4  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3]); break;
        case 5  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4]); break;
        case 6  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5]); break;
        case 7  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6]); break;
        case 8  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7]); break;
        case 9  : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8]); break;
        case 10 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9]); break;
        case 11 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10]); break;
        case 12 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11]); break;
        case 13 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12]); break;
        case 14 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13]); break;
        case 15 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14]); break;
        case 16 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14], ar[15]); break;
        case 17 : met.invoke(obj, Qt::DirectConnection, ret, ar[0], ar[1], ar[2], ar[3], ar[4], ar[5], ar[6], ar[7], ar[8], ar[9], ar[10], ar[11], ar[12], ar[13], ar[14], ar[15], ar[16]); break;
        default: qCritical("Num arguments is > 17"); exit(333);
        }
    }
#endif

    //qDebug() << "converting and returning result...";
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

/*
void APythonInterface::initialize()
{
    Py_Initialize();

    PyObject * mainModule = PyImport_GetModuleDict();

    for (size_t iUnit = 0; iUnit < ModData.size(); iUnit++)
    {
        //qDebug() << "-->" << iUnit;
        strName = &ModData[iUnit]->StrName;
        evalScript( QString("import %1").arg(strName->data()) );

        PyObject * module = PyDict_GetItemString(mainModule, strName->data());
        //qDebug() << "  module:" << module;

        const int numMethods = ModData[iUnit]->Object->metaObject()->methodCount();
        //qDebug() << "  num meta methods" << numMethods;
        for (int iMet = 0; iMet < numMethods; iMet++)
        {
            QMetaMethod meta = ModData[iUnit]->Object->metaObject()->method(iMet);
            if ((meta.methodType() == QMetaMethod::Slot) && meta.access() == QMetaMethod::Public)
            {
                if (meta.name() == "deleteLater") continue;
                //qDebug() << "  ->" << meta.name() << " with meta index:" << iMet;

                PyObject * caller = PyTuple_Pack(2, PyLong_FromSize_t(iUnit), PyLong_FromLong(iMet));
                PyObject * pyFunc = PyCFunction_New(&baseMethodDef, caller);
                if (pyFunc == NULL) qDebug() << "PyCFunction is null!";
                int res = PyModule_AddObject(module, meta.name(), pyFunc);
                if (res < 0) qDebug() << "Failed to create object for method" << meta.name();
            }
        }
    }

    //evalScript( "def print(txt): core.print(str(txt))" );
    evalScript( "def print(*txt):\n s=''\n for t in txt: s+=str(t)+' '\n core.print(s)" );

    //qDebug() << "initialized finished\n";
}
*/

void APythonInterface::initialize()
{
    Py_Initialize();

    PyObject * mainModule = PyImport_GetModuleDict();

    for (size_t iUnit = 0; iUnit < ModData.size(); iUnit++)
    {
        strName = &ModData[iUnit]->StrName;
        //qDebug() << "-->" << QString(strName->data());
        evalScript( QString("import %1").arg(strName->data()) );

        PyObject * module = PyDict_GetItemString(mainModule, strName->data());

        const int numMethods = ModData[iUnit]->Object->metaObject()->methodCount();
        //qDebug() << "  num meta methods" << numMethods;
        QSet<int> AlreadyProcessedMethods;
        for (int iMet = 0; iMet < numMethods; iMet++)
        {
            if (AlreadyProcessedMethods.contains(iMet))
            {
                //qDebug() << "Already processed!";
                continue;
            }
            QMetaMethod meta = ModData[iUnit]->Object->metaObject()->method(iMet);
            if ((meta.methodType() == QMetaMethod::Slot) && meta.access() == QMetaMethod::Public)
            {
                if (meta.name() == "deleteLater") continue;
                //qDebug() << "  ->" << meta.name() << " with meta index:" << iMet;

                std::vector<int> sameNameMethods;
                collectOverloaded(iUnit, iMet, sameNameMethods);
                PyObject * caller;
                if (sameNameMethods.empty())
                {
                    caller = PyTuple_Pack(2, PyLong_FromSize_t(iUnit), PyLong_FromLong(iMet));
                }
                else
                {
                    caller = PyTuple_New(2 + sameNameMethods.size());
                    PyTuple_SetItem(caller, 0, PyLong_FromSize_t(iUnit));
                    PyTuple_SetItem(caller, 1, PyLong_FromLong(iMet));
                    for (size_t index = 0; index < sameNameMethods.size(); index++)
                    {
                        const int iOtherMethod = sameNameMethods[index];
                        PyTuple_SetItem(caller, 2 + index, PyLong_FromLong(iOtherMethod));
                        AlreadyProcessedMethods << iOtherMethod;
                    }
                    //qDebug() << PyTuple_Size(caller);
                }
                PyObject * pyFunc = PyCFunction_New(&baseMethodDef, caller);
                if (pyFunc == NULL) qDebug() << "PyCFunction is null!";
                int res = PyModule_AddObject(module, meta.name(), pyFunc);
                if (res < 0) qDebug() << "Failed to create object for method" << meta.name();
            }
        }
    }

    evalScript( "def print(*txt):\n s=''\n for t in txt: s+=str(t)+' '\n core.print(s)" );
}

void APythonInterface::collectOverloaded(int Unit, int Method, std::vector<int> & SameNameMethods)
{
    const int numMethods = ModData[Unit]->Object->metaObject()->methodCount();
    const QByteArray Name = ModData[Unit]->Object->metaObject()->method(Method).name();
    for (int iMet = Method + 1; iMet < numMethods; iMet++)
    {
        QMetaMethod meta = ModData[Unit]->Object->metaObject()->method(iMet);
        if (meta.methodType() == QMetaMethod::Slot && meta.access() == QMetaMethod::Public)
        {
            if (Name != meta.name()) continue;
            SameNameMethods.push_back(iMet);
        }
    }
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

    //qDebug() << "Running script:" << script;
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
    //qDebug() << "--> abort of Python Interface...";
    PyErr_SetInterrupt();
    //qDebug() << PyErr_SetInterruptEx(2);
}

PyObject * APythonInterface::getMainModule()
{
    PyObject * md = PyImport_GetModuleDict();    // borrowed
    return PyDict_GetItemString(md, "__main__"); // borrowed
}

void APythonInterface::handleError()
{
    //qDebug() << "-------------Python interpreter signals an error-----------";

    PyObject * pyErr = PyErr_Occurred();
    if (!pyErr)
    {
        ErrorDescription = "Error reported, by PyErr not generated!";
        return;
    }

    //qDebug() << QString::number(PY_VERSION_HEX, 16);

#if PY_VERSION_HEX < 0x030C0000
    PyObject * ptype;
    PyObject * pvalue;
    PyObject * ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);

    if (PyErr_GivenExceptionMatches(pyErr, PyExc_SyntaxError))
    {
        //qDebug() << "...syntax-related error";  //e.g. exception SyntaxError(message, details)

        PyObject * first = PyTuple_GetItem(pvalue, 0); // borrowed
        ErrorDescription = PyUnicode_AsUTF8(PyObject_Str(first));
        PyObject * second = PyTuple_GetItem(pvalue, 1); // borrowed
        ErrorLineNumber = PyLong_AsLong( PyTuple_GetItem(second, 1) );

        return;
    }

    if (ptraceback)
    {
        PyTracebackObject *tb_o = (PyTracebackObject *)ptraceback;
        ErrorLineNumber = tb_o->tb_lineno;
    }

    PyObject * repr = PyObject_Repr(pvalue);
    PyObject * str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char * bytes = PyBytes_AS_STRING(str);
    ErrorDescription = QString(bytes);
    Py_XDECREF(repr);
    Py_XDECREF(str);

#else
    // SyntaxError is special, doesn't have __traceback__ attribute
    if (PyErr_GivenExceptionMatches(pyErr, PyExc_SyntaxError))
    {
        //qDebug() << "...syntax-related error";
        PyObject * exc = PyErr_GetRaisedException();
        PyObject* linePy = PyObject_GetAttrString(exc,"lineno");
        if (linePy) ErrorLineNumber = PyLong_AsLong(linePy);
        PyObject* descPy = PyObject_GetAttrString(exc,"msg");
        //if (descPy) ErrorDescription = PyUnicode_AsUTF8(PyObject_Str(descPy));
        if (descPy) ErrorDescription = QString("(SyntaxError) %1").arg(PyUnicode_AsUTF8(PyObject_Str(descPy)));
        //ErrorDescription = PyUnicode_AsUTF8(PyObject_Str(exc));

        Py_XDECREF(descPy);
        Py_XDECREF(linePy);
        Py_XDECREF(exc);
        return;
    }

    PyObject* pException = PyErr_GetRaisedException();
    PyObject* pErrClass = PyObject_GetAttrString(PyObject_Type(pException), "__name__");
    const char* err_class = PyUnicode_AsUTF8(pErrClass);
    PyObject* pErrorString = PyObject_Str(pException); // just in case: In the case of syntax error, also contains line number.
    const char* err_msg = PyUnicode_AsUTF8(pErrorString);
    if (err_class && err_msg)
    {
        ErrorDescription = QString("(%1) %2").arg(err_class).arg(err_msg);
        //qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaa" <<err_class <<err_msg;
    }
    else
        ErrorDescription = "Unknown error";

    PyObject* pTraceback = PyObject_GetAttrString(pException,"__traceback__");
    PyObject* pErrLine = PyObject_GetAttrString(pTraceback,"tb_lineno");
    if (pErrLine)
    {
        ErrorLineNumber = PyLong_AsLong(pErrLine);
        //qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << PyLong_AsLong(pErrLine);
    }
    else ErrorLineNumber = -1;

    Py_XDECREF(pErrLine);
    Py_XDECREF(pTraceback);
    Py_XDECREF(pErrorString);
    Py_XDECREF(pErrClass);
    Py_XDECREF(pException);

    //PyErr_Fetch(&ptype, &pvalue, &ptraceback); // Deprecated since version 3.12: Use PyErr_GetRaisedException() instead.
    //PyErr_GetExcInfo(&ptype, &pvalue, &ptraceback); // no info in traceback
    /*
    PyObject * ptype;
    PyObject * pvalue;
    PyObject * ptraceback;

    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    if (ptraceback)
    {
        PyTracebackObject *tb_o = (PyTracebackObject *)ptraceback;
        ErrorLineNumber = tb_o->tb_lineno;
        //qDebug() << "bbbbbbbbbbbbbbbbbbbbbbbbbbb" << ErrorLineNumber;
    }

    PyObject * repr = PyObject_Repr(pvalue);
    PyObject * str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char * bytes = PyBytes_AS_STRING(str);
    ErrorDescription = QString(bytes);
    Py_XDECREF(repr);
    Py_XDECREF(str);
    */
#endif

    //qDebug() << ">>>>>>>" << ErrorDescription << ErrorLineNumber;
}
