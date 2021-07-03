#ifndef A3SCRIPTMANAGER_H
#define A3SCRIPTMANAGER_H

#include <QObject>
#include <QJSValue>

class A3ScriptRes;
class QThread;
class A3ScriptWorker;

class A3ScriptManager : public QObject
{
    Q_OBJECT
public:
    A3ScriptManager(A3ScriptRes & ScrRes, QObject * parent = nullptr);
    ~A3ScriptManager();

    bool evaluate(const QString & script);
    bool isRunning() const;
    void abort();

    QJSValue getResult();

    int getErrorLineNumber(); //-1 if not error state
    bool getError(QString & errorString, int & lineNumber, QString & errorFileName); // false if busy or no error //***!!! handle interrupted

public slots:
    void start();

private slots:
    void evalFinished(bool flag);

signals:
    void finished(bool bSuccess);
    void doEval(const QString & script);
    void doExit();

protected:
    A3ScriptRes & ScrRes;
    QThread * Thread = nullptr;
    A3ScriptWorker * Worker = nullptr;

};

#endif // A3SCRIPTMANAGER_H
