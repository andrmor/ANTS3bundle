#include "amaterial.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "aerrorhub.h"
#include "arandomhub.h"

#include <QDebug>
#include <QStandardPaths>
#include <QFile>

#include "TH1D.h"
#include "TRandom2.h"
#include "TGeoMaterial.h"

#include <cmath>

AMaterial::AMaterial()
{
    clear();
}

double AMaterial::getRefractiveIndex(int iWave) const
{
    if (iWave == -1 || _RefIndex_WaveBinned.empty()) return RefIndex;
    return _RefIndex_WaveBinned[iWave];
}

const std::complex<double> & AMaterial::getComplexRefractiveIndex(int iWave) const
{
    if (iWave == -1 || _RefIndex_Comlex_WaveBinned.empty()) return RefIndexComplex;
    return _RefIndex_Comlex_WaveBinned[iWave];
}

double AMaterial::getAbsorptionCoefficient(int iWave) const
{
    if (Dielectric)
    {
        //qDebug() << iWave << absWaveBinned.size();
        if (iWave == -1 || _AbsCoeff_WaveBinned.empty()) return AbsCoeff;
        return _AbsCoeff_WaveBinned[iWave];
    }
    else
    {
        return 1e50;
        //if (iWave == -1 || Abs_FromComplex_WaveBinned.empty()) return Abs_FromComplex;
        //return Abs_FromComplex_WaveBinned[iWave];
    }
}

double AMaterial::getReemissionProbability(int iWave) const
{
    //qDebug() << "reemis->" << iWave << ( reemissionProbBinned.size() > 0 ? reemissionProbBinned.at(iWave) : reemissionProb );
    if (iWave == -1 || _ReemissionProb_WaveBinned.empty()) return ReemissionProb;
    return _ReemissionProb_WaveBinned[iWave];
}

double AMaterial::getSpeedOfLight(int iWave) const
{
    double refIndexReal;
    if (Dielectric) refIndexReal = getRefractiveIndex(iWave);
    else
    {
        if (iWave == -1 || _RefIndex_Comlex_WaveBinned.empty()) refIndexReal = RefIndexComplex.real();
        else refIndexReal = _RefIndex_Comlex_WaveBinned[iWave].real();
    }

    return c_in_vac / refIndexReal;
}

void AMaterial::generateTGeoMat()
{
    _GeoMat = Composition.constructGeoMaterial(Name.toLatin1().data(), Density, Temperature);
}

double AMaterial::FT(double td, double tr, double t) const
{
    return 1.0 - ((tr + td) / td) * exp(- t / td) + (tr / td) * exp(- t * (1.0 / tr + 1.0 / td));
}

double single_exp(double t, double tau2)
{
    return exp(-1.0*t/tau2)/tau2;
}
double bi_exp(double t, double tau1,double tau2)
{
    return exp(-1.0*t/tau2)*(1.0-exp(-1.0*t/tau1))/tau2/tau2*(tau1+tau2);
}

double AMaterial::generatePrimScintTime(ARandomHub & Random) const
{
    double DecayTime = 0;
    if (PriScint_Decay.size() == 1) DecayTime = PriScint_Decay.front().first;
    else
    {
        //selecting decay time component
        const double generatedStatWeight = _PrimScintSumStatWeight_Decay * Random.uniform();
        double cumulativeStatWeight = 0;
        for (size_t i = 0; i < PriScint_Decay.size(); i++)
        {
            cumulativeStatWeight += PriScint_Decay[i].second;
            if (generatedStatWeight < cumulativeStatWeight)
            {
                DecayTime = PriScint_Decay[i].first;
                break;
            }
        }
    }

    if (DecayTime == 0)
        return 0; // decay time is 0 -> rise time is ignored

    double RiseTime = 0;
    if (PriScint_Raise.size() == 1) RiseTime = PriScint_Raise.front().first;
    else
    {
        //selecting raise time component
        const double generatedStatWeight = _PrimScintSumStatWeight__Raise * Random.uniform();
        double cumulativeStatWeight = 0;
        for (size_t i = 0; i < PriScint_Raise.size(); i++)
        {
            cumulativeStatWeight += PriScint_Raise[i].second;
            if (generatedStatWeight < cumulativeStatWeight)
            {
                RiseTime = PriScint_Raise[i].first;
                break;
            }
        }
    }

    if (RiseTime == 0)
        return Random.exp(DecayTime);

    double EmissionTime = 0;
    // From G4Scintillation of Geant4
    double d = (RiseTime + DecayTime) / DecayTime;
    while (true)
    {
        double ran1 = Random.uniform();
        double ran2 = Random.uniform();
        EmissionTime = -1.0 * DecayTime * log(1.0 - ran1);
        double gg = d * single_exp(EmissionTime, DecayTime);
        if (ran2 <= bi_exp(EmissionTime, RiseTime, DecayTime) / gg)
            break;
    }

    return EmissionTime;
}

#include "aphotonsimhub.h"
void AMaterial::updateRuntimeProperties()
{
    //RefIndex_Complex = {ReN, (ImN > 0 ? -ImN : ImN) };
    //if (!Dielectric) Abs_FromComplex = fabs(4.0 * 3.1415926535 * ImN / ComplexEffectiveWave * 1e6); // [mm-1]

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    const int WaveNodes = WaveSet.countNodes();
    _RefIndex_WaveBinned.clear();
    _AbsCoeff_WaveBinned.clear();
    _RefIndex_Comlex_WaveBinned.clear();
    _Rayleigh_WaveBinned.clear();
    _ReemissionProb_WaveBinned.clear();
    delete _PrimarySpectrumHist;   _PrimarySpectrumHist   = nullptr;
    delete _SecondarySpectrumHist; _SecondarySpectrumHist = nullptr;

    if (WaveSet.Enabled)
    {
        if (Dielectric)
        {
            if (!RefIndex_Wave.empty())
            {
                WaveSet.toStandardBins(RefIndex_Wave, _RefIndex_WaveBinned);
                for (double d : _RefIndex_WaveBinned) _RefIndex_Comlex_WaveBinned.push_back({d, 0}); // !!!*** still need?
            }
            if (!AbsCoeff_Wave.empty()) WaveSet.toStandardBins(AbsCoeff_Wave, _AbsCoeff_WaveBinned);
        }
        else
        {
            if (!RefIndexComplex_Wave.empty())
            {
                WaveSet.toStandardBins(RefIndexComplex_Wave, _RefIndex_Comlex_WaveBinned);
                for (auto & cri : _RefIndex_Comlex_WaveBinned)
                    if (cri.imag() > 0) cri = std::conj(cri);

                //for (size_t i = 0; i < RefIndex_Comlex_WaveBinned.size(); i++)
                //{
                //    const double wave = WaveSet.From + WaveSet.Step * i; // [nm]
                //    Abs_FromComplex_WaveBinned.push_back(  fabs(4.0 * 3.1415926535 * RefIndex_Comlex_WaveBinned[i].imag() / wave * 1e6) ); // [mm-1]);
                //}
            }
        }

        if (!ReemissionProb_Wave.empty())
            WaveSet.toStandardBins(ReemissionProb_Wave, _ReemissionProb_WaveBinned);

        if (RayleighMFP != 0)
        {
            double baseWave4 = RayleighWave * RayleighWave * RayleighWave * RayleighWave;
            double base = RayleighMFP / baseWave4;
            for (int i = 0; i < WaveNodes; i++)
            {
                double wave  = WaveSet.From + WaveSet.Step * i;
                double wave4 = wave * wave * wave * wave;
                _Rayleigh_WaveBinned.push_back(base * wave4);
            }
        }

        if (!PrimarySpectrum.empty())
        {
            delete _PrimarySpectrumHist; _PrimarySpectrumHist = new TH1D("", "Primary scintillation", WaveNodes, WaveSet.From, WaveSet.To);
            std::vector<double> y;
            WaveSet.toStandardBins(PrimarySpectrum, y);
            for (int j = 1; j < WaveNodes + 1; j++)  _PrimarySpectrumHist->SetBinContent(j, y[j-1]);
            _PrimarySpectrumHist->GetIntegral(); //to make thread safe
        }

        if (!SecondarySpectrum.empty())
        {
            delete _SecondarySpectrumHist; _SecondarySpectrumHist = new TH1D("","Secondary scintillation", WaveNodes, WaveSet.From, WaveSet.To);
            std::vector<double> y;
            WaveSet.toStandardBins(SecondarySpectrum, y);
            for (int j = 1; j<WaveNodes+1; j++)  _SecondarySpectrumHist->SetBinContent(j, y[j-1]);
            _SecondarySpectrumHist->GetIntegral(); //to make thread safe
        }
    }

    //updating sum stat weights for primary scintillation time generator
    _PrimScintSumStatWeight_Decay = 0;
    _PrimScintSumStatWeight__Raise = 0;
    for (const auto & pair : PriScint_Decay) _PrimScintSumStatWeight_Decay += pair.second;
    for (const auto & pair : PriScint_Raise) _PrimScintSumStatWeight__Raise += pair.second;
}

void AMaterial::clear()
{
    Name = "Undefined";
    Density = 1e-24;
    RefIndex = 1.0;
    AbsCoeff = RayleighMFP = ReemissionProb = 0;
    Temperature = 298.0;
    ElDriftVelocity = W = SecScintPhotonYield = SecScintDecayTime = ElDiffusionL = ElDiffusionT = 0;
    RayleighWave = 500.0;
    Comments = "";

    Dielectric = true;
    RefIndexComplex = {1.0, 0};
    RefIndexComplex_Wave.clear();
    _RefIndex_Comlex_WaveBinned.clear();

    PriScint_Decay.clear();
    PriScint_Decay.push_back( {0, 1.0} );
    PriScint_Raise.clear();
    PriScint_Raise.push_back( {0, 1.0} );

    _Rayleigh_WaveBinned.clear();

    RefIndex_Wave.clear();
    _RefIndex_WaveBinned.clear();

    AbsCoeff_Wave.clear();
    _AbsCoeff_WaveBinned.clear();

    ReemissionProb_Wave.clear();
    _ReemissionProb_WaveBinned.clear();

    PrimarySpectrum.clear();

    SecondarySpectrum.clear();

    PhotonYield = 0;
    IntrEnergyRes = 0;

    _GeoMat = nullptr; //if created, will be deleted by TGeoManager
    _GeoMed = nullptr; //if created, will be deleted by TGeoManager

    clearDynamicProperties();
}

void AMaterial::clearDynamicProperties()
{
    delete _PrimarySpectrumHist;   _PrimarySpectrumHist   = nullptr;
    delete _SecondarySpectrumHist; _SecondarySpectrumHist = nullptr;
}

void AMaterial::writeToJson(QJsonObject & json) const
{
    json["*Name"]           = Name;

    Composition.writeToJson(json);
    json["Density"]         = Density;
    json["UseNistMaterial"] = UseNistMaterial;
    json["NistMaterial"]    = NistMaterial;
    json["Temperature"]     = Temperature;

    json["RefIndex"]    = RefIndex;
    json["AbsCoeff"]    = AbsCoeff;
    json["RayleighMFP"] = RayleighMFP;
    json["RayleighWave"] = RayleighWave;

    json["ReemissionProb"] = ReemissionProb;

    json["Dielectric"] = Dielectric;
    {
        QJsonArray ar;
        ar << RefIndexComplex.real() << RefIndexComplex.imag();
        json["RefIndexComplex"] = ar;
    }
    {
        QJsonArray ar;
        for (const auto & rec : RefIndexComplex_Wave)
        {
            QJsonArray el;
            el << rec.first << rec.second.real() << rec.second.imag();
            ar.append(el);
        }
        json["RefIndexComplex_Wave"] = ar;
    }

    json["PhotonYield"] = PhotonYield;
    json["IntrEnergyRes"] = IntrEnergyRes;

    {
        QJsonArray ar;
        for (const auto & pair : PriScint_Decay)
        {
            QJsonArray el;
            el << pair.first << pair.second;
            ar.push_back(el);
        }
        json["PrimScintDecay"] = ar;
    }

    {
        QJsonArray ar;
        for (const auto & pair : PriScint_Raise)
        {
            QJsonArray el;
            el << pair.first << pair.second;
            ar.push_back(el);
        }
        json["PrimScintRaise"] = ar;
    }

    json["W"] = W;
    json["SecScint_PhYield"] = SecScintPhotonYield;
    json["SecScint_Tau"] = SecScintDecayTime;
    json["ElDriftVelo"] = ElDriftVelocity;
    json["ElDiffusionL"] = ElDiffusionL;
    json["ElDiffusionT"] = ElDiffusionT;

    json["Comments"] = Comments;

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(RefIndex_Wave, ar);
        json["RefIndexWave"] = ar;
    }

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(AbsCoeff_Wave, ar);
        json["BulkAbsorptionWave"] = ar;
    }

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(ReemissionProb_Wave, ar);
        json["ReemissionProbabilityWave"] = ar;
    }

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(PrimarySpectrum, ar);
        json["PrimScintSpectrum"] = ar;
    }

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(SecondarySpectrum, ar);
        json["SecScintSpectrum"] = ar;
    }

    {
        QJsonArray ar;
        for (const QString & s : Tags) ar.append(s);
        json["Tags"] = ar;
    }
}

bool AMaterial::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "*Name", Name);

    Composition.readFromJson(json);
    jstools::parseJson(json, "Density", Density);
    jstools::parseJson(json, "UseNistMaterial",  UseNistMaterial);
    jstools::parseJson(json, "NistMaterial",     NistMaterial);
    jstools::parseJson(json, "Temperature", Temperature);

    jstools::parseJson(json, "RefIndex", RefIndex);
    jstools::parseJson(json, "AbsCoeff", AbsCoeff);
    jstools::parseJson(json, "RayleighMFP", RayleighMFP);
    jstools::parseJson(json, "RayleighWave", RayleighWave);
    jstools::parseJson(json, "ReemissionProb", ReemissionProb);

    jstools::parseJson(json, "PhotonYield",   PhotonYield);
    jstools::parseJson(json, "IntrEnergyRes", IntrEnergyRes);

    jstools::parseJson(json, "Dielectric", Dielectric);
    {
        QJsonArray ar;
        jstools::parseJson(json, "RefIndexComplex", ar);
        if (ar.size() != 2) AErrorHub::addError("Bad size of complex refractive index record");
        else
        {
            const double real = ar[0].toDouble(-1e99);
            const double imag = ar[1].toDouble(-1e99);
            RefIndexComplex = {real, imag};
        }
    }
    {
        QJsonArray ar;
        jstools::parseJson(json, "RefIndexComplex_Wave", ar);
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonArray el = ar[i].toArray();
            if (el.size() < 3)
            {
                AErrorHub::addError("Bad size for a record of complex N");
                continue;
            }
            const double wave = el[0].toDouble(-1e99);
            const double real = el[1].toDouble(-1e99);
            const double imag = el[2].toDouble(-1e99);
            if (wave == -1e99 || real == -1e99 || imag == -1e99)
            {
                AErrorHub::addError("Convertion to double error for a record of complex N");
                continue;
            }
            RefIndexComplex_Wave.push_back( {wave, {real, imag}} );
        }
    }

    {
        PriScint_Decay.clear();
        QJsonArray ar = json["PrimScintDecay"].toArray();
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonArray el = ar[i].toArray();
            if (el.size() == 2) PriScint_Decay.push_back( {el[0].toDouble(), el[1].toDouble()} );
            else
                qWarning() << "Bad size of decay time pair, skipping!";
        }
        if (PriScint_Decay.empty()) PriScint_Decay.push_back({0,1.0});
    }

    {
        PriScint_Raise.clear();
        QJsonArray ar = json["PrimScintRaise"].toArray();
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonArray el = ar[i].toArray();
            if (el.size() == 2) PriScint_Raise.push_back( {el[0].toDouble(), el[1].toDouble()} );
            else
                qWarning() << "Bad size of raise time pair, skipping!";
        }
        if (PriScint_Raise.empty()) PriScint_Raise.push_back({0,1.0});
    }

    jstools::parseJson(json, "W",                W);
    jstools::parseJson(json, "SecScint_PhYield", SecScintPhotonYield);
    jstools::parseJson(json, "SecScint_Tau",     SecScintDecayTime);
    jstools::parseJson(json, "ElDriftVelo",      ElDriftVelocity);
    jstools::parseJson(json, "ElDiffusionL",     ElDiffusionL);
    jstools::parseJson(json, "ElDiffusionT",     ElDiffusionT);
    jstools::parseJson(json, "Comments",         Comments);

    {
        QJsonArray ar = json["RefIndexWave"].toArray();
        jstools::readDPairVectorFromArray(ar, RefIndex_Wave);
    }

    {
        QJsonArray ar = json["BulkAbsorptionWave"].toArray();
        jstools::readDPairVectorFromArray(ar, AbsCoeff_Wave);
    }

    {
        QJsonArray ar = json["ReemissionProbabilityWave"].toArray();
        jstools::readDPairVectorFromArray(ar, ReemissionProb_Wave);
    }

    {
        QJsonArray ar = json["PrimScintSpectrum"].toArray();
        jstools::readDPairVectorFromArray(ar, PrimarySpectrum);
    }

    {
        QJsonArray ar = json["SecScintSpectrum"].toArray();
        jstools::readDPairVectorFromArray(ar, SecondarySpectrum);
    }

    QJsonArray ar = json["Tags"].toArray();
    for (int i = 0; i < ar.size(); i++) Tags.push_back(ar[i].toString());

    return true;
}

QString AMaterial::checkMaterial() const
{
    if (Name.isEmpty()) return "Empty material name!";
    if (!Composition.ErrorString.isEmpty())
        return Name + ": " + Composition.ErrorString;

    if (Density <= 0) return Name + ": Non-positive density";
    if (Temperature <= 0) return Name + ": Non-positive temperature";

    return "";
}

void AMaterial::importComposition(TGeoMaterial * mat)
{
    QString compStr = AMatComposition::geoMatToCompositionString(mat);
    Composition.setCompositionString(compStr);
    // !!!*** error handling
}
