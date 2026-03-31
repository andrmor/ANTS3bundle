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
    const double photonYield = MatHub.getS1PhotonYield(rec.MatIndex, rec.Particle);
    const double fanoS1      = MatHub.getS1FanoFactor(rec.MatIndex);

    double meanPhotons = rec.Energy * photonYield;
    //if (fanoS1 == 0)
    //{
    //    have to keep remainer?
    //}
    int numPhotons = PhotonGenerator.sampleFromMean(meanPhotons, fanoS1);

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
