#include "aphotongenerator.h"
#include "aphoton.h"
#include "amaterialhub.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"

#include <QDebug>

#include "TH1D.h"

void APhotonGenerator::generateWave(APhoton & Photon, int iMaterial)
{
    const AMaterialHub & MaterialHub = AMaterialHub::getConstInstance();
    const APhotonSimSettings & SimSet = APhotonSimHub::getConstInstance().Settings;

    const AMaterial* Material = MaterialHub[iMaterial];

    if (!SimSet.WaveSet.Enabled)
        Photon.waveIndex = -1;
    else
    {
        if (Photon.SecondaryScint)
        {
            if (SimSet.WaveSet.Enabled && Material->SecondarySpectrumHist)
            {
                double wavelength = Material->SecondarySpectrumHist->GetRandom();
                Photon.waveIndex = SimSet.WaveSet.toIndexFast(wavelength);
                //  qDebug()<<"sec! lambda "<<wavelength<<" index:"<<Photon.waveIndex;
            }
        }
        else
        {
            if (Material->PrimarySpectrumHist)
            {
                double wavelength = Material->PrimarySpectrumHist->GetRandom();
                Photon.waveIndex = SimSet.WaveSet.toIndexFast(wavelength);
                //  qDebug()<<"prim! lambda "<<wavelength<<" index:"<<Photon.waveIndex;
            }
        }
    }
}

void APhotonGenerator::generateTime(APhoton & Photon, int iMaterial)
{
    ARandomHub & RandomHub = ARandomHub::getInstance();
    const AMaterialHub & MaterialHub = AMaterialHub::getConstInstance();
    const AMaterial * Material = MaterialHub[iMaterial];

    if (!Photon.SecondaryScint) //primary scintillation
        Photon.time += Material->generatePrimScintTime(RandomHub);
    else //secondary scintillation
        Photon.time += RandomHub.exp(Material->SecScintDecayTime);
    //  qDebug()<<"Final time"<<Photon->time;
}
