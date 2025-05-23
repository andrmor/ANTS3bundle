#include "arootobjcollection.h"
#include "arootobjbase.h"

ARootObjCollection::~ARootObjCollection()
{
    //clear();
    // clear is called from AScriptObjStore: at the moment Trees.clear() causes crash on exit of ANTS3
}

bool ARootObjCollection::append(const QString &name, ARootObjBase* record, bool bAbortIfExists)
{
    QMutexLocker locker(&Mutex);

    if (Collection.contains(name))
    {
        if (bAbortIfExists) return false;
        delete Collection[name];
        Collection[name] = record;
    }
    else
        Collection.insert(name, record);

    return true;
}

ARootObjBase *ARootObjCollection::getRecord(const QString &name)
{
    QMutexLocker locker(&Mutex);
    return Collection.value(name, 0);
}

bool ARootObjCollection::remove(const QString &name)
{
    QMutexLocker locker(&Mutex);

    if (!Collection.contains(name)) return false;

    delete Collection[name];
    Collection.remove(name);

    return true;
}

void ARootObjCollection::clear()
{
    QMutexLocker locker(&Mutex);
    QMapIterator<QString, ARootObjBase*> iter(Collection);
    while (iter.hasNext())
    {
        iter.next();
        delete iter.value();
    }
    Collection.clear();
}

const QStringList ARootObjCollection::getAllRecordNames() const
{
    QMutexLocker locker(&Mutex);

    QStringList sl;
    QMapIterator<QString, ARootObjBase*> iter(Collection);
    while (iter.hasNext())
    {
        iter.next();
        sl << iter.key();
    }

    sl.sort();
    return sl;
}
