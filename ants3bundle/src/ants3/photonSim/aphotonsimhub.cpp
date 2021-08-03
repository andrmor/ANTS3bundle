#include "aphotonsimhub.h"
#include "ajsontools.h"

APhotonSimHub & APhotonSimHub::getInstance()
{
    static APhotonSimHub instance;
    return instance;
}

const APhotonSimHub &APhotonSimHub::getConstInstance()
{
    return getInstance();
}

void APhotonSimHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;
        Settings.writeToJson(js);
    json["PhotonSim"] = js;
}

QString APhotonSimHub::readFromJson(const QJsonObject & json)
{
    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonSim", js);
    if (!ok) return "json does not contain photon sim settings!";

    QString err = Settings.readFromJson(js);
    if (!err.isEmpty()) return err;
    emit settingsChanged();
    return "";
}
