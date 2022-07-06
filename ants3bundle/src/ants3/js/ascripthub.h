#ifndef ASCRIPTHUB_H
#define ASCRIPTHUB_H

#include <QObject>
#include <QString>

class AJScriptManager;
class AScriptInterface;

#ifdef ANTS3_PYTHON
    class APythonScriptManager;
#endif

class AScriptHub : public QObject
{
    Q_OBJECT

public:
    static AScriptHub      & getInstance();
    static AJScriptManager & manager(); // !!!*** to kill

    static void              abort(const QString & message); // !!!*** to kill, use JS or Python independently

    AJScriptManager        & getJScriptManager() {return *JavaScriptM;}
#ifdef ANTS3_PYTHON
    APythonScriptManager   & getPythonManager()  {return *PythonM;}
#endif

    void addCommonInterface(AScriptInterface * interface, QString name);
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
    AJScriptManager      * JavaScriptM = nullptr;
#ifdef ANTS3_PYTHON
    APythonScriptManager * PythonM = nullptr;
#endif
};

#endif // ASCRIPTHUB_H
