#ifndef A3SCRIPTWORKER_H
#define A3SCRIPTWORKER_H

#include <QObject>
#include <QJSValue>

class QJSEngine;
class QString;
class ADispatcherInterface;

class A3ScriptWorker : public QObject
{
    Q_OBJECT
public:
    A3ScriptWorker() {}
    ~A3ScriptWorker();

    bool isBusy() const {return bBusy;}
    void abort();

    QJSValue getResult() const {return Result;}
    bool getError(QString & errorString, int & lineNumber, QString & errorFileName);
    int getErrorLineNumber();

public slots:
    void initialize();
    void evaluate(const QString & script);
    void exit();

signals:
    void evalFinished(bool bSuccess);
    void stopped();

protected:
    ADispatcherInterface * Disp = nullptr;

    QJSEngine * Engine = nullptr;
    bool bBusy = false;
    QJSValue Result;

};

#endif // A3SCRIPTWORKER_H
