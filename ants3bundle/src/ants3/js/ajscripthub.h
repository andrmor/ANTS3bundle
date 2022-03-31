#ifndef AJSCRIPTHUB_H
#define AJSCRIPTHUB_H

#include <QObject>
#include <QString>

class AJScriptManager;
class AScriptInterface;

class AJScriptHub : public QObject
{
    Q_OBJECT

public:
    static AJScriptHub &     getInstance();
    static AJScriptManager & manager();
    static void              abort(const QString & message);

    AJScriptManager & getJScriptManager() {return *SM;}

    void addInterface(AScriptInterface * interface, QString name);

private:
    AJScriptHub();
    ~AJScriptHub();

    AJScriptHub(const AJScriptHub&)            = delete;
    AJScriptHub(AJScriptHub&&)                 = delete;
    AJScriptHub& operator=(const AJScriptHub&) = delete;
    AJScriptHub& operator=(AJScriptHub&&)      = delete;

signals:
    //for gui
    void outputText(QString);
    void outputHtml(QString);
    void showAbortMessage(QString message);
    void clearOutput();
    void requestUpdateGui();

private:
    AJScriptManager * SM = nullptr;
};

#endif // AJSCRIPTHUB_H
