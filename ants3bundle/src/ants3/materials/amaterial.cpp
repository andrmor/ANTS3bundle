#include "amaterial.h"
#include "achemicalelement.h"
//#include "acommonfunctions.h"
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

/*
double AMaterial::getPhotonYield(int iParticle) const
{
    if (iParticle < 0 || iParticle >= MatParticle.size()) return PhotonYieldDefault;

    const double & py = MatParticle.at(iParticle).PhYield;
    if (py == -1) return PhotonYieldDefault;
    return py;
}

double AMaterial::getIntrinsicEnergyResolution(int iParticle) const
{
    if (iParticle < 0 || iParticle >= MatParticle.size()) return IntrEnResDefault;

    const double & er = MatParticle.at(iParticle).IntrEnergyRes;
    if (er == -1) return IntrEnResDefault;
    return er;
}
*/

double AMaterial::getRefractiveIndex(int iWave) const
{
    if (iWave == -1 || nWaveBinned.isEmpty()) return RefIndex;
    return nWaveBinned[iWave];
}

const std::complex<double> & AMaterial::getComplexRefractiveIndex(int iWave) const
{
    if (iWave == -1 || RefIndex_Comlex_WaveBinned.empty()) return RefIndexComplex;
    return RefIndex_Comlex_WaveBinned[iWave];
}

double AMaterial::getAbsorptionCoefficient(int iWave) const
{
    if (Dielectric)
    {
        //qDebug() << iWave << absWaveBinned.size();
        if (iWave == -1 || absWaveBinned.isEmpty()) return AbsCoeff;
        return absWaveBinned[iWave];
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
    if (iWave == -1 || reemissionProbBinned.isEmpty()) return ReemissionProb;
    return reemissionProbBinned[iWave];
}

double AMaterial::getSpeedOfLight(int iWave) const
{
    double refIndexReal;
    if (Dielectric) refIndexReal = getRefractiveIndex(iWave);
    else
    {
        if (iWave == -1 || RefIndex_Comlex_WaveBinned.empty()) refIndexReal = RefIndexComplex.real();
        else refIndexReal = RefIndex_Comlex_WaveBinned[iWave].real();
    }

    return c_in_vac / refIndexReal;
}

void AMaterial::generateTGeoMat()
{
    GeoMat = ChemicalComposition.generateTGeoMaterial(Name.toLocal8Bit().data(), Density);
    GeoMat->SetTemperature(Temperature);
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
    nWaveBinned.clear();
    absWaveBinned.clear();
    RefIndex_Comlex_WaveBinned.clear();
    //Abs_FromComplex_WaveBinned.clear();
    rayleighBinned.clear();
    reemissionProbBinned.clear();
    delete PrimarySpectrumHist;   PrimarySpectrumHist   = nullptr;
    delete SecondarySpectrumHist; SecondarySpectrumHist = nullptr;

    if (WaveSet.Enabled)
    {
        if (Dielectric)
        {
            if (nWave_lambda.size() > 0)
            {
                WaveSet.toStandardBins(&nWave_lambda, &nWave, &nWaveBinned);
                for (const double & d : nWaveBinned) RefIndex_Comlex_WaveBinned.push_back({d, 0});
            }
            if (absWave_lambda.size() > 0) WaveSet.toStandardBins(&absWave_lambda, &absWave, &absWaveBinned);
        }
        else
        {
            if (!RefIndexComplex_Wave.empty())
            {
                WaveSet.toStandardBins(RefIndexComplex_Wave, RefIndex_Comlex_WaveBinned);
                for (auto & cri : RefIndex_Comlex_WaveBinned)
                    if (cri.imag() > 0) cri = std::conj(cri);

                //for (size_t i = 0; i < RefIndex_Comlex_WaveBinned.size(); i++)
                //{
                //    const double wave = WaveSet.From + WaveSet.Step * i; // [nm]
                //    Abs_FromComplex_WaveBinned.push_back(  fabs(4.0 * 3.1415926535 * RefIndex_Comlex_WaveBinned[i].imag() / wave * 1e6) ); // [mm-1]);
                //}
            }
        }

        if (reemisProbWave_lambda.size() > 0)
            WaveSet.toStandardBins(&reemisProbWave_lambda, &reemisProbWave, &reemissionProbBinned);

        if (RayleighMFP != 0)
        {
            double baseWave4 = RayleighWave * RayleighWave * RayleighWave * RayleighWave;
            double base = RayleighMFP / baseWave4;
            for (int i = 0; i < WaveNodes; i++)
            {
                double wave  = WaveSet.From + WaveSet.Step * i;
                double wave4 = wave * wave * wave * wave;
                rayleighBinned.append(base * wave4);
            }
        }

        if (PrimarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            WaveSet.toStandardBins(&PrimarySpectrum_lambda, &PrimarySpectrum, &y);
            TString name = "PrimScSp";
            PrimarySpectrumHist = new TH1D(name,"Primary scintillation", WaveNodes, WaveSet.From, WaveSet.To);
            for (int j = 1; j<WaveNodes+1; j++)  PrimarySpectrumHist->SetBinContent(j, y[j-1]);
            PrimarySpectrumHist->GetIntegral(); //to make thread safe
        }

        if (SecondarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            WaveSet.toStandardBins(&SecondarySpectrum_lambda, &SecondarySpectrum, &y);
            TString name = "SecScSp";
            SecondarySpectrumHist = new TH1D(name,"Secondary scintillation", WaveNodes, WaveSet.From, WaveSet.To);
            for (int j = 1; j<WaveNodes+1; j++)  SecondarySpectrumHist->SetBinContent(j, y[j-1]);
            SecondarySpectrumHist->GetIntegral(); //to make thread safe
        }
    }

    //updating sum stat weights for primary scintillation time generator
    _PrimScintSumStatWeight_Decay = 0;
    _PrimScintSumStatWeight__Raise = 0;
    for (const auto & pair : PriScint_Decay)
        _PrimScintSumStatWeight_Decay += pair.second;
    for (const auto & pair : PriScint_Raise)
        _PrimScintSumStatWeight__Raise += pair.second;
}

void AMaterial::clear()
{
    Name = "Undefined";
    RefIndex = 1.0;
    Density = AbsCoeff = RayleighMFP = ReemissionProb = 0;
    Temperature = 298.0;
    e_driftVelocity = W = SecYield = SecScintDecayTime = e_diffusion_L = e_diffusion_T = 0;
    RayleighWave = 500.0;
    Comments = "";

    Dielectric = true;
    RefIndexComplex = {1.0, 0};
    RefIndexComplex_Wave.clear();
    RefIndex_Comlex_WaveBinned.clear();

    PriScint_Decay.clear();
    PriScint_Decay.push_back( {0, 1.0} );
    PriScint_Raise.clear();
    PriScint_Raise.push_back( {0, 1.0} );

    rayleighBinned.clear();

    nWave_lambda.clear();
    nWave.clear();
    nWaveBinned.clear();

    absWave_lambda.clear();
    absWave.clear();
    absWaveBinned.clear();

    reemisProbWave.clear();
    reemisProbWave_lambda.clear();
    reemissionProbBinned.clear();

    PrimarySpectrum_lambda.clear();
    PrimarySpectrum.clear();

    SecondarySpectrum_lambda.clear();
    SecondarySpectrum.clear();

    PhotonYieldDefault = 0;
    IntrEnResDefault = 0;

    GeoMat = nullptr; //if created, will be deleted by TGeoManager
    GeoMed = nullptr; //if created, will be deleted by TGeoManager

    clearDynamicProperties();
}

void AMaterial::clearDynamicProperties()
{
    delete PrimarySpectrumHist;   PrimarySpectrumHist   = nullptr;
    delete SecondarySpectrumHist; SecondarySpectrumHist = nullptr;
}

void AMaterial::writeToJson(QJsonObject & json) const
{
    json["*Name"]       = Name;
    json["Density"]     = Density;
    json["Temperature"] = Temperature;

    json["Composition"] = ChemicalComposition.writeToJson();

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

    json["PhotonYieldDefault"] = PhotonYieldDefault;
    json["IntrEnergyResDefault"] = IntrEnResDefault;

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
    json["SecScint_PhYield"] = SecYield;
    json["SecScint_Tau"] = SecScintDecayTime;
    json["ElDriftVelo"] = e_driftVelocity;
    json["ElDiffusionL"] = e_diffusion_L;
    json["ElDiffusionT"] = e_diffusion_T;

    json["Comments"] = Comments;

    /*
    {
        QJsonArray ar;
        writeTwoQVectorsToJArray(nWave_lambda, nWave, ar);
        json["RefractiveIndexWave"] = ar;
    }

    {
        QJsonArray ar;
        writeTwoQVectorsToJArray(absWave_lambda, absWave, ar);
        json["BulkAbsorptionWave"] = ar;
    }

    {
        QJsonArray ar;
        writeTwoQVectorsToJArray(reemisProbWave_lambda, reemisProbWave, ar);
        json["ReemissionProbabilityWave"] = ar;
    }

    {
        QJsonArray ar;
        writeTwoQVectorsToJArray(PrimarySpectrum_lambda, PrimarySpectrum, ar);
        json["PrimScintSpectrum"] = ar;
    }

    {
        QJsonArray ar;
        writeTwoQVectorsToJArray(SecondarySpectrum_lambda, SecondarySpectrum, ar);
        json["SecScintSpectrum"] = ar;
    }

    */
    {
        QJsonArray ar;
        for (const QString & s : Tags) ar.append(s);
        json["*Tags"] = ar;
    }

    json["UseNistMaterial"] = UseNistMaterial;
    json["NistMaterial"]    = NistMaterial;
}

bool AMaterial::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "*Name", Name);
    jstools::parseJson(json, "Density", Density);
    jstools::parseJson(json, "Temperature", Temperature);

    QJsonObject ccjson = json["Composition"].toObject();
    ChemicalComposition.readFromJson(ccjson);

    jstools::parseJson(json, "RefIndex", RefIndex);
    jstools::parseJson(json, "AbsCoeff", AbsCoeff);
    jstools::parseJson(json, "RayleighMFP", RayleighMFP);
    jstools::parseJson(json, "RayleighWave", RayleighWave);
    jstools::parseJson(json, "ReemissionProb", ReemissionProb);

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
    jstools::parseJson(json, "SecScint_PhYield", SecYield);
    jstools::parseJson(json, "SecScint_Tau",     SecScintDecayTime);
    jstools::parseJson(json, "ElDriftVelo",      e_driftVelocity);
    jstools::parseJson(json, "ElDiffusionL",     e_diffusion_L);
    jstools::parseJson(json, "ElDiffusionT",     e_diffusion_T);
    jstools::parseJson(json, "Comments",         Comments);
    jstools::parseJson(json, "UseNistMaterial",  UseNistMaterial);
    jstools::parseJson(json, "NistMaterial",     NistMaterial);

/*
    //wavelength-resolved data
    if (json.contains("RefractiveIndexWave"))
    {
        QJsonArray ar = json["RefractiveIndexWave"].toArray();
        readTwoQVectorsFromJArray(ar, nWave_lambda, nWave);
    }
    if (json.contains("BulkAbsorptionWave"))
    {
        QJsonArray ar = json["BulkAbsorptionWave"].toArray();
        readTwoQVectorsFromJArray(ar, absWave_lambda, absWave);
    }
    if (json.contains("ReemissionProbabilityWave"))
    {
        QJsonArray ar = json["ReemissionProbabilityWave"].toArray();
        readTwoQVectorsFromJArray(ar, reemisProbWave_lambda, reemisProbWave);
    }
    if (json.contains("PrimScintSpectrum"))
    {
        QJsonArray ar = json["PrimScintSpectrum"].toArray();
        readTwoQVectorsFromJArray(ar, PrimarySpectrum_lambda, PrimarySpectrum);
    }
    if (json.contains("SecScintSpectrum"))
    {
        QJsonArray ar = json["SecScintSpectrum"].toArray();
        readTwoQVectorsFromJArray(ar, SecondarySpectrum_lambda, SecondarySpectrum);
    }

*/
    if (json.contains("Tags"))
    {
        QJsonArray ar = json["*Tags"].toArray();
        for (int i = 0; i < ar.size(); i++) Tags.push_back(ar[i].toString());
    }

    jstools::parseJson(json, "PhotonYieldDefault",   PhotonYieldDefault);
    jstools::parseJson(json, "IntrEnergyResDefault", IntrEnResDefault);

    return true;
}

QString AMaterial::checkMaterial() const
{
    const QString errInComposition = ChemicalComposition.checkForErrors();
    if (!errInComposition.isEmpty())
        return Name + ": " + errInComposition;

    if (Density <= 0) return Name + ": Non-positive density";
    if (Temperature <= 0) return Name + ": Non-positive temperature";

    return "";
}

void AMaterial::importComposition(TGeoMaterial * mat)
{
    ChemicalComposition.import(mat);
}
