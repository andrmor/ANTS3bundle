#include "alocalnormalsampler.h"
#include "asurfacesettings.h"
#include "arandomhub.h"

#include <QDebug>

#include <cmath>

#include "TVector3.h"

ALocalNormalSampler::ALocalNormalSampler(const ASurfaceSettings & settings) :
    Settings(settings), RandomHub(ARandomHub::getInstance()) {}

// input:
// globalNormal    - double[3]
// photonDirection - double[3]
// output:
// localNormal     - double[3]
void ALocalNormalSampler::getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal)
{
    qDebug() << "globNorm:" << globalNormal[0] << ' ' << globalNormal[1] << ' ' << globalNormal[2];
    qDebug() << "photDir:"  << photonDirection[0] << ' ' << photonDirection[1] << ' ' << photonDirection[2];

    switch (Settings.Model)
    {
    case ASurfaceSettings::Polished :
        // for Polished, it should not be called at all!
        for (int i = 0; i < 3; i++) localNormal[i] = globalNormal[i];
        break;
    case ASurfaceSettings::GaussSimplistic :
        {
            TVector3 gn(globalNormal);
            TVector3 ort = gn.Orthogonal();

            double scal = 0;
            do
            {
                TVector3 vec(gn);

                double rand = RandomHub.gauss(0, 15.0);
                vec.Rotate(rand * 3.1415926/180.0, ort);
                vec.Rotate(RandomHub.uniform() * 2.0*3.1415926, gn);

                scal = 0;
                for (int i = 0; i < 3; i++)
                {
                    localNormal[i] = vec[i];
                    scal += localNormal[i] * photonDirection[i];
                }
                qDebug() << "nk" << scal;
            }
            while (scal < 0);

            break;
        }
//    case ASurfaceSettings::Model3 :
//        {

//            break;
//        }
    default:;
    }

    qDebug() << "localNorm:"  << localNormal[0] << ' ' << localNormal[1] << ' ' << localNormal[2];
}
