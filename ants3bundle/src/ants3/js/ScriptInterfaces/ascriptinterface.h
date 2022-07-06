#ifndef ASCRIPTINTERFACE_H
#define ASCRIPTINTERFACE_H

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

public slots:
    QString help() const;

protected:
    bool bAbortRequested = false; // each unit have to be aaware of this flag! Dispatcher-based tasks are aborter automatically!

    void abort(const QString & message);

private:
    QString NoHelp = "Help not provided";
};

#endif // ASCRIPTINTERFACE_H
