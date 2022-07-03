#ifndef APYTHONSCRIPTMANAGER_H
#define APYTHONSCRIPTMANAGER_H

#include "avirtualscriptmanager.h"

#include <QObject>
#include <QString>
#include <QJSValue>

#include <vector>

class AScriptInterface;
class APythonInterface;

class APythonScriptManager : public AVirtualScriptManager
{
    Q_OBJECT
public:
    APythonScriptManager(QObject * parent = nullptr);
    ~APythonScriptManager(){}

    void registerInterface(AScriptInterface * interface, QString name) override;
    const std::vector<AScriptInterface*> & getInterfaces() const override;

    bool evaluate(const QString & script) override;
    void abort() override {}  // to abort script use AJScriptHub::abort(message) !!!***

    bool isRunning() const override {return false;} // !!!***
    bool isAborted() const override {return bAborted;} // !!!***

    QVariant getResult() override;

    bool isError() const override;
    int  getErrorLineNumber() override; //-1 if no errors

    void collectGarbage() override {}

    void finalizeInit();

private:
    bool             bAborted = false;  // TODO !!!***

    APythonInterface * PyInterface = nullptr;

    std::vector<AScriptInterface*> Interfaces;

};

/*
private:
    void start();

private slots:
    void evalFinished(bool flag);

signals:
    void finished(bool bSuccess);
    void doEval(const QString & script);
    void doExit();
    void doRegisterInterface(AScriptInterface * interface, QString name);

protected:
    QThread        * Thread = nullptr;
    AJScriptWorker * Worker = nullptr;


public:
    //for minimizer
    QString          MiniFunctionName;
    int              MiniNumVariables  = 0;
    double           MiniBestResult    = 1e30;
*/

#endif // APYTHONSCRIPTMANAGER_H
