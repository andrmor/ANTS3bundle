#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"

void APFM_OpticalFiber::writeToJson(QJsonObject & json) const
{

}

void APFM_OpticalFiber::readFromJson(const QJsonObject & json)
{

}

bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target)
{
    photonData.LocalPosition[2] = 0;

    // check angle of incidence inside acceptance come
    // check absorption
    // time increase

    return true;
}
