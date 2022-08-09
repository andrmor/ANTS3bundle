#include "alocalnormalsampler.h"
#include "asurfacesettings.h"
#include "arandomhub.h"

#include <QDebug>

#include <cmath>

ALocalNormalSampler::ALocalNormalSampler(const ASurfaceSettings & settings) :
    Settings(settings), RandomHub(ARandomHub::getInstance()) {}

#include "TVector3.h"
void ALocalNormalSampler::getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal)
{
    // input:
    // globalNormal    - double[3]
    // photonDirection - double[3]
    // output:
    // localNormal     - double[3]

    qDebug() << "globNorm:" << globalNormal[0] << ' ' << globalNormal[1] << ' ' << globalNormal[2];
    qDebug() << "photDir:"  << photonDirection[0] << ' ' << photonDirection[1] << ' ' << photonDirection[2];

    TVector3 gn(globalNormal);
    TVector3 ort = gn.Orthogonal();

    double scal = 0;
    do
    {
        /*
        //Sphere function of Root:
        double a = 0, b = 0, r2 = 1.0;
        while (r2 > 0.25)
        {
            a  = RandomHub.uniform() - 0.5;
            b  = RandomHub.uniform() - 0.5;
            r2 = a*a + b*b;
        }
        double scale = 8.0 * sqrt(0.25 - r2);
        localNormal[0] = a * scale;
        localNormal[1] = b * scale;
        localNormal[2] = ( -1.0 + 8.0 * r2 );
        */

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
