#ifndef AROOTHTTPSERVER_H
#define AROOTHTTPSERVER_H

#include <QObject>
#include <QString>

class THttpServer;
class TGeoManager;
class QJsonObject;

class ARootHttpServer : public QObject
{
    Q_OBJECT

public:
    static ARootHttpServer & getInstance();

private:
    ARootHttpServer(){}
    ~ARootHttpServer();

    ARootHttpServer(const ARootHttpServer&)            = delete;
    ARootHttpServer(ARootHttpServer&&)                 = delete;
    ARootHttpServer& operator=(const ARootHttpServer&) = delete;
    ARootHttpServer& operator=(ARootHttpServer&&)      = delete;

public:
    ARootHttpServer(unsigned int port, QString OptionalUrlJsRoot = "");

    bool start();
    void stop();

    bool isRunning() const;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    int     Port = 8080;
    //QString ExternalJSROOT = "https://root.cern.ch/js/latest/"; // "https://root.cern/js/5.9.0/";     //QString ExternalJSROOT = "https://root.cern/js/latest/";
    QString ExternalJSROOT = "https://root.cern.ch/js/6.3.4/";
    bool    Autostart = false;

public slots:
    void onNewGeoManagerCreated();

private:
    THttpServer * Server = nullptr;
    TGeoManager * RegisteredGeoManager = nullptr;

signals:
    void StatusChanged();
    void RootServerStarted();
};

#endif // AROOTHTTPSERVER_H
