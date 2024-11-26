#include "aspectralbasicinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "aphotonstatistics.h"
//#include "acommonfunctions.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QDebug>

#include "TMath.h"
#include "TRandom2.h"
#include "TH1D.h"

ASpectralBasicInterfaceRule::ASpectralBasicInterfaceRule(int MatFrom, int MatTo)
    : ABasicInterfaceRule(MatFrom, MatTo), WaveSet(APhotonSimHub::getConstInstance().Settings.WaveSet)
{
    Wave << 500;
    ProbLoss << 0;
    ProbRef << 0;
    ProbDiff << 0;
}

AInterfaceRule::OpticalOverrideResultEnum ASpectralBasicInterfaceRule::calculate(APhoton *Photon, const double *NormalVector)
{
    int waveIndex = Photon->waveIndex;
    if (!WaveSet.Enabled || waveIndex == -1) waveIndex = effectiveWaveIndex; //guard: if not resolved, script ovberride can in principle assign index != -1

    Abs = ProbLossBinned.at(waveIndex);
    Scat = ProbDiffBinned.at(waveIndex);
    Spec  = ProbRefBinned.at(waveIndex);

    return ABasicInterfaceRule::calculate(Photon, NormalVector);
}

QString ASpectralBasicInterfaceRule::getReportLine() const
{
    return QString("Spectral data with %1 points").arg(Wave.size());
}

QString ASpectralBasicInterfaceRule::getLongReportLine() const
{
    QString s = "--> Simplistic spectral <--\n";
    s += QString("Spectral data from %1 to %2 nm\n").arg(Wave.first()).arg(Wave.last());
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
    for (int i=0; i<Wave.size(); i++)
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
    if (sp.isEmpty()) return false;
    for (int i=0; i<sp.size(); i++)
    {
        QJsonArray ar = sp.at(i).toArray();
        if (ar.size() != 4) return false;
        Wave     << ar.at(0).toDouble();
        ProbLoss << ar.at(1).toDouble();
        ProbRef  << ar.at(2).toDouble();
        ProbDiff << ar.at(3).toDouble();
    }
    return true;
}

void ASpectralBasicInterfaceRule::initializeWaveResolved()
{
    if (WaveSet.Enabled)
    {
        WaveSet.toStandardBins(&Wave, &ProbLoss, &ProbLossBinned);
        WaveSet.toStandardBins(&Wave, &ProbRef, &ProbRefBinned);
        WaveSet.toStandardBins(&Wave, &ProbDiff, &ProbDiffBinned);

        effectiveWaveIndex = WaveSet.toIndex(effectiveWavelength);
    }
    else
    {
        int isize = Wave.size();
        int i = 0;
        if (isize != 1)  //esle use i = 0
        {
            for (; i < isize; i++)
                if (Wave.at(i) > effectiveWavelength) break;

            //closest is i-1 or i
            if (i != 0)
                if ( fabs(Wave.at(i-1) - effectiveWavelength) < fabs(Wave.at(i) - effectiveWavelength) ) i--;
        }

        //qDebug() << "Selected i = "<< i << "with wave"<<Wave.at(i) << Wave;
        effectiveWaveIndex = 0;
        ProbLossBinned = QVector<double>(1, ProbLoss.at(i));
        ProbRefBinned = QVector<double>(1, ProbRef.at(i));
        ProbDiffBinned = QVector<double>(1, ProbDiff.at(i));
        //qDebug() << "LossRefDiff"<<ProbLossBinned.at(effectiveWaveIndex)<<ProbRefBinned.at(effectiveWaveIndex)<<ProbDiffBinned.at(effectiveWaveIndex);
    }
}

#include "afiletools.h"
QString ASpectralBasicInterfaceRule::loadData(const QString &fileName)
{
    /*
    QVector< QVector<double>* > vec;
    vec << &Wave << &ProbLoss << &ProbRef << &ProbDiff;
    QString err = LoadDoubleVectorsFromFile(fileName, vec);
    if (!err.isEmpty()) return err;

    for (int i=0; i<Wave.size(); i++)
    {
        double sum = ProbLoss.at(i) + ProbRef.at(i) + ProbDiff.at(i);
        if (sum > 1.0) return QString("Sum of probabilities is larger than 1.0 for wavelength of ") + QString::number(Wave.at(i)) + " nm";
    }

    if (Wave.isEmpty())
        return "No data were read from the file!";
*/
    return "";
}

QString ASpectralBasicInterfaceRule::doCheckOverrideData()
{
    //checking spectrum
    if (Wave.size() == 0) return "Spectral data are not defined";
    if (Wave.size() != ProbLoss.size() || Wave.size() != ProbRef.size() || Wave.size() != ProbDiff.size()) return "Spectral data do not match in size";
    for (int i=0; i<Wave.size(); i++)
    {
        if (Wave.at(i) < 0) return "negative wavelength are not allowed";
        if (ProbLoss.at(i) < 0 || ProbLoss.at(i) > 1.0) return "absorption probability has to be in the range of [0, 1.0]";
        if (ProbDiff.at(i) < 0 || ProbDiff.at(i) > 1.0) return "scattering probability has to be in the range of [0, 1.0]";
        if (ProbRef.at(i) < 0 || ProbRef.at(i) > 1.0) return "scattering probability has to be in the range of [0, 1.0]";
        double sum = ProbLoss.at(i) + ProbRef.at(i) + ProbDiff.at(i);
        if (sum > 1.0) return QString("Sum of probabilities is larger than 1.0 for wavelength of %1 nm").arg(Wave.at(i));
    }
    if (ScatterModel < 0 || ScatterModel > 2) return "unknown scattering model";

    return "";
}
