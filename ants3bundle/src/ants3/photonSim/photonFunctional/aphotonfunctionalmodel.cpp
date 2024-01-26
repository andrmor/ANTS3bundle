#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"

void APhotonFunctionalModel::writeToJson(QJsonObject & json) const
{
    json["Type"] = getType();

    QJsonObject js;
    writeSettingsToJson(js);
    json["Settings"] = js;
}

void APhotonFunctionalModel::readFromJson(const QJsonObject & json)
{
    // Type is external as it will be used to construct the model
    QJsonObject js;
    jstools::parseJson(json, "Settings", js);
    readSettingsFromJson(js);
}

// ---

void APFM_OpticalFiber::writeSettingsToJson(QJsonObject & json) const
{
    json["Length_mm"] = Length_mm;
    json["MaxAngle_deg"] = MaxAngle_deg;
}

void APFM_OpticalFiber::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Length_mm", Length_mm);
    jstools::parseJson(json, "MaxAngle_deg", MaxAngle_deg);
}

QString APFM_OpticalFiber::printSettingsToString() const
{
    return QString("L = %0 mm; MaxAngle = %1 deg").arg(Length_mm).arg(MaxAngle_deg);
}

bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target)
{
    // check angle of incidence inside acceptance come
    //if (photonData.LocalDirection[])

    // check absorption

    photonData.LocalPosition[2] = 0;

    // time increase

    return true;
}


