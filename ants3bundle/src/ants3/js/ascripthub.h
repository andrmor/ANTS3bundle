#ifndef ASCRIPTHUB_H
#define ASCRIPTHUB_H

#include <QObject>
#include <QString>

class AJScriptManager;
class APythonScriptManager;
class AScriptInterface;

class AScriptHub : public QObject
{
    Q_OBJECT

public:
    static AScriptHub      & getInstance();
    static AJScriptManager & manager(); // !!!*** to kill

    static void              abort(const QString & message);

    AJScriptManager        & getJScriptManager() {return *JSM;}
    APythonScriptManager   & getPythonManager()  {return *PyM;}

    void addInterface(AScriptInterface * interface, QString name);
    void finalizeInit(); // run when initialization is finished (all additional script units already registered)

private:
    AScriptHub();
    ~AScriptHub();

    AScriptHub(const AScriptHub&)            = delete;
    AScriptHub(AScriptHub&&)                 = delete;
    AScriptHub& operator=(const AScriptHub&) = delete;
    AScriptHub& operator=(AScriptHub&&)      = delete;

signals:
    //for gui
    void outputText(QString);
    void outputHtml(QString);
    void showAbortMessage(QString message);
    void clearOutput();
    void requestUpdateGui();

private:
    AJScriptManager      * JSM = nullptr;
    APythonScriptManager * PyM = nullptr;
};

#endif // ASCRIPTHUB_H
