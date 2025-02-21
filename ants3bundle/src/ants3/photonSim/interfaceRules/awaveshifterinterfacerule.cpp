#include "awaveshifterinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "arandomhub.h"
#include "aphotonstatistics.h"
#include "aphotonsimhub.h"
#include "ajsontools.h"
#include "afiletools.h"

#include <QJsonObject>
#include <QDebug>

#include "TMath.h"
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

        WaveSet.toStandardBins(ReemissionProbability, ReemissionProbabilityBinned);

        std::vector<double> emisSpecBinned;
        WaveSet.toStandardBins(EmissionSpectrum, emisSpecBinned);

        TString name = "WLSEmSpec";
        name += MatFrom;
        name += "to";
        name += MatTo;
        delete Spectrum; Spectrum = new TH1D(name, "", WaveSet.countNodes(), WaveSet.From, WaveSet.From + WaveSet.Step * WaveNodes);
        for (int j = 1; j < WaveSet.countNodes(); j++)  Spectrum->SetBinContent(j, emisSpecBinned[j-1]);
        Spectrum->GetIntegral(); //to make thread safe
    }
    else
    {
        ReemissionProbabilityBinned.clear();
        delete Spectrum; Spectrum = nullptr;
    }
}

#include "astatisticshub.h"
AInterfaceRule::EInterfaceRuleResult AWaveshifterInterfaceRule::calculate(APhoton *Photon, const double *NormalVector)
{
    //currently assuming there is no scattering on original wavelength - only reemission or absorption

    if ( !Spectrum ||                             // emission spectrum not defined
         Photon->waveIndex == -1 ||               // or photon without wavelength
         ReemissionProbabilityBinned.empty() )    // or probability not defined
    {
        Status = Absorption;
        return Absorbed;
    }

    double prob = ReemissionProbabilityBinned[Photon->waveIndex]; // probability of reemission
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
            wavelength = Spectrum->GetRandom(); // !!!*** can be faster if sample from waveIndex histogram
            waveIndex = WaveSet.toIndexFast(wavelength);
            if (!ConserveEnergy) break;
        }
        while (waveIndex < Photon->waveIndex); //conserving energy

        AStatisticsHub::getInstance().SimStat.WaveChanged++;
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
    QString s = QString("RProb %1 pts; Spectr %2 pts; Mod: ").arg(ReemissionProbability.size()).arg(EmissionSpectrum.size());
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
    s += QString("Reemission probaility from %1 to %2 nm\n").arg(ReemissionProbability.front().first).arg(ReemissionProbability.back().first);
    s += QString("Number of points: %1\n").arg(ReemissionProbability.size());
    s += QString("Emission spectrum from %1 to %2 nm\n").arg(EmissionSpectrum.front().first).arg(EmissionSpectrum.back().first);
    s += QString("Number of points: %1\n").arg(EmissionSpectrum.size());
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

QString AWaveshifterInterfaceRule::getDescription() const
{
    QString txt = "This rule is active only for wavelength-resolved simulations!\n\n"
        "Each incoming photon is first absorped.\n"
        "Then a check is made vs loaded reemision probability (wavelength-resolved).\n"
        "If reemitted, the photon wavelength is sampled from the provided emission spectrum.\n"
        "If ConserveEnergy is checked, the photon generation is attempted up to 10 times\n"
        "checking that the wavelength is longer than that for the absorped photon.\n"
        "If unseccessful, the photon is absorpbed.\n\n"
        "The direcion of the reemitted photon can be configured to be generated according to the\n"
        "isotropic (4Pi) or Lambertian distribution (back-scattering or forward).";
    return txt;
}

QString AWaveshifterInterfaceRule::loadReemissionProbability(const QString & fileName)
{
    QString err = ftools::loadPairs(fileName, ReemissionProbability, true);
    if (!err.isEmpty()) return err;

    for (const auto & pair : ReemissionProbability)
    {
        if (pair.second < 0 || pair.second > 1.0)
        {
            ReemissionProbability.clear();
            return "Reemission probability should be in the range from 0 to 1.0";
        }
    }
    return "";
}

QString AWaveshifterInterfaceRule::loadEmissionSpectrum(const QString & fileName)
{
    QString err = ftools::loadPairs(fileName, EmissionSpectrum, true);
    return err;
}

void AWaveshifterInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    QJsonArray arRP;
    jstools::writeDPairVectorToArray(ReemissionProbability, arRP);
    json["ReemissionProbability"] = arRP;

    QJsonArray arEm;
    jstools::writeDPairVectorToArray(EmissionSpectrum, arEm);
    json["EmissionSpectrum"] = arEm;
    json["ReemissionModel"] = ReemissionModel;

    json["ConserveEnergy"] = ConserveEnergy;
}

bool AWaveshifterInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    if ( !jstools::parseJson(json, "ReemissionModel", ReemissionModel) )
    {
        ReemissionModel = 1;
        qWarning() << "Load WLS optical override: ReemissionModel not given, assuming Lambert back";
    }

    QJsonArray arRP;
    if ( !jstools::parseJson(json, "ReemissionProbability", arRP) ) return false;
    if (arRP.isEmpty()) return false;
    if ( !jstools::readDPairVectorFromArray(arRP, ReemissionProbability) ) return false;

    QJsonArray arES;
    if ( !jstools::parseJson(json, "EmissionSpectrum", arES) ) return false;
    if (arES.isEmpty()) return false;
    if ( !jstools::readDPairVectorFromArray(arES, EmissionSpectrum) ) return false;

    jstools::parseJson(json, "ConserveEnergy", ConserveEnergy);

    return true;
}

QString AWaveshifterInterfaceRule::doCheckOverrideData()
{
    if (ReemissionModel < 0 || ReemissionModel > 2) return "Invalid reemission model";

    if (ReemissionProbability.empty()) return "Reemission probability not loaded";
    if (EmissionSpectrum.empty()) return "Emission spectrum not loaded";

    initializeWaveResolved();

    if (WaveSet.Enabled && Spectrum->ComputeIntegral() <= 0)
            return "Binned emission spectrum: integral should be > 0";
    return "";
}
