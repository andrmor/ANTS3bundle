#ifndef AWEBSERVERINTERFACE_H
#define AWEBSERVERINTERFACE_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class AWebSocketServer;

class AWebServer_SI: public AScriptInterface
{
  Q_OBJECT

public:
    AWebServer_SI();
    ~AWebServer_SI() {}

    AScriptInterface * cloneBase() const override {return new AWebServer_SI();}

    //void abortRun() override;

public slots:
    void     sendText(QString message);
    void     sendFile(QString fileName);
    void     sendObject(QVariantMap object);
    void     sendObjectAsJSON(QVariantMap object);

    bool     isBufferEmpty();
    void     clearBuffer();

    QVariantMap getBufferAsObject() const;
    bool        saveBufferToFile(QString fileName);

    void     sendProgressReport(int percents);
    void     setAcceptExternalProgressReport(bool flag);

private:
    AWebSocketServer & Server;
};

#endif // AWEBSERVERINTERFACE_H
