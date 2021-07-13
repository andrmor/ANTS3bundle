#include "amaterial.h"
#include "ajsontools.h"
#include "achemicalelement.h"
//#include "acommonfunctions.h"
#include "afiletools.h"

#include <QStandardPaths>
#include <QFile>
#include <QDebug>

#include "TH1D.h"
#include "TRandom2.h"

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
    if (iWave == -1 || nWaveBinned.isEmpty()) return n;
    return nWaveBinned.at(iWave);
}

double AMaterial::getAbsorptionCoefficient(int iWave) const
{
    //qDebug() << iWave << ( absWaveBinned.size()  >0 ? absWaveBinned.at(iWave) : -1 ) ;
    if (iWave == -1 || absWaveBinned.isEmpty()) return abs;
    return absWaveBinned.at(iWave);
}

double AMaterial::getReemissionProbability(int iWave) const
{
    //qDebug() << "reemis->" << iWave << ( reemissionProbBinned.size() > 0 ? reemissionProbBinned.at(iWave) : reemissionProb );
    if (iWave == -1 || reemissionProbBinned.isEmpty()) return reemissionProb;
    return reemissionProbBinned.at(iWave);
}

void AMaterial::generateTGeoMat()
{
    GeoMat = ChemicalComposition.generateTGeoMaterial(name.toLocal8Bit().data(), density);
}

double AMaterial::FT(double td, double tr, double t) const
{
    return 1.0 - ((tr + td) / td) * exp(- t / td) + (tr / td) * exp(- t * (1.0 / tr + 1.0 / td));
}

double single_exp(double t, double tau2)
{
    return std::exp(-1.0*t/tau2)/tau2;
}
double bi_exp(double t, double tau1,double tau2)
{
    return std::exp(-1.0*t/tau2)*(1-std::exp(-1.0*t/tau1))/tau2/tau2*(tau1+tau2);
}
double AMaterial::GeneratePrimScintTime(TRandom2 * RandGen) const
{
    //select decay time component
    double DecayTime = 0;
    if (PriScint_Decay.size() == 1)
        DecayTime = PriScint_Decay.first().value;
    else
    {
        //selecting decay time component
        const double generatedStatWeight = _PrimScintSumStatWeight_Decay * RandGen->Rndm();
        double cumulativeStatWeight = 0;
        for (int i=0; i<PriScint_Decay.size(); i++)
        {
            cumulativeStatWeight += PriScint_Decay.at(i).statWeight;
            if (generatedStatWeight < cumulativeStatWeight)
            {
                DecayTime = PriScint_Decay.at(i).value;
                break;
            }
        }
    }

    if (DecayTime == 0)
        return 0; // decay time is 0 -> rise time is ignored

    //select rise time component
    double RiseTime = 0;
    if (PriScint_Raise.size() == 1)
        RiseTime = PriScint_Raise.first().value;
    else
    {
        //selecting raise time component
        const double generatedStatWeight = _PrimScintSumStatWeight__Raise * RandGen->Rndm();
        double cumulativeStatWeight = 0;
        for (int i=0; i<PriScint_Raise.size(); i++)
        {
            cumulativeStatWeight += PriScint_Raise.at(i).statWeight;
            if (generatedStatWeight < cumulativeStatWeight)
            {
                RiseTime = PriScint_Raise.at(i).value;
                break;
            }
        }
    }

    if (RiseTime == 0)
        return RandGen->Exp(DecayTime);

    double EmissionTime = 0;
    // From G4Scintillation of Geant4
    double d = (RiseTime + DecayTime) / DecayTime;
    while (true)
    {
        double ran1 = RandGen->Rndm();
        double ran2 = RandGen->Rndm();
        EmissionTime = -1.0 * DecayTime * std::log(1.0 - ran1);
        double gg = d * single_exp(EmissionTime, DecayTime);
        if (ran2 <= bi_exp(EmissionTime, RiseTime, DecayTime) / gg)
            break;
    }

    return EmissionTime;
}

void AMaterial::updateRuntimeProperties()
{
    //updating sum stat weights for primary scintillation time generator
    _PrimScintSumStatWeight_Decay = 0;
    _PrimScintSumStatWeight__Raise = 0;
    for (const APair_ValueAndWeight& pair : PriScint_Decay)
        _PrimScintSumStatWeight_Decay += pair.statWeight;
    for (const APair_ValueAndWeight& pair : PriScint_Raise)
        _PrimScintSumStatWeight__Raise += pair.statWeight;
}

void AMaterial::clear()
{
    name = "Undefined";
    n = 1.0;
    density = abs = rayleighMFP = reemissionProb = 0;
    temperature = 298.0;
    e_driftVelocity = W = SecYield = SecScintDecayTime = e_diffusion_L = e_diffusion_T = 0;
    rayleighWave = 500.0;
    Comments = "";

    PriScint_Decay.clear();
    PriScint_Decay << APair_ValueAndWeight(0, 1.0);
    PriScint_Raise.clear();
    PriScint_Raise << APair_ValueAndWeight(0, 1.0);

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

    delete PrimarySpectrumHist;   PrimarySpectrumHist   = nullptr;
    delete SecondarySpectrumHist; SecondarySpectrumHist = nullptr;

    PhotonYieldDefault = 0;
    IntrEnResDefault = 0;

    GeoMat = nullptr; //if created, deleted by TGeoManager
    GeoMed = nullptr; //if created, deleted by TGeoManager

    //Do not touch overrides - handled by loaded (want to keep overrides intact when handling inspector window)
}

void AMaterial::writeToJson(QJsonObject & json) const
{
    json["*MaterialName"] = name;
    json["Density"] = density;
    json["Temperature"] = temperature;
    json["ChemicalComposition"] = ChemicalComposition.writeToJson();
    json["RefractiveIndex"] = n;
    json["BulkAbsorption"] = abs;
    json["RayleighMFP"] = rayleighMFP;
    json["RayleighWave"] = rayleighWave;
    json["ReemissionProb"] = reemissionProb;

    json["PhotonYieldDefault"] = PhotonYieldDefault;
    json["IntrEnergyResDefault"] = IntrEnResDefault;

    {
        QJsonArray ar;
        for (const APair_ValueAndWeight& pair : PriScint_Decay)
        {
            QJsonArray el;
            el << pair.value << pair.statWeight;
            ar.append(el);
        }
        json["PrimScintDecay"] = ar;
    }

    {
        QJsonArray ar;
        for (const APair_ValueAndWeight& pair : PriScint_Raise)
        {
            QJsonArray el;
            el << pair.value << pair.statWeight;
            ar.append(el);
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

    {
        QJsonArray ar;
        for (const QString & s : Tags) ar.append(s);
        json["*Tags"] = ar;
    }

    json["bG4UseNistMaterial"] = bG4UseNistMaterial;
    json["G4NistMaterial"] = G4NistMaterial;

    //MatParticle properties
    //if a particle has default configuration (TrackingAllowed and MatIsTransparent), skip its record
    QJsonArray jParticleEntries;
    const QVector<AParticle *> *ParticleCollection = MpCollection->getParticleCollection();
    for (int ip=0; ip<ParticleCollection->size(); ip++)
    {
        QJsonObject jMatParticle;

        QJsonObject jparticle;
        ParticleCollection->at(ip)->writeToJson(jparticle);
        jMatParticle["*Particle"] = jparticle;
        jMatParticle["PrimScintPhYield"] = MatParticle[ip].PhYield;
        jMatParticle["IntrEnergyRes"] = MatParticle[ip].IntrEnergyRes;

        //appending this particle entry to the json array
        jParticleEntries.append(jMatParticle);
    }
    //if not empty, appending array with Matparticle properties to main json object
    if (!jParticleEntries.isEmpty()) json["MatParticles"] = jParticleEntries;
    */
}

bool AMaterial::readFromJson(const QJsonObject & json)
{
    clear(); //clear all settings and set default properties
    //general data
    jstools::parseJson(json, "*MaterialName", name);
    jstools::parseJson(json, "Density", density);
    temperature = 298.0; //compatibility
    jstools::parseJson(json, "Temperature", temperature);
    if (json.contains("ChemicalComposition"))
    {
        QJsonObject ccjson = json["ChemicalComposition"].toObject();
        ChemicalComposition.readFromJson(ccjson);
    }
    else ChemicalComposition.clear();
    jstools::parseJson(json, "RefractiveIndex", n);
    jstools::parseJson(json, "BulkAbsorption", abs);
    jstools::parseJson(json, "RayleighMFP", rayleighMFP);
    jstools::parseJson(json, "RayleighWave", rayleighWave);
    jstools::parseJson(json, "ReemissionProb", reemissionProb);
    //PhotonYieldDefault for compatibility at the end

    if (json.contains("PrimScint_Tau")) //compatibility
    {
        double tau = json["PrimScint_Tau"].toDouble();
        PriScint_Decay.clear();
        PriScint_Decay << APair_ValueAndWeight(tau, 1.0);
    }
    if (json.contains("PrimScint_Decay")) //compatibility
    {
        PriScint_Decay.clear();
        if (json["PrimScint_Decay"].isArray())
        {
            QJsonArray ar = json["PrimScint_Decay"].toArray();
            for (int i=0; i<ar.size(); i++)
            {
                QJsonArray el = ar.at(i).toArray();
                if (el.size() == 2)
                    PriScint_Decay << APair_ValueAndWeight(el.at(1).toDouble(), el.at(0).toDouble());
                else
                    qWarning() << "Bad size of decay time pair, skipping!";
            }
        }
        else
        {
            double tau = json["PrimScint_Decay"].toDouble();
            PriScint_Decay << APair_ValueAndWeight(tau, 1.0);
        }
    }
    if (json.contains("PrimScint_Raise")) //compatibility
    {
        PriScint_Raise.clear();
        if (json["PrimScint_Raise"].isArray())
        {
            QJsonArray ar = json["PrimScint_Raise"].toArray();
            for (int i=0; i<ar.size(); i++)
            {
                QJsonArray el = ar.at(i).toArray();
                if (el.size() == 2)
                    PriScint_Raise << APair_ValueAndWeight(el.at(1).toDouble(), el.at(0).toDouble());
                else
                    qWarning() << "Bad size of raise time pair, skipping!";
            }
        }
        else
        {
            //compatibility
            double tau = json["PrimScint_Raise"].toDouble();
            PriScint_Raise << APair_ValueAndWeight(tau, 1.0);
        }
    }
    if (json.contains("PrimScintDecay"))
    {
        PriScint_Decay.clear();
        if (json["PrimScintDecay"].isArray())
        {
            QJsonArray ar = json["PrimScintDecay"].toArray();
            for (int i=0; i<ar.size(); i++)
            {
                QJsonArray el = ar.at(i).toArray();
                if (el.size() == 2)
                    PriScint_Decay << APair_ValueAndWeight(el.at(0).toDouble(), el.at(1).toDouble());
                else
                    qWarning() << "Bad size of decay time pair, skipping!";
            }
        }
        else
        {
            double tau = json["PrimScintDecay"].toDouble();
            PriScint_Decay << APair_ValueAndWeight(tau, 1.0);
        }
    }
    if (json.contains("PrimScintRaise"))
    {
        PriScint_Raise.clear();
        if (json["PrimScintRaise"].isArray())
        {
            QJsonArray ar = json["PrimScintRaise"].toArray();
            for (int i=0; i<ar.size(); i++)
            {
                QJsonArray el = ar.at(i).toArray();
                if (el.size() == 2)
                    PriScint_Raise << APair_ValueAndWeight(el.at(0).toDouble(), el.at(1).toDouble());
                else
                    qWarning() << "Bad size of raise time pair, skipping!";
            }
        }
        else
        {
            //compatibility
            double tau = json["PrimScintRaise"].toDouble();
            PriScint_Raise << APair_ValueAndWeight(tau, 1.0);
        }
    }

    jstools::parseJson(json, "W",                W);
    jstools::parseJson(json, "SecScint_PhYield", SecYield);
    jstools::parseJson(json, "SecScint_Tau",     SecScintDecayTime);
    jstools::parseJson(json, "ElDriftVelo",      e_driftVelocity);
    jstools::parseJson(json, "ElDiffusionL",     e_diffusion_L);
    jstools::parseJson(json, "ElDiffusionT",     e_diffusion_T);
    jstools::parseJson(json, "Comments",         Comments);

    bG4UseNistMaterial = false;
    jstools::parseJson(json, "bG4UseNistMaterial", bG4UseNistMaterial);
    G4NistMaterial.clear();
    jstools::parseJson(json, "G4NistMaterial", G4NistMaterial);

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

    Tags.clear();
    if (json.contains("*Tags"))
    {
        QJsonArray ar = json["*Tags"].toArray();
        for (int i=0; i<ar.size(); i++) Tags << ar[i].toString();
    }

    //MatParticle properties
    //filling default properties for all particles
    int numParticles = MpCollection->countParticles();
    MatParticle.resize(numParticles);
    for (int ip=0; ip<numParticles; ip++)
    {
        MatParticle[ip].TrackingAllowed = true;
        MatParticle[ip].MaterialIsTransparent = true;
    }
    // reading defined properties
    QJsonArray jParticleEntries = json["MatParticles"].toArray();
    for (int index = 0; index < jParticleEntries.size(); index++)
    {
        QJsonObject jMatParticle = jParticleEntries[index].toObject();

        QJsonObject jparticle = jMatParticle["*Particle"].toObject();
        AParticle pa;
        pa.readFromJson(jparticle);
        bool bNewParticle = (MpCollection->findParticle(pa) == -1);
        if (bNewParticle && SuppressParticles.contains(pa.ParticleName))
            continue;

        int ip = MpCollection->findOrAddParticle(jparticle);
        MatParticle.resize(MpCollection->countParticles());
        MatParticleStructure & MP = MatParticle[ip];

        jstools::parseJson(jMatParticle, "TrackingAllowed",  MP.TrackingAllowed);
        jstools::parseJson(jMatParticle, "MatIsTransparent", MP.MaterialIsTransparent);
        jstools::parseJson(jMatParticle, "PrimScintPhYield", MP.PhYield);

        MP.IntrEnergyRes = 0;
        jstools::parseJson(jMatParticle, "IntrEnergyRes", MP.IntrEnergyRes);

        MP.DataSource.clear();
        jstools::parseJson(jMatParticle, "DataSource", MP.DataSource);
        MP.DataString.clear();
        jstools::parseJson(jMatParticle, "DataString", MP.DataString);

        MP.bCaptureEnabled = true; //compatibility
        jstools::parseJson(jMatParticle, "CaptureEnabled", MP.bCaptureEnabled);
        MP.bElasticEnabled = false; //compatibility
        jstools::parseJson(jMatParticle, "EllasticEnabled", MP.bElasticEnabled); //old configs were with this typo
        jstools::parseJson(jMatParticle, "ElasticEnabled",  MP.bElasticEnabled);
        MP.bUseNCrystal = false; //compatibility
        jstools::parseJson(jMatParticle, "UseNCrystal", MP.bUseNCrystal);

        MP.bAllowAbsentCsData = false;
        jstools::parseJson(jMatParticle, "AllowAbsentCsData", MP.bAllowAbsentCsData);

        if (jMatParticle.contains("TotalInteraction"))
        {
            QJsonArray iar = jMatParticle["TotalInteraction"].toArray();
            readTwoQVectorsFromJArray(iar, MP.InteractionDataX, MP.InteractionDataF);
        }

        QJsonArray arTerm;
        jstools::parseJson(jMatParticle, "Terminators", arTerm);
        jstools::parseJson(jMatParticle, "GammaTerminators", arTerm); //compatibility
        // old neutron terminators are NOT compatible with new system
        if (!arTerm.isEmpty())
        {
            MP.Terminators.clear();
            for (int iTerm=0; iTerm<arTerm.size(); iTerm++)
            {
                QJsonObject jterm = arTerm[iTerm].toObject();
                NeutralTerminatorStructure newTerm;
                newTerm.readFromJson(jterm, MpCollection);
                MP.Terminators << newTerm;
            }
        }
        else
        {
            if (jMatParticle.contains("NeutronTerminators"))
            {
                qWarning() << "This config file contains old (incompatible) standard for neutron interaction data - material is set to transparent for neutrons!";
                //making material transparent
                MP.MaterialIsTransparent = true;
            }
        }
    }
*/

    jstools::parseJson(json, "PhotonYieldDefault", PhotonYieldDefault);
    jstools::parseJson(json, "IntrEnergyResDefault", IntrEnResDefault);

    return true;
}

void MatParticleStructure::Clear()
{
    PhYield = 0;
    IntrEnergyRes = 0;
}

QString AMaterial::CheckMaterial() const
{
    const QString errInComposition = ChemicalComposition.checkForErrors();
    if (!errInComposition.isEmpty())
        return name + ": " + errInComposition;

    return "";
}
