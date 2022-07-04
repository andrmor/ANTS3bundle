#ifndef APYTHONWORKER_H
#define APYTHONWORKER_H

#include <QObject>
#include <QString>

class APythonInterface;
class AScriptInterface;

class APythonWorker : public QObject
{
    Q_OBJECT
public:
    explicit APythonWorker(QObject *parent = nullptr);
    ~APythonWorker();

    bool isBusy() const {return bBusy;}
    void abort();

    bool    isError() const;
    QString getErrorDescription() const;
    int     getErrorLineNumber() const;

    const std::vector<AScriptInterface*> & getInterfaces() const {return Interfaces;}

//    bool     testMinimizationFunction(const QString & name) const;
//    double   runMinimizationFunction(const QString & name, const double * p, int numParameters);

public slots:
    void initialize();
    void onRegisterInterface(AScriptInterface * interface, QString name);
    void onFinalizeInit();
    void evaluate(const QString & script);
    void exit();  // !!!***

signals:
    void evalFinished(bool bSuccess);
    void stopped();

protected:
    APythonInterface * PyInterface = nullptr;
    bool               bBusy       = false;

    std::vector<AScriptInterface*> Interfaces;

};

#endif // APYTHONWORKER_H
