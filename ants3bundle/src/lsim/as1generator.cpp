#include "as1generator.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "aphotontracer.h"
#include "aphoton.h"
#include "aphotongenerator.h"
#include "amaterialhub.h"
#include "adeporecord.h"

#ifdef USE_MERCURY
#include "lrmodel.h"
#include "asensorhub.h"
#include "alightresponsehub.h"
#include "alightsensorevent.h"
#endif

#include <QDebug>

AS1Generator::AS1Generator(APhotonTracer & photonTracer, ALightSensorEvent & event) :
    PhotonTracer(photonTracer),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    MatHub(AMaterialHub::getConstInstance()),
    Event(event) {}

void AS1Generator::generate(ADepoRecord & rec)
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

#ifdef USE_MERCURY
    if (SimSet.OptSet.TracingMode == APhotOptSettings::LRF)
    {

        const int numSens = ASensorHub::getConstInstance().countSensors(); // !!!*** check existance of the model
        for (int iSens = 0; iSens < numSens; iSens++)
        {
            double meanSignal = ALightResponseHub::getInstance().Model->Eval(iSens, rec.Pos.data()) * NumPhotons / SimSet.OptSet.LRF_photonsPerNode;
            Event.PMhits[iSens] += RandomHub.poisson(meanSignal * SimSet.OptSet.LRF_photoElectrons);
        }
        return;
    }
#endif

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
}
