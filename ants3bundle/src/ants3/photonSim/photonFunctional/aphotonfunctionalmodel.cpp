#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"

bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, const AGeoObject &trigger, const AGeoObject &target)
{
    photonData.LocalPosition[2] = 0;

    // check angle of incidence inside acceptance come
    // check absorption
    // time increase

    return true;
}
