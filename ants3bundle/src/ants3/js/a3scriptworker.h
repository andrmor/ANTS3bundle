#ifndef A3SCRIPTWORKER_H
#define A3SCRIPTWORKER_H

#include "a3scriptres.h"

#include <QObject>
#include <QJSValue>

class QJSEngine;
class QString;
class A3DispInterface;

class A3ScriptWorker : public QObject
{
    Q_OBJECT
public:
    A3ScriptWorker(A3ScriptRes & ScrRes) : ScrRes(ScrRes){}
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
    A3ScriptRes & ScrRes;
    A3DispInterface * Disp = nullptr;

    QJSEngine * Engine = nullptr;
    bool bBusy = false;
    QJSValue Result;

};

#endif // A3SCRIPTWORKER_H
