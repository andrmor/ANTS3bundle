#ifndef APYTHONSCRIPTMANAGER_H
#define APYTHONSCRIPTMANAGER_H

#include "avirtualscriptmanager.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <vector>

class AScriptInterface;
class APythonWorker;
class QThread;

class APythonScriptManager : public AVirtualScriptManager
{
    Q_OBJECT
public:
    APythonScriptManager(QObject * parent = nullptr);
    ~APythonScriptManager();

    void registerInterface(AScriptInterface * interface, QString name) override;
    const std::vector<AScriptInterface*> & getInterfaces() const override;

    bool evaluate(const QString & script) override;
    void abort() override;

    bool isRunning() const override;
    bool isAborted() const override {return bAborted;}

    QVariant getResult() override;

    bool    isError() const override;
    QString getErrorDescription() const override;
    int     getErrorLineNumber() override; //-1 if no errors

    void collectGarbage() override {}

    void finalizeInit();

    bool   testMinimizationFunction();
    double runMinimizationFunction(const double * p);

    bool   isCallable(const QString & name) const;
    bool   callFunctionNoArguments(const QString & name);

    QString getVersion();

    void   checkSignals();

private slots:
    void evalFinished(bool flag);

signals:
    void finished(bool bSuccess);
    void doEval(const QString & script);
    void doExit();
    void doRegisterInterface(AScriptInterface * interface, QString name);
    void doFinalizeInit();

protected:
    QThread       * Thread = nullptr;
    APythonWorker * Worker = nullptr;
    bool            bAborted = false;
};

#endif // APYTHONSCRIPTMANAGER_H
