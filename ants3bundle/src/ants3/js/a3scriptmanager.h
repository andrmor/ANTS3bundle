#ifndef A3SCRIPTMANAGER_H
#define A3SCRIPTMANAGER_H

#include <QObject>
#include <QString>
#include <QJSValue>

class QThread;
class A3ScriptWorker;
class AScriptInterface;

class A3ScriptManager : public QObject
{
    Q_OBJECT

public:
    A3ScriptManager(QObject * parent = nullptr);
    ~A3ScriptManager();

    void registerInterface(AScriptInterface * interface, QString name);

    bool evaluate(const QString & script);
    bool isRunning() const;
    void abort();

    QJSValue getResult();

    int getErrorLineNumber(); //-1 if not error state
    bool getError(QString & errorString, int & lineNumber, QString & errorFileName); // false if busy or no error //***!!! handle interrupted

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
    A3ScriptWorker * Worker = nullptr;

};

#endif // A3SCRIPTMANAGER_H
