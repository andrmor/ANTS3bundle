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
    AJScriptWorker * Worker = nullptr;

};

#endif // AJSCRIPTMANAGER_H
