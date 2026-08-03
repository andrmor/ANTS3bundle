#include "as2generator.h"
#include "aphotontracer.h"
#include "aphoton.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "amaterialhub.h"
#include "amaterial.h"
#include "ageometryhub.h"
#include "adeporecord.h"
#include "aphotongenerator.h"
#include "alightsensorevent.h"

#include <QDebug>

#include "TGeoManager.h"

AS2Generator::AS2Generator(APhotonGenerator & photonGenerator, APhotonTracer & photonTracer, ALightSensorEvent &event) :
    PhotonGenerator (photonGenerator),
    PhotonTracer(photonTracer),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    MatHub(AMaterialHub::getConstInstance()),
    GeoManager(AGeometryHub::getInstance().GeoManager),
    Event(event) {}

void AS2Generator::generate(ADepoRecord & rec) //uses MW->EnergyVector as the input parameter
{
    GeoManager->SetCurrentPoint(rec.Pos.data());
    GeoManager->FindNode();

    double meanElectrons = 0;
    const double & W = MatHub[rec.MatIndex]->W;
    if (W > 0) meanElectrons = rec.Energy / W;

    int numElectrons = PhotonGenerator.sampleFromMean(meanElectrons, 1.0);
    if (numElectrons > 0)
    {
        double time = rec.Time;
        bool foundSecScint = doDrift(time); // updates time!
        if (foundSecScint) generateLight(numElectrons, rec.Pos.data(), time);
    }
}

bool AS2Generator::doDrift(double & time)
{
    //field is assumed in z direction, electrons drift upward!

    DiffusionRecords.clear();
    GeoManager->SetCurrentDirection(0, 0, 1.0);

    const char * marker = GeoManager->GetCurrentVolume()->GetTitle();
    while (marker[0] != '2')
    {
        //drifting up until entered secondary scintillator (marker[0] == '2') or the position is outside of the defined geometry
        const int thisMatIndex = GeoManager->GetCurrentVolume()->GetMaterial()->GetIndex();

        GeoManager->FindNextBoundaryAndStep();
        if (GeoManager->IsOutside()) return false;

        const double step = GeoManager->GetStep();
        const double driftSpeed = MatHub.getDriftSpeed(thisMatIndex);
        if (driftSpeed != 0)
        {
            time += step / driftSpeed;

            const double sigmaTime       = MatHub.getDiffusionSigmaTime(thisMatIndex, step);
            const double sigmaTransverse = MatHub.getDiffusionSigmaTransverse(thisMatIndex, step);

            if (sigmaTime != 0 || sigmaTransverse != 0)
                DiffusionRecords.push_back( DiffSigmas(sigmaTime, sigmaTransverse) );
        }

        marker = GeoManager->GetCurrentVolume()->GetTitle();
    }
    return true;
}

void AS2Generator::generateLight(int numElectrons, double * xyPosition, double time)
{
    const int    matIndexSecScint = GeoManager->GetCurrentVolume()->GetMaterial()->GetIndex();
    const double photonsPerElectron = MatHub[matIndexSecScint]->SecScintPhotonYield;
    const double Zstart = GeoManager->GetCurrentPoint()[2];

    GeoManager->FindNextBoundary();
    const double Zspan = GeoManager->GetStep();

    //generate photons
    if (DiffusionRecords.empty() || SimSet.OptSet.TracingMode == APhotOptSettings::LRF)
    {
        double meanPhotons = numElectrons * photonsPerElectron;
        int    numPhotons = PhotonGenerator.sampleFromMean(meanPhotons, 1.0);
        if (numPhotons > 0)
        {
            if (SimSet.OptSet.TracingMode == APhotOptSettings::LRF)
                Event.generateHitsForLrfMode(numPhotons, xyPosition);
            else
                generateAndTracePhotons(xyPosition, time, numPhotons, matIndexSecScint, Zstart, Zspan);
        }
    }
    else
    {
        //diffusion is in effect
        for (int iElectron = 0; iElectron < numElectrons; iElectron++)
        {
            double pos[3];
            pos[0] = xyPosition[0];
            pos[1] = xyPosition[1];
            pos[2] = Zstart + 0.5*Zspan; // try to be inside the SecScint in Z
            bool bInside = true;
            for (const DiffSigmas & rec : DiffusionRecords)
            {
                time += RandomHub.gauss(0, rec.sigmaTime);
                pos[0] += RandomHub.gauss(0, rec.sigmaX);
                pos[1] += RandomHub.gauss(0, rec.sigmaX);

                GeoManager->SetCurrentPoint(pos);
                GeoManager->FindNode();

                const char * marker = GeoManager->GetCurrentVolume()->GetTitle();
                if (marker[0] != '2')
                {
                    bInside = false;
                    break;
                }
            }
            if (!bInside)
            {
                //qDebug() << "Left SecScint during diffusion";
                continue;
            }

            double meanPhotons = photonsPerElectron;
            int    numPhotons = PhotonGenerator.sampleFromMean(meanPhotons, 1.0);
            if (numPhotons > 0)
            {
                if (SimSet.OptSet.TracingMode == APhotOptSettings::LRF)
                    Event.generateHitsForLrfMode(numPhotons, xyPosition);
                else
                    generateAndTracePhotons(pos, time, numPhotons, matIndexSecScint, Zstart, Zspan);
            }
        }
    }
}

void AS2Generator::generateAndTracePhotons(double * Position, double Time, int NumPhotonsToGenerate, int MatIndexSecScint, double Zstart, double Zspan)
{
    APhoton photon;
    photon.r[0] = Position[0];
    photon.r[1] = Position[1];
    photon.SecondaryScint = true;

    const double DriftSpeed = MatHub.getDriftSpeed(MatIndexSecScint);
    for (int iPhoton = 0; iPhoton < NumPhotonsToGenerate; iPhoton++)
    {
        //random z inside secondary scintillator
        const double z = Zspan * RandomHub.uniform();
        photon.r[2] = Zstart + z;
        photon.time = Time + z / DriftSpeed;

        PhotonGenerator.generateDirection(photon);
        PhotonGenerator.generateWave(photon, MatIndexSecScint);
        PhotonGenerator.generateTime(photon, MatIndexSecScint);

        PhotonTracer.tracePhoton(photon);
    }
}
