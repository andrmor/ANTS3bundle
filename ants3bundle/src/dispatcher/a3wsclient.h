#ifndef A3WSCLIENT_H
#define A3WSCLIENT_H

#include "a3workdistrconfig.h"

#include <QObject>
#include <QString>

#include <vector>

class A3WorkNodeConfig;
class AWebSocketSession;

class A3WSClient : public QObject
{
    Q_OBJECT

public:
    A3WSClient(const A3WorkNodeConfig & Node, const QString & Command, const QString & ExchangeDir, const std::vector<QString> & CommonFiles);
    ~A3WSClient();

    QString ErrorString;

public slots:
    bool start();

private slots:
    void onWorkFinished(QString message);

signals:
    void finished();
    void progressReceived(int eventsDone);
    void remoteWorkFinished(QString message);

protected:
    A3WorkNodeConfig     Node;
    QString              Command;
    QString              ExchangeDir;
    std::vector<QString> CommonFiles;

    AWebSocketSession  * Session = nullptr;

    void reportFinished(QString message = "");
};

#endif // A3WSCLIENT_H
