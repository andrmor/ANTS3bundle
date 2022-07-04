#ifndef AJSCRIPTMANAGER_H
#define AJSCRIPTMANAGER_H

#include "avirtualscriptmanager.h"

#include <QObject>
#include <QString>
#include <QJSValue>

#include <vector>

class QThread;
class AJScriptWorker;
class AScriptInterface;

class AJScriptManager : public AVirtualScriptManager
{
    Q_OBJECT

public:
    AJScriptManager(QObject * parent = nullptr);
    ~AJScriptManager();

    void registerInterface(AScriptInterface * interface, QString name) override;
    const std::vector<AScriptInterface*> & getInterfaces() const override;

    bool evaluate(const QString & script) override;
    void abort() override;  // to abort script use AJScriptHub::abort(message)

    bool isRunning() const override;
    bool isAborted() const override {return bAborted;}

    QVariant getResult() override;

    bool isError() const override;
    QString getErrorDescription() const override;
    int  getErrorLineNumber() override; //-1 if no errors

    bool   testMinimizationFunction();
    double runMinimizationFunction(const double * p);

    void collectGarbage() override;
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

    bool             bAborted = false;

public:
    //for minimizer
    QString          MiniFunctionName;
    int              MiniNumVariables  = 0;
    double           MiniBestResult    = 1e30;

};

#endif // AJSCRIPTMANAGER_H
