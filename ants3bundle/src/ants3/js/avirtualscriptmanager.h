#ifndef AVIRTUALSCRIPTMANAGER_H
#define AVIRTUALSCRIPTMANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>

class AScriptInterface;

class AVirtualScriptManager : public QObject
{
    Q_OBJECT
public:
    AVirtualScriptManager(QObject * parent) : QObject(parent) {}

    virtual void registerInterface(AScriptInterface * interface, QString name) = 0;
    virtual const std::vector<AScriptInterface*> & getInterfaces() const = 0;

    virtual bool evaluate(const QString & script) = 0;
    virtual void abort() = 0;  // to abort script use AJScriptHub::abort(message)

    virtual bool isRunning() const = 0;
    virtual bool isAborted() const = 0;

    virtual QVariant getResult() = 0;

    virtual bool isError() const = 0;
    virtual QString getErrorDescription() const = 0;
    virtual int  getErrorLineNumber() = 0; //-1 if no errors

    virtual void collectGarbage() = 0;
};

#endif // AVIRTUALSCRIPTMANAGER_H
