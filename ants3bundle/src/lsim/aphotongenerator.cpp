#include "aphotongenerator.h"
#include "aphoton.h"
#include "amaterialhub.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"

#include <QDebug>

#include "TH1D.h"

void APhotonGenerator::init()
{
    const APhotonSimSettings     & SimSet       = APhotonSimHub::getConstInstance().Settings;
    const APhGenOverrideSettings & PhGenOverSet = SimSet.PhGenOverrideSet;

    // Direction inits
    APhoton photon;
    photon.v[0] = PhGenOverSet.DirDX;
    photon.v[1] = PhGenOverSet.DirDY;
    photon.v[2] = PhGenOverSet.DirDZ;
    photon.ensureUnitaryLength();
    ColDirUnitary = TVector3(photon.v);
    CosConeAngle = cos(PhGenOverSet.ConeAngle * TMath::Pi() / 180.0);

    // Waveindex init
    FixedWaveIndex = SimSet.WaveSet.toIndex(PhGenOverSet.FixedWavelength);
}

void APhotonGenerator::generateDirection(APhoton & photon)
{
    const APhotonSimSettings     & SimSet       = APhotonSimHub::getConstInstance().Settings;
    const APhGenOverrideSettings & PhGenOverSet = SimSet.PhGenOverrideSet;
    ARandomHub                   & RandomHub    = ARandomHub::getInstance();

    if      (PhGenOverSet.DirectionMode == APhGenOverrideSettings::Isotropic)
        photon.generateRandomDir();
    else if (PhGenOverSet.DirectionMode == APhGenOverrideSettings::Cone)
    {
        const double z = CosConeAngle + RandomHub.uniform() * (1.0 - CosConeAngle);
        const double tmp = sqrt(1.0 - z*z);
        const double phi = RandomHub.uniform() * 2.0 * TMath::Pi();
        TVector3 K1(tmp*cos(phi), tmp*sin(phi), z);
        K1.RotateUz(ColDirUnitary);
        for (int i = 0; i < 3; i++) photon.v[i] = K1[i];
    }
    else
    {
        for (size_t i = 0; i < 3; i++)
            photon.v[i] = ColDirUnitary[i];
    }
}

void APhotonGenerator::generateWave(APhoton & Photon, int iMaterial)
{
    const AMaterialHub           & MaterialHub  = AMaterialHub::getConstInstance();
    const APhotonSimSettings     & SimSet       = APhotonSimHub::getConstInstance().Settings;
    const APhGenOverrideSettings & PhGenOverSet = SimSet.PhGenOverrideSet;

    const AMaterial * Material = MaterialHub[iMaterial];

    if (!SimSet.WaveSet.Enabled)
        Photon.waveIndex = -1;
    else if (PhGenOverSet.bFixWave)
        Photon.waveIndex = FixedWaveIndex;
    else
    {
        if (Photon.SecondaryScint)
        {
            if (SimSet.WaveSet.Enabled && Material->_SecondarySpectrumHist)
            {
                double wavelength = Material->_SecondarySpectrumHist->GetRandom();
                Photon.waveIndex = SimSet.WaveSet.toIndexFast(wavelength);
                //  qDebug()<<"sec! lambda "<<wavelength<<" index:"<<Photon.waveIndex;
            }
        }
        else
        {
            if (Material->_PrimarySpectrumHist)
            {
                double wavelength = Material->_PrimarySpectrumHist->GetRandom();
                Photon.waveIndex = SimSet.WaveSet.toIndexFast(wavelength);
                //  qDebug()<<"prim! lambda "<<wavelength<<" index:"<<Photon.waveIndex;
            }
        }
    }
}

void APhotonGenerator::generateTime(APhoton & Photon, int iMaterial)
{
    ARandomHub                   & RandomHub    = ARandomHub::getInstance();
    const APhotonSimSettings     & SimSet       = APhotonSimHub::getConstInstance().Settings;
    const APhGenOverrideSettings & PhGenOverSet = SimSet.PhGenOverrideSet;
    const AMaterialHub           & MaterialHub  = AMaterialHub::getConstInstance();

    const AMaterial * Material = MaterialHub[iMaterial];

    if (PhGenOverSet.bFixDecay)
        Photon.time += RandomHub.exp(PhGenOverSet.DecayTime);
    else
    {
        if (!Photon.SecondaryScint) //primary scintillation
            Photon.time += Material->generatePrimScintTime(RandomHub);
        else //secondary scintillation
            Photon.time += RandomHub.exp(Material->SecScintDecayTime);
    }

    //  qDebug()<<"Final time"<<Photon->time;
}
