#ifndef ASCRIPTINTERFACE_H
#define ASCRIPTINTERFACE_H

#include "escriptlanguage.h"

#include <QObject>
#include <QString>
#include <map>

class AScriptInterface : public QObject
{
    Q_OBJECT

public:
    AScriptInterface() : QObject() {}
    virtual ~AScriptInterface(){}

    virtual bool beforeRun() {return true;}   // automatically called before script evaluation
    virtual bool afterRun()  {return true;}   // automatically called after  script evaluation
    virtual void abortRun()  {}

    virtual AScriptInterface * cloneBase() const = 0;

//    virtual bool isMultithreadCapable() const {return false;}

    const QString & getMethodHelp(const QString & method) const;

    QString                    Name;
    QString                    Description;
    std::map<QString, QString> Help;
    std::map<QString, QString> DeprecatedMethods;
    std::map<QString, QString> RemovedMethods;

    bool bGuiThread      = true;

    EScriptLanguage Lang = EScriptLanguage::JavaScript;

public slots:
    QString help() const;

protected:
    bool bAbortRequested = false; // each unit have to be aware of this flag! Dispatcher-based tasks are aborted automatically!

    void abort(const QString & message);

private:
    QString NoHelp = "Description not provided";
};

#endif // ASCRIPTINTERFACE_H
