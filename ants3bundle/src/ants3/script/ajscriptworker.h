#ifndef AJSCRIPTWORKER_H
#define AJSCRIPTWORKER_H

#include <QObject>
#include <QJSValue>
#include <QString>

#include <vector>

class QJSEngine;
class QString;
class AScriptInterface;

class AJScriptWorker : public QObject
{
    Q_OBJECT

public:
    AJScriptWorker(){}
    ~AJScriptWorker();

    bool isBusy() const {return bBusy;}
    void abort();

    QJSValue getResult() const {return Result;}

    bool     isError() const;
    QString  getErrorDescription() const;
    int      getErrorLineNumber();

    const std::vector<AScriptInterface*> & getInterfaces() const {return Interfaces;}

    bool     isCallable(const QString & name) const;
    double   runMinimizationFunction(const QString & name, const double * p, int numParameters);

    bool     callFunctionNoArguments(const QString & functionName);

public slots:
    void initialize();
    void onRegisterInterface(AScriptInterface * interface, QString name);
    void evaluate(const QString & script);
    void exit();
    void onRequestGarbageCollection();

signals:
    void evalFinished(bool bSuccess);
    void stopped();

protected:
    QJSEngine            * Engine = nullptr;
    bool                   bBusy  = false;
    QJSValue               Result;

    std::vector<AScriptInterface*> Interfaces;

};

#endif // AJSCRIPTWORKER_H
