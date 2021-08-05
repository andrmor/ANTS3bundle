#include "awaveshifterinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "arandomhub.h"
#include "asimulationstatistics.h"
#include "aphotonsimhub.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QDebug>

#include "TMath.h"
#include "TRandom2.h"
#include "TH1D.h"

AWaveshifterInterfaceRule::AWaveshifterInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo), WaveSet(APhotonSimHub::getConstInstance().Settings.WaveSet) {}

AWaveshifterInterfaceRule::~AWaveshifterInterfaceRule()
{
    delete Spectrum;
}

void AWaveshifterInterfaceRule::initializeWaveResolved()
{
    if (WaveSet.Enabled)
    {
        const int WaveNodes = WaveSet.countNodes();

        WaveSet.toStandardBins(&ReemissionProbability_lambda, &ReemissionProbability, &ReemissionProbabilityBinned);

        QVector<double> y;
        WaveSet.toStandardBins(&EmissionSpectrum_lambda, &EmissionSpectrum, &y);

        TString name = "WLSEmSpec";
        name += MatFrom;
        name += "to";
        name += MatTo;
        delete Spectrum; Spectrum = new TH1D(name, "", WaveSet.countNodes(), WaveSet.From, WaveSet.From + WaveSet.Step * WaveNodes);
        for (int j = 1; j<WaveNodes+1; j++)  Spectrum->SetBinContent(j, y[j-1]);
        Spectrum->GetIntegral(); //to make thread safe
    }
    else
    {
        ReemissionProbabilityBinned.clear();
        delete Spectrum; Spectrum = nullptr;
    }
}

AInterfaceRule::OpticalOverrideResultEnum AWaveshifterInterfaceRule::calculate(APhoton *Photon, const double *NormalVector)
{
    //currently assuming there is no scattering on original wavelength - only reemission or absorption

    if ( !Spectrum ||                               //emission spectrum not defined
         Photon->waveIndex == -1 ||                 //or photon without wavelength
         ReemissionProbabilityBinned.isEmpty() )    //or probability not defined
    {
        Status = Absorption;
        return Absorbed;
    }

    double prob = ReemissionProbabilityBinned.at(Photon->waveIndex); // probability of reemission
    if (RandomHub.uniform() < prob)
    {
        //triggered!

        //generating new wavelength and waveindex
        double wavelength;
        int waveIndex;
        int attempts = -1;
        do
        {
            attempts++;
            if (attempts > 9)
              {
                Status = Absorption;
                return Absorbed;
              }
            wavelength = Spectrum->GetRandom();
            waveIndex = WaveSet.toIndexFast(wavelength);
        }
        while (waveIndex < Photon->waveIndex); //conserving energy

        Photon->SimStat->wavelengthChanged++;
        Photon->waveIndex = waveIndex;

        if (ReemissionModel == 0)
        {
            Photon->generateRandomDir();
            //enering new volume or backscattering?
            //normal is in the positive direction in respect to the original direction!
            if (Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2] < 0)
              {
                // qDebug()<<"   scattering back";
                Status = LambertianReflection;
                return Back;
              }
            // qDebug()<<"   continuing to the next volume";
            Status = Transmission;
            return Forward;
        }

        double norm2 = 0;
        if (ReemissionModel == 1)
        {
            // qDebug()<<"2Pi lambertian scattering backward";
            do
            {
                Photon->generateRandomDir();
                Photon->v[0] -= NormalVector[0]; Photon->v[1] -= NormalVector[1]; Photon->v[2] -= NormalVector[2];
                norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
            }
            while (norm2 < 0.000001);

            double normInverted = 1.0/TMath::Sqrt(norm2);
            Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;
            Status = LambertianReflection;

            return Back;
        }

        // qDebug()<<"2Pi lambertian scattering forward";
        do
          {
            Photon->generateRandomDir();
            Photon->v[0] += NormalVector[0]; Photon->v[1] += NormalVector[1]; Photon->v[2] += NormalVector[2];
            norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
          }
        while (norm2 < 0.000001);

        double normInverted = 1.0/TMath::Sqrt(norm2);
        Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;
        Status = Transmission;
        return Forward;
    }

    // else absorption
    Status = Absorption;
    return Absorbed;
}

QString AWaveshifterInterfaceRule::getReportLine() const
{
    QString s = QString("CProb %1 pts; Spectr %2 pts; Mod: ").arg(ReemissionProbability_lambda.size()).arg(EmissionSpectrum_lambda.size());
    switch( ReemissionModel )
    {
    case 0:
        s += "Iso";
        break;
    case 1:
        s += "Lamb_B";
        break;
    case 2:
        s += "Lamb_F";
        break;
    }
    return s;
}

QString AWaveshifterInterfaceRule::getLongReportLine() const
{
    QString s = "--> Wavelength shifter <--\n";
    s += QString("Reemission probaility from %1 to %2 nm\n").arg(ReemissionProbability_lambda.first()).arg(ReemissionProbability_lambda.last());
    s += QString("Number of points: %1\n").arg(ReemissionProbability_lambda.size());
    s += QString("Emission spectrum from %1 to %2 nm\n").arg(EmissionSpectrum_lambda.first()).arg(EmissionSpectrum_lambda.last());
    s += QString("Number of points: %1\n").arg(EmissionSpectrum_lambda.size());
    s += "Reemission model: ";
    switch( ReemissionModel )
    {
    case 0:
        s += "isotropic";
        break;
    case 1:
        s += "Lambertian, back";
        break;
    case 2:
        s += "Lambertian, forward";
        break;
    }
    return s;
}

void AWaveshifterInterfaceRule::writeToJson(QJsonObject &json) const
{
    AInterfaceRule::writeToJson(json);
/*
    QJsonArray arRP;
    writeTwoQVectorsToJArray(ReemissionProbability_lambda, ReemissionProbability, arRP);
    json["ReemissionProbability"] = arRP;
    QJsonArray arEm;
    writeTwoQVectorsToJArray(EmissionSpectrum_lambda, EmissionSpectrum, arEm);
    json["EmissionSpectrum"] = arEm;
    json["ReemissionModel"] = ReemissionModel;
*/
}

bool AWaveshifterInterfaceRule::readFromJson(const QJsonObject &json)
{
    if ( !jstools::parseJson(json, "ReemissionModel", ReemissionModel) )
    {
        ReemissionModel = 1;
        qWarning() << "Load WLS optical override: ReemissionModel not given, assuming Lambert back";
    }
/*
    QJsonArray arRP;
    if ( !jstools::parseJson(json, "ReemissionProbability", arRP) ) return false;
    if (arRP.isEmpty()) return false;
    if ( !readTwoQVectorsFromJArray(arRP, ReemissionProbability_lambda, ReemissionProbability) ) return false;

    QJsonArray arES;
    if ( !jstools::parseJson(json, "EmissionSpectrum", arES) ) return false;
    if (arES.isEmpty()) return false;
    if ( !readTwoQVectorsFromJArray(arES, EmissionSpectrum_lambda, EmissionSpectrum) ) return false;
*/
    return true;
}

QString AWaveshifterInterfaceRule::checkOverrideData()
{
    if (ReemissionModel<0 || ReemissionModel>2) return "Invalid reemission model";

    if (ReemissionProbability_lambda.isEmpty())
        return "Reemission probability not loaded";
    if (ReemissionProbability_lambda.size() != ReemissionProbability.size())
        return "Mismatch in reemission probability data";

    if (EmissionSpectrum_lambda.isEmpty())
        return "Emission spectrum not loaded";
    if (EmissionSpectrum_lambda.size() != EmissionSpectrum.size())
        return "Mismatch in emission spectrum data";

    initializeWaveResolved();

    if (WaveSet.Enabled && Spectrum->ComputeIntegral() <= 0)
            return "Binned emission spectrum: integral should be > 0";
    return "";
}
