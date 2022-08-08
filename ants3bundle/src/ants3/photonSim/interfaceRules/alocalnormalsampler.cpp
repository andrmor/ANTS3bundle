#include "alocalnormalsampler.h"
#include "asurfacesettings.h"
#include "arandomhub.h"

ALocalNormalSampler::ALocalNormalSampler(ASurfaceSettings & settings) :
    Settings(settings), RandHub(ARandomHub::getInstance()) {}

void ALocalNormalSampler::getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal)
{
    // input:
    // globalNormal    - double[3]
    // photonDirection - double[3]
    // output:
    // localNormal     - double[3]

    switch (Settings.Model)
    {
    case ASurfaceSettings::Model1 :
        {
            // use Random.uniform() to get a random number with uniform distribution in the range [0, 1)
            break;
        }
    case ASurfaceSettings::Model2 :
        {

            break;
        }
    case ASurfaceSettings::Model3 :
        {

            break;
        }
    default:;
    }
}
