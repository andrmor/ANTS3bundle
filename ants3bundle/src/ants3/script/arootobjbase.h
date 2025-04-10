#ifndef AROOTOBJBASE_H
#define AROOTOBJBASE_H

#include <QString>
#include <QMutex>

class TObject;

class ARootObjBase
{
public:
    ARootObjBase(TObject * object, const QString &  title, const QString & type);
    virtual ~ARootObjBase();

    virtual TObject * GetObject();

    void              externalLock();
    void              externalUnlock();

    const QString   & getType() const;
    const QString   & getTitle() const;

    enum EStatus {OK, NotApplicable, DataMimatch};

protected:
    TObject * Object = nullptr;
    QString   Title;
    QString   Type;                    // object type according to ROOT (e.g. "TH1D")     // !!!*** change to enum

    mutable QMutex Mutex;
};

#endif // AROOTOBJBASE_H
