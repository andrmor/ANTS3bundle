#include "as1generator.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "aphotontracer.h"
#include "aphoton.h"
#include "aphotongenerator.h"
#include "amaterialhub.h"

#include <QDebug>

AS1Generator::AS1Generator(APhotonTracer & photonTracer) :
    PhotonTracer(photonTracer),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    MatHub(AMaterialHub::getConstInstance()) {}

bool AS1Generator::generate(ADepoRecord & rec)
{
    const double PhotonYield = MatHub.getS1PhotonYield(rec.MatIndex, rec.Particle);
    const double EnergyRes   = MatHub.getS1IntrEnRes  (rec.MatIndex, rec.Particle);

    double Photons;
    if (EnergyRes == 0)
        Photons = rec.Energy * PhotonYield + Remainer;
    else
    {
        const double mean  =  rec.Energy * PhotonYield + Remainer;
        const double sigma = EnergyRes * mean / 2.35482;
        Photons = RandomHub.gauss(mean, sigma);
    }
    Photons += Remainer; Remainer = 0;

    int NumPhotons = (int)Photons;
    Remainer = Photons - NumPhotons;

    APhoton Photon;
    for (int i = 0; i < 3; i++) Photon.r[i] = rec.Pos[i];
    Photon.time = rec.Time;  // can be adjusted by PhotonGenerator!

    for (int iPhot = 0; iPhot < NumPhotons; iPhot++)
    {
        Photon.generateRandomDir();
        APhotonGenerator::generateWave(Photon, rec.MatIndex);
        APhotonGenerator::generateTime(Photon, rec.MatIndex);

        PhotonTracer.tracePhoton(Photon);
    }

    return true;
}
