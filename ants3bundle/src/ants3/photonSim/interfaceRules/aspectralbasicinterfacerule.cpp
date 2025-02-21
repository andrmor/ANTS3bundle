#include "aspectralbasicinterfacerule.h"
#include "aphoton.h"
#include "aphotonsimhub.h"
#include "aphotonstatistics.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QDebug>

ASpectralBasicInterfaceRule::ASpectralBasicInterfaceRule(int MatFrom, int MatTo)
    : ABasicInterfaceRule(MatFrom, MatTo), WaveSet(APhotonSimHub::getConstInstance().Settings.WaveSet)
{
    Wave     = {200, 800};
    ProbLoss = {0,   1.0};
    ProbRef  = {1.0, 0};
    ProbDiff = {0,   0};
}

AInterfaceRule::EInterfaceRuleResult ASpectralBasicInterfaceRule::calculate(APhoton *Photon, const double *NormalVector)
{
    int waveIndex = Photon->waveIndex;
    if (!WaveSet.Enabled || waveIndex == -1) waveIndex = effectiveWaveIndex; //guard: if not resolved, script ovberride can in principle assign index != -1

    Abs = ProbLossBinned[waveIndex];
    Scat = ProbDiffBinned[waveIndex];
    Spec  = ProbRefBinned[waveIndex];

    return ABasicInterfaceRule::calculate(Photon, NormalVector);
}

QString ASpectralBasicInterfaceRule::getReportLine() const
{
    return QString("Spectral data with %1 points").arg(Wave.size());
}

QString ASpectralBasicInterfaceRule::getLongReportLine() const
{
    QString s = "--> Simplistic spectral <--\n";
    s += QString("Spectral data from %1 to %2 nm\n").arg(Wave.front()).arg(Wave.back());
    s += QString("Number of points: %1\n").arg(Wave.size());
    s += QString("Effective wavelength: %1").arg(effectiveWavelength);
    return s;
}

QString ASpectralBasicInterfaceRule::getDescription() const
{
    QString txt = "This interface defines three wavelength-dependent parameters:\n"
                  "1) Absorption - the probability the photon is killed\n"
                  "2) Specuar reflection - the probability of specular reflection (flat or rough surface: see below)\n"
                  "3) Scattering\n"
                  "    3a) Lambertian back in 2Pi\n"
                  "    3b) Lambertian forward in 2Pi\n"
                  "    3c) Isotropic scattering in 4Pi\n"
                  "All values must be in the range from 0 to 1\n"
                  "If the sum does not ammounts to 1, the remaining fraction is the \"normal\" Freshnel/Snell physics\n"
                  "\n"
                  "Data are to be loaded from a text file, with every line containing 4 numbers:\n"
                  "wavelength[nm] absorption_prob[0..1] reflection_prob[0..1] scattering_prob[0..1]\n"
                  "\n"
                  "The rough surface settings only affect specular reflection!";
    return txt;
}

void ASpectralBasicInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    json["ScatMode"] = ScatterModel;
    json["EffWavelength"] = effectiveWavelength;

    if (Wave.size() != ProbLoss.size() || Wave.size() != ProbRef.size() || Wave.size() != ProbDiff.size())
    {
        qWarning() << "Mismatch in data size for SpectralBasicOverride! skipping data!";
        return;
    }
    QJsonArray sp;
    for (size_t i = 0; i < Wave.size(); i++)
    {
        QJsonArray ar;
        ar << Wave.at(i) << ProbLoss.at(i) << ProbRef.at(i) << ProbDiff.at(i);
        sp << ar;
    }
    json["Data"] = sp;
}

bool ASpectralBasicInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    if ( !jstools::parseJson(json, "ScatMode", ScatterModel) ) return false;
    if ( !jstools::parseJson(json, "EffWavelength", effectiveWavelength) ) return false;

    //after constructor vectors are not empty!
    Wave.clear();
    ProbLoss.clear();
    ProbRef.clear();
    ProbDiff.clear();

    QJsonArray sp;
    if ( !jstools::parseJson(json, "Data", sp) ) return false;
    size_t size = sp.size();
    if (size == 0) return false;

    Wave.    resize(size);
    ProbLoss.resize(size);
    ProbRef. resize(size);
    ProbDiff.resize(size);

    for (size_t i = 0; i < size; i++)
    {
        QJsonArray ar = sp[i].toArray();
        if (ar.size() != 4) return false;

        Wave[i]     = ar[0].toDouble();
        ProbLoss[i] = ar[1].toDouble();
        ProbRef[i]  = ar[2].toDouble();
        ProbDiff[i] = ar[3].toDouble();
    }
    return true;
}

void ASpectralBasicInterfaceRule::initializeWaveResolved()
{
    if (WaveSet.Enabled)
    {
        WaveSet.toStandardBins(Wave, ProbLoss, ProbLossBinned);
        WaveSet.toStandardBins(Wave, ProbRef,  ProbRefBinned);
        WaveSet.toStandardBins(Wave, ProbDiff, ProbDiffBinned);

        effectiveWaveIndex = WaveSet.toIndex(effectiveWavelength);
    }
    else
    {
        size_t isize = Wave.size();
        size_t i = 0;
        if (isize != 1)  //esle use i = 0
        {
            for (; i < isize; i++)
                if (Wave[i] > effectiveWavelength) break;

            //closest is i-1 or i
            if (i != 0 && i != isize-1)
                if ( fabs(Wave[i-1] - effectiveWavelength) < fabs(Wave[i] - effectiveWavelength) ) i--;
        }

        //qDebug() << "Selected i = "<< i << "with wave"<<Wave.at(i) << Wave;
        effectiveWaveIndex = 0;
        ProbLossBinned = { ProbLoss[i] };
        ProbRefBinned  = { ProbRef[i] };
        ProbDiffBinned = { ProbDiff[i] };
        //qDebug() << "LossRefDiff"<<ProbLossBinned.at(effectiveWaveIndex)<<ProbRefBinned.at(effectiveWaveIndex)<<ProbDiffBinned.at(effectiveWaveIndex);
    }
}

#include "afiletools.h"
QString ASpectralBasicInterfaceRule::loadData(const QString & fileName)
{
    std::vector< std::vector<double>* > vec = { &Wave, &ProbLoss, &ProbRef, &ProbDiff};
    QString err = ftools::loadDoubleVectorsFromFile(fileName, vec);
    if (!err.isEmpty()) return err;

    for (size_t i = 0; i < Wave.size(); i++)
    {
        double sum = ProbLoss[i] + ProbRef[i] + ProbDiff[i];
        if (sum > 1.0) return QString("Sum of probabilities is larger than 1.0 for wavelength of ") + QString::number(Wave[i]) + " nm";
    }

    if (Wave.empty())
        return "No data were read from the file!";

    return "";
}

QString ASpectralBasicInterfaceRule::doCheckOverrideData()
{
    //checking spectrum
    if (Wave.size() == 0) return "Spectral data are not defined";
    if (Wave.size() != ProbLoss.size() || Wave.size() != ProbRef.size() || Wave.size() != ProbDiff.size()) return "Spectral data do not match in size";
    for (size_t i = 0; i < Wave.size(); i++)
    {
        if (Wave[i] < 0) return "Negative wavelength are not allowed";
        if (ProbLoss[i] < 0 || ProbLoss[i] > 1.0) return "Absorption probability has to be in the range of [0, 1.0]";
        if (ProbRef[i]  < 0 || ProbRef[i]  > 1.0) return "Reflection probability has to be in the range of [0, 1.0]";
        if (ProbDiff[i] < 0 || ProbDiff[i] > 1.0) return "Scattering probability has to be in the range of [0, 1.0]";
        double sum = ProbLoss[i] + ProbRef[i] + ProbDiff[i];
        if (sum > 1.0) return QString("Sum of probabilities is larger than 1.0 for wavelength of %1 nm").arg(Wave[i]);
    }
    if (ScatterModel < 0 || ScatterModel > 2) return "Unknown scattering option";

    return "";
}
