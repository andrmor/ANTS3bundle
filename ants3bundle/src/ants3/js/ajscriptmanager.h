#ifndef AJSCRIPTMANAGER_H
#define AJSCRIPTMANAGER_H

#include <QObject>
#include <QString>
#include <QJSValue>

class QThread;
class AJScriptWorker;
class AScriptInterface;

class AJScriptManager : public QObject
{
    Q_OBJECT

public:
    AJScriptManager(QObject * parent = nullptr);
    ~AJScriptManager();

    void registerInterface(AScriptInterface * interface, QString name);
    const std::vector<AScriptInterface*> & getInterfaces() const;

    bool evaluate(const QString & script);
    void abort();  // to abort script use AJScriptHub::abort(message)

    bool isRunning() const;
    bool isAborted() const {return bAborted;}

    QJSValue getResult();

    bool isError() const;
    bool getError(QString & errorString, int & lineNumber); // false if busy or no error //***!!! handle interrupted
    int  getErrorLineNumber(); //-1 if no errors

    bool   testMinimizationFunction();
    double runMinimizationFunction(const double * p);

    void collectGarbage();
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
