#include "aphoton.h"

#include "TRandom2.h"
#include "TMath.h"

APhoton::APhoton() {}

APhoton::APhoton(double *pos, double *dir, int waveIndex, double time) :
    time(time), waveIndex(waveIndex), scint_type(0)
{
    r[0] = pos[0]; r[1] = pos[1]; r[2] = pos[2];
    v[0] = dir[0]; v[1] = dir[1]; v[2] = dir[2];
}

void APhoton::copyFrom(const APhoton *Another)
{
    r[0] = Another->r[0];
    r[1] = Another->r[1];
    r[2] = Another->r[2];

    v[0] = Another->v[0];
    v[1] = Another->v[1];
    v[2] = Another->v[2];

    time = Another->time;
    waveIndex = Another->waveIndex;
    scint_type = Another->scint_type;

    SimStat = Another->SimStat;
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
        v[0] = 0; v[1] = 0; v[2] = 1.0;
    }
}

void APhoton::randomDir(TRandom2 *RandGen)
{
    //Sphere function of Root:
    double a = 0, b = 0, r2 = 1.0;
    while (r2 > 0.25)
    {
        a  = RandGen->Rndm() - 0.5;
        b  = RandGen->Rndm() - 0.5;
        r2 =  a*a + b*b;
    }
    v[2] = ( -1.0 + 8.0 * r2 );
    double scale = 8.0 * TMath::Sqrt(0.25 - r2);
    v[0] = a * scale;
    v[1] = b * scale;
}
