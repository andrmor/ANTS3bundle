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
    Settings.writeToJson(json);
}

QString APhotonSimHub::readFromJson(const QJsonObject & json)
{
    QString ErrorString = Settings.readFromJson(json);

    if (ErrorString.isEmpty()) emit settingsChanged();

    return ErrorString;
}
