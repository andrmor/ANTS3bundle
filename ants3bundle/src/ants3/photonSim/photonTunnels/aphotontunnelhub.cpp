#include "aphotontunnelhub.h"
#include "ageometryhub.h"
#include "aerrorhub.h"
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

QString APhotonTunnelHub::readFromJson(const QJsonObject & json)
{
    clearAllConnections();
    //clear models

    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonTunnels", js);
    if (!ok) return ""; // cannot enforce, some old configs do not have this field

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

    return "";
}

bool APhotonTunnelHub::isValidConnection(const ATunnelRecord & rec, bool registerError) const
{
    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    if (rec.From < 0)
    {
        if (registerError) AErrorHub::addError("Bad 'From' index in photon tunnel record");
        return false;
    }
    if (rec.From >= GeoHub.PhotonTunnelsIn.size())
    {
        if (registerError) AErrorHub::addError("Bad 'From' index in photon tunnel record");
        return false;
    }

    if (rec.To < 0)
    {
        if (registerError) AErrorHub::addError("Bad 'To' index in photon tunnel record");
        return false;
    }
    if (rec.To >= GeoHub.PhotonTunnelsOut.size())
    {
        if (registerError) AErrorHub::addError("Bad 'To' index in photon tunnel record");
        return false;
    }

    // !!!*** check model
    return true;
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

bool APhotonTunnelHub::updateRuntimeProperties()
{
    RuntimeData.clear();

    for (const ATunnelRecord & rec : Connections)
    {
        bool ok = isValidConnection(rec, true);
        if (!ok) return false;
    }

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();
    int numEntrances = GeoHub.PhotonTunnelsIn.size();
    int numExits = GeoHub.PhotonTunnelsOut.size();

    int numNotConnectedEntrances = 0;
    int numMultipleEntrances = 0;
    for (int i = 0; i < numEntrances; i++)
    {
        int seenTimes = 0;
        for (const ATunnelRecord & rec : Connections)
            if (rec.From == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedEntrances++;
        if (seenTimes > 1) numMultipleEntrances++;
    }

    int numNotConnectedExits = 0;
    int numMultipleExits = 0;
    for (int i = 0; i < numExits; i++)
    {
        int seenTimes = 0;
        for (const ATunnelRecord & rec : Connections)
            if (rec.To == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedExits++;
        if (seenTimes > 1) numMultipleExits++;
    }

    if (numNotConnectedEntrances > 0)
    {
        AErrorHub::addError("There are not connected photon tunnel entrances!");
        return false;
    }

    if (numNotConnectedExits > 0)
    {
        AErrorHub::addError("There are not connected photon tunnel exits!");
        return false;
    }

    if (numMultipleExits > 0)
    {
        AErrorHub::addError("A photon tunnel leading to multiple exits was found: not yet implemented");
        return false;
    }

    // updating
    RuntimeData.resize(GeoHub.PhotonTunnelsIn.size());
    for (const ATunnelRecord & rec : Connections)
    {
        RuntimeData[rec.From].ExitIndex = rec.To;
        // Model
    }
    return true;
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
