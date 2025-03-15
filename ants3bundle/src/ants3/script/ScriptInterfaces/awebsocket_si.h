#ifndef AWEBSOCKET_SI_H
#define AWEBSOCKET_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

class AWebSocketSession;

// !!!!! WARNING! !!!!!
// The same AWebSocketSession is used by the Dispatcher infrastructure!
// If modifications to the functionality have to be made, it is better to make a dedicated copy of the AWebSocketSession class for the script interface!
class AWebSocket_SI: public AScriptInterface
{
  Q_OBJECT

public:
    AWebSocket_SI();
    ~AWebSocket_SI();

    AScriptInterface * cloneBase() const override {return new AWebSocket_SI();}

    void abortRun() override;

public slots:    
    QString  connect(QString url, bool getAnswerOnConnection);
    void     disconnect();

    QString  sendText(QString message);
    QString  sendObject(QVariantMap object);
    QString  sendFile(QString fileName);

    QString  resumeWaitForAnswer();

    QVariantMap getBinaryReplyAsObject();
    bool        saveBinaryReplyToFile(QString fileName);

    void     setTimeout(int milliseconds);

signals:
    void showTextOnMessageWindow(const QString & text);
    void clearTextOnMessageWindow();

private:
    AWebSocketSession * Socket = nullptr;

    int TimeOut = 3000; //milliseconds

private:
    QString sendQJsonObject(const QJsonObject & json);
    //QString sendQByteArray(const QByteArray & ba);
};

#endif // AWEBSOCKET_SI_H
