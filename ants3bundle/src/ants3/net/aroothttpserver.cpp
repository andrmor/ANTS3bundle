#include "aroothttpserver.h"
#include "ajsontools.h"

#include <QDebug>

#include "THttpServer.h"
#include "TGeoManager.h"

ARootHttpServer & ARootHttpServer::getInstance()
{
    static ARootHttpServer instance;
    return instance;
}

bool ARootHttpServer::start()
{
#ifdef USE_ROOT_HTML
    delete Server;
    QString s = "http:" + QString::number(Port);
    Server = new THttpServer(s.toLatin1());
    if (!ExternalJSROOT.isEmpty())
    {
        Server->SetJSROOT(ExternalJSROOT.toLatin1());
        //Server->SetJSROOT("https://root.cern.ch/js/latest/");
    }

    //Server->SetDefaultPage("/home/andr/WORK/ANTS3/js/test.htm");
    Server->SetDefaultPage("/home/andr/WORK/ANTS3/js/index.htm");
    //Server->SetDefaultPage("/opt/root62802/js/files/online.htm");

    if (isRunning())
    {
        qDebug() << "ANTS3 root server is now listening";
        emit StatusChanged();
        //emit RootServerStarted(); //to update current geometry on the server
        onNewGeoManagerCreated();
        return true;
    }
    else
    {
        delete Server; Server = nullptr;
        qDebug() << "Root http server failed to start!\nCheck if another server is already listening at this port";
    }
#endif
    return false;
}

void ARootHttpServer::stop()
{
#ifdef USE_ROOT_HTML
    delete Server; Server = nullptr;
    qDebug() << "ANTS3 root http server has stopped listening";
    emit StatusChanged();
#endif
}

ARootHttpServer::~ARootHttpServer()
{
    delete Server;
}

bool ARootHttpServer::isRunning() const
{
    if (!Server) return false;
    return Server->IsAnyEngine();
}

void ARootHttpServer::writeToJson(QJsonObject & json) const
{
    json["Port"] = Port;
    json["ExternalJSROOT"] = ExternalJSROOT;
    json["Autostart"] = Autostart;
}

void ARootHttpServer::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Port", Port);
    jstools::parseJson(json, "ExternalJSROOT", ExternalJSROOT);
    jstools::parseJson(json, "Autostart", Autostart);
}

#include "ageometryhub.h"
void ARootHttpServer::onNewGeoManagerCreated()
{
#ifdef USE_ROOT_HTML
    if (!Server) return;

    if (RegisteredGeoManager)
    {
        qDebug() << "-----------------------Root html server: unregistering old GeoManager";
        Server->Unregister(RegisteredGeoManager);
    }
    qDebug() << "-----------------------Root html server: registering new GeoManager";
    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    GeoManager->SetName("world");
    Server->Register("GeoWorld", GeoManager);
    RegisteredGeoManager = GeoManager;

    Server->SetItemField("/","_drawitem","world");
#endif
}
