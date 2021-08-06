#include "aphoton.h"
#include "arandomhub.h"

#include "TMath.h"   // !!!*** remove?

APhoton::APhoton() {}

APhoton::APhoton(double * pos, double * dir, int waveIndex, double time) :
    time(time), waveIndex(waveIndex), SecondaryScint(false)
{
    for (int i=0; i<3; i++)
    {
        r[i] = pos[i];
        v[i] = dir[i];
    }
}

void APhoton::copyFrom(const APhoton *CopyFrom)
{
    for (int i=0; i<3; i++)
    {
        r[i] = CopyFrom->r[i];
        v[i] = CopyFrom->v[i];
    }

    time           = CopyFrom->time;
    waveIndex      = CopyFrom->waveIndex;
    SecondaryScint = CopyFrom->SecondaryScint;

    SimStat    = CopyFrom->SimStat;
}

void APhoton::ensureUnitaryLength()
{
    double mod = 0;
    for (int i=0; i<3; i++)
        mod += ( v[i] * v[i] );

    if (mod == 1.0) return;
    mod = TMath::Sqrt(mod);

    if (mod != 0)
        for (int i=0; i<3; i++) v[i] /= mod;
    else
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 1.0;
    }
}

void APhoton::generateRandomDir()
{
    ARandomHub & RandomHub = ARandomHub::getInstance();

    //Sphere function of Root:
    double a = 0, b = 0, r2 = 1.0;
    while (r2 > 0.25)
      {
        a  = RandomHub.uniform() - 0.5;
        b  = RandomHub.uniform() - 0.5;
        r2 =  a*a + b*b;
      }
    v[2] = ( -1.0 + 8.0 * r2 );
    double scale = 8.0 * TMath::Sqrt(0.25 - r2);
    v[0] = a*scale;
    v[1] = b*scale;
}
