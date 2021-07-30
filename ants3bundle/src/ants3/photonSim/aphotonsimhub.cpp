#include "aphotonsimhub.h"
#include "a3config.h"
#include "ajsontools.h"

// ---

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
    Settings.writeToJson(json);
}

void APhotonSimHub::readFromJson(const QJsonObject & json)
{
    Settings.readFromJson(json);
    emit settingsChanged();
}
