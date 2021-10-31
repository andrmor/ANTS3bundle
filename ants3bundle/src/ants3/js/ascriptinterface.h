#ifndef ASCRIPTINTERFACE_H
#define ASCRIPTINTERFACE_H

#include <QObject>
#include <QString>
#include <map>

class AScriptInterface : public QObject
{
    Q_OBJECT

public:
    AScriptInterface() : QObject() {}
    virtual ~AScriptInterface(){}

    virtual bool beforeRun() {return true;}   // automatically called before script evaluation
    virtual bool afterRun()  {return true;}   // automatically called after  script evaluation

    virtual void forceStop() {}               // called when abort was triggered

//    virtual bool isMultithreadCapable() const {return false;}

    QString                    Name;
    QString                    Description;
    std::map<QString, QString> Help;
    std::map<QString, QString> DeprecatedMethods;
    std::map<QString, QString> RemovedMethods;
//    bool GuiThread = true;

signals:
    void abort(QString);                      //abort request is automatically linked to abort slot of core unit
};

#endif // ASCRIPTINTERFACE_H
