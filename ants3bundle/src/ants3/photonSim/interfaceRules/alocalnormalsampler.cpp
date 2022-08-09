#include "alocalnormalsampler.h"
#include "asurfacesettings.h"
#include "arandomhub.h"

#include <QDebug>

ALocalNormalSampler::ALocalNormalSampler(const ASurfaceSettings & settings) :
    Settings(settings), RandHub(ARandomHub::getInstance()) {}

void ALocalNormalSampler::getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal)
{
    // input:
    // globalNormal    - double[3]
    // photonDirection - double[3]
    // output:
    // localNormal     - double[3]

    qDebug() << "globNorm:" << globalNormal[0] << ' ' << globalNormal[1] << ' ' << globalNormal[2];
    qDebug() << "photDir:"  << photonDirection[0] << ' ' << photonDirection[1] << ' ' << photonDirection[1];

    for (int i = 0; i < 3; i++) localNormal[i] = globalNormal[i];

    qDebug() << "localNorm:"  << localNormal[0] << ' ' << localNormal[1] << ' ' << localNormal[2];


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
