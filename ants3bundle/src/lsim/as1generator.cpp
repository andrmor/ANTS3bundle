#include "as1generator.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "aphotontracer.h"
#include "aphoton.h"
#include "aphotongenerator.h"
#include "amaterialhub.h"
#include "adeporecord.h"
#include "alightsensorevent.h"

#include <QDebug>

AS1Generator::AS1Generator(APhotonGenerator & photonGenerator, APhotonTracer & photonTracer, ALightSensorEvent & event) :
    PhotonGenerator(photonGenerator),
    PhotonTracer(photonTracer),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    MatHub(AMaterialHub::getConstInstance()),
    Event(event) {}

void AS1Generator::generate(ADepoRecord & rec)
{
    const double PhotonYield = MatHub.getS1PhotonYield(rec.MatIndex, rec.Particle);
    const double FanoS1      = MatHub.getS1FanoFactor(rec.MatIndex);

    double meanPhotons = rec.Energy * PhotonYield;
    int    numPhotons = 0;
    if (FanoS1 == 1.0)
    {
        if (meanPhotons > 25.0)
        {
            double sigma = std::sqrt(meanPhotons);
            numPhotons = int(RandomHub.gauss(meanPhotons, sigma) + 0.5);
        }
        else
            numPhotons = RandomHub.poisson(meanPhotons);
    }
    else if (FanoS1 == 0)
        numPhotons = int(meanPhotons + 0.5); // avoid! results in problems with events with many low energy deposition nodes
    else
    {
        if (meanPhotons > 25.0)
        {
            double sigma = std::sqrt(FanoS1 * meanPhotons);
            numPhotons = int(RandomHub.gauss(meanPhotons, sigma) + 0.5);
        }
        else
        {
            if (FanoS1 < 1.0)
            {
                double p = 1.0 - FanoS1;
                int n = int(meanPhotons / p + 0.5);                 // !!!*** what if meanPhotons/p < 0.5 ???
                double p_adj = ( n == 0 ? p : meanPhotons / n);     // still see above
                numPhotons = RandomHub.binomial(n, p_adj);
            }
            else
            {
                double p = 1.0 / FanoS1;
                double n = meanPhotons * p / (1 - p);
                numPhotons = RandomHub.negativeBinomial(n, p);
            }
        }
    }

    if (SimSet.OptSet.TracingMode == APhotOptSettings::LRF)
    {
        Event.generateHitsForLrfMode(numPhotons, rec.Pos.data());
        return;
    }

    APhoton photon;
    for (int i = 0; i < 3; i++) photon.r[i] = rec.Pos[i];
    photon.time = rec.Time;  // can be adjusted by PhotonGenerator!

    for (int iPhot = 0; iPhot < numPhotons; iPhot++)
    {
        PhotonGenerator.generateDirection(photon);
        PhotonGenerator.generateWave(photon, rec.MatIndex);
        PhotonGenerator.generateTime(photon, rec.MatIndex);

        PhotonTracer.tracePhoton(photon);
    }
}
