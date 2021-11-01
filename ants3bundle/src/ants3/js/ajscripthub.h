#ifndef AJSCRIPTHUB_H
#define AJSCRIPTHUB_H

#include <QObject>
#include <QString>

class AJScriptManager;

class AJScriptHub : public QObject
{
    Q_OBJECT

public:
    static AJScriptHub &     getInstance();
    static AJScriptManager & manager();
    static void              abort(const QString & message);

    AJScriptManager & getJScriptManager() {return *SM;}

signals:
    void showAbortMessage(QString message);

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
    void clearOutput();

private:
    AJScriptManager * SM = nullptr;
};

#endif // AJSCRIPTHUB_H
