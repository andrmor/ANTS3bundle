#ifndef A3WSCLIENT_H
#define A3WSCLIENT_H

#include "a3workdistrconfig.h"

#include <QObject>
#include <QString>
#include <QVector>

class A3WorkNodeConfig;
class AWebSocketSession;

class A3WSClient : public QObject
{
    Q_OBJECT

public:
    A3WSClient(const A3WorkNodeConfig & Node, const QString & Command, const QString & ExchangeDir, const QVector<QString> & CommonFiles);
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
    A3WorkNodeConfig Node;
    QString          Command;
    QString          ExchangeDir;
    QVector<QString> CommonFiles;

    AWebSocketSession * Session = nullptr;

};

#endif // A3WSCLIENT_H
