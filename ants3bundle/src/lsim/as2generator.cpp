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

#include <QDebug>

#include "TGeoManager.h"

AS2Generator::AS2Generator(APhotonTracer & photonTracer) :
    PhotonTracer(photonTracer),
    SimSet(APhotonSimHub::getConstInstance().Settings),
    RandomHub(ARandomHub::getInstance()),
    MatHub(AMaterialHub::getConstInstance()),
    GeoManager(AGeometryHub::getInstance().GeoManager) {}

// in ANTS2 PhotonRemainer resets on new particle, ElectronRemainer resets on new event   Need update? !!!***
void AS2Generator::generate(ADepoRecord & rec) //uses MW->EnergyVector as the input parameter
{
    GeoManager->SetCurrentPoint(rec.Pos.data());
    GeoManager->FindNode();

    const double & W = MatHub[rec.MatIndex]->W;
    double Electrons = ( W > 0 ? rec.Energy/W : 0 );
    Electrons += ElectronRemainer;

    NumElectrons = (int)Electrons;
    ElectronRemainer = Electrons - (double)NumElectrons;

    if (NumElectrons > 0)
    {
        double Time = rec.Time;
        bool foundSecScint = doDrift(Time); // updates Time!
        if (foundSecScint) generateLight(rec.Pos.data(), Time);
    }
}

bool AS2Generator::doDrift(double & time)
{
    //field is always in z direction, electrons drift up!
    DiffusionRecords.clear();
    GeoManager->SetCurrentDirection(0, 0, 1.0);  //up

    const char * marker = GeoManager->GetCurrentVolume()->GetTitle();
    while (marker[0] != '2')
    {
        //drifting up until entered secondary scintillator (marker[0] == '2') or the position is outside of the defined geometry
        const int ThisMatIndex = GeoManager->GetCurrentVolume()->GetMaterial()->GetIndex();

        GeoManager->FindNextBoundaryAndStep();
        if (GeoManager->IsOutside()) return false;

        const double Step = GeoManager->GetStep();
        const double DriftSpeed = MatHub.getDriftSpeed(ThisMatIndex);
        if (DriftSpeed != 0)
        {
            time += Step / DriftSpeed;

            const double SigmaTime       = MatHub.getDiffusionSigmaTime(ThisMatIndex, Step);
            const double SigmaTransverse = MatHub.getDiffusionSigmaTransverse(ThisMatIndex, Step);

            if (SigmaTime != 0 || SigmaTransverse != 0)
                DiffusionRecords.push_back( DiffSigmas(SigmaTime, SigmaTransverse) );
        }

        marker = GeoManager->GetCurrentVolume()->GetTitle();
    }
    return true;
}

void AS2Generator::generateLight(double * xyPosition, double time)
{
    const int    MatIndexSecScint = GeoManager->GetCurrentVolume()->GetMaterial()->GetIndex();
    const double PhotonsPerElectron = MatHub[MatIndexSecScint]->SecYield;
    const double Zstart = GeoManager->GetCurrentPoint()[2];

    GeoManager->FindNextBoundary();
    const double Zspan = GeoManager->GetStep();

    //generate photons
    if (DiffusionRecords.empty()) // || SimSet->fLRFsim)
    {
        const double Photons = NumElectrons * PhotonsPerElectron + PhotonRemainer;
        NumPhotons     = (int)Photons;
        PhotonRemainer = Photons - (double)NumPhotons;

        //if (PhotonGenerator->SimSet->fLRFsim) PhotonGenerator->GenerateSignalsForLrfMode(NumPhotons, DepoPosition, PhotonTracker->getEvent());
        //else
        generateAndTracePhotons(xyPosition, time, NumPhotons, MatIndexSecScint, Zstart, Zspan);
    }
    else
    {
        //diffusion is in effect
        for (int iElectron = 0; iElectron < NumElectrons; iElectron++)
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

            double Photons = PhotonsPerElectron + PhotonRemainer;
            int NumPhotonsThisEl = (int)Photons;
            PhotonRemainer = Photons - (double)NumPhotonsThisEl;
            NumPhotons += NumPhotonsThisEl;

            generateAndTracePhotons(pos, time, NumPhotonsThisEl, MatIndexSecScint, Zstart, Zspan);
        }
    }
}

void AS2Generator::generateAndTracePhotons(double * Position, double Time, int NumPhotonsToGenerate, int MatIndexSecScint, double Zstart, double Zspan)
{
    APhoton Photon;
    Photon.r[0] = Position[0];
    Photon.r[1] = Position[1];
    Photon.SecondaryScint = true;

    const double DriftSpeed = MatHub.getDriftSpeed(MatIndexSecScint);
    for (int iPhoton = 0; iPhoton < NumPhotonsToGenerate; iPhoton++)
    {
        //random z inside secondary scintillator
        const double z = Zspan * RandomHub.uniform();
        Photon.r[2] = Zstart + z;
        Photon.time = Time + z / DriftSpeed;

        Photon.generateRandomDir();
        APhotonGenerator::generateWave(Photon, MatIndexSecScint);
        APhotonGenerator::generateTime(Photon, MatIndexSecScint);

        PhotonTracer.tracePhoton(Photon);
    }
}
