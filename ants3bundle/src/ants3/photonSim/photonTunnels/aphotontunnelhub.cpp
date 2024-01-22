#include "aphotontunnelhub.h"
#include "ajsontools.h"

APhotonTunnelHub & APhotonTunnelHub::getInstance()
{
    static APhotonTunnelHub instance;
    return instance;
}

const APhotonTunnelHub & APhotonTunnelHub::getConstInstance()
{
    return getInstance();
}

void APhotonTunnelHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;

        // Models

        // Connections
        {
            QJsonArray ar;
            for (const ATunnelRecord & rec : Connections)
            {
                QJsonObject jse;
                    rec.writeToJson(jse);
                ar.push_back(jse);
            }
            js["Connections"] = ar;
        }

    json["PhotonTunnels"] = js;
}

void APhotonTunnelHub::readFromJson(const QJsonObject & json)
{
    clearAllConnections();

    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonTunnels", js);
    if (!ok) return;

    //Models

    // Connections
    {
        QJsonArray ar;
        jstools::parseJson(js, "Connections", ar);
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonObject jse = ar[i].toObject();
            ATunnelRecord rec;
                rec.readFromJson(jse);
            Connections.push_back(rec);
        }
    }
}

QString APhotonTunnelHub::addOrModifyConnection(int from, int to, int model, const QString & settings)
{
    for (ATunnelRecord & rec : Connections)
    {
        if (rec.To != to) continue;

        // !!!*** check model

        rec.From = from;
        rec.ModelIndex = model;
        rec.Settings = settings;
        return "";
    }

    Connections.push_back({from, to, model, settings});
    return "";
}

void APhotonTunnelHub::removeConnection(int from, int to)
{
    for (auto it = Connections.begin(); it < Connections.end(); ++it)
    {
        if (it->From != from) continue;
        if (it->To   != to)   continue;

        Connections.erase(it);
        return;
    }
}

APhotonTunnelHub::APhotonTunnelHub() {}

void ATunnelRecord::writeToJson(QJsonObject & json) const
{
    json["From"]       = From;
    json["To"]         = To;
    json["ModelIndex"] = ModelIndex;
    json["Settings"]   = Settings;
}

void ATunnelRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "From",       From);
    jstools::parseJson(json, "To",         To);
    jstools::parseJson(json, "ModelIndex", ModelIndex);
    jstools::parseJson(json, "Settings",   Settings);
}
