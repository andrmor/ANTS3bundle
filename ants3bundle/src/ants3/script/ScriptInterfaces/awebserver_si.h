#ifndef AWEBSERVER_SI_H
#define AWEBSERVER_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

class AWebSocketServer;

class AWebServer_SI: public AScriptInterface
{
  Q_OBJECT

public:
    AWebServer_SI();

    AScriptInterface * cloneBase() const override {return new AWebServer_SI();}

    //void abortRun() override;

public slots:
    void     sendText(QString message);
    void     sendFile(QString fileName);
    void     sendObject(QVariantMap object);

    bool     isBufferEmpty();
    void     clearBuffer();

    QVariantMap getBufferAsObject() const;
    bool        saveBufferToFile(QString fileName);

private:
    AWebSocketServer & Server;
};

#endif // AWEBSERVER_SI_H
