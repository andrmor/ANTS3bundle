#ifndef AMATERIAL_H
#define AMATERIAL_H

#include <QVector>
#include <QPair>
#include <QString>

#include "amaterialcomposition.h"

class QJsonObject;
class TH1D;
class TGeoMaterial;
class TGeoMedium;
class APair_ValueAndWeight;
class TRandom2;

class AMaterial
{
public:
    AMaterial();

    QString name;
    double density; //in g/cm3
    double temperature = 298.0; //in K
    double n;   //refractive index for monochrome
    double abs; //exp absorption per mm   for monochrome    (I = I0*exp(-abs*length[mm]))
    double reemissionProb; //for waveshifters: probability that absorbed photon is reemitted
    double rayleighMFP = 0; //0 - no rayleigh scattering
    double rayleighWave;
    QVector<double> rayleighBinned;//regular step (WaveStep step, WaveNodes bins)
    double e_driftVelocity;
    double W; //default W
    double e_diffusion_T = 0; //in mm2/ns
    double e_diffusion_L = 0; //in mm2/ns
    double SecYield;  // ph per secondary electron
    QVector<APair_ValueAndWeight> PriScint_Decay;
    QVector<APair_ValueAndWeight> PriScint_Raise;

    double PhotonYieldDefault = 0;   //make it possible to define different value for different particle names
    //double getPhotonYield(int iParticle) const;   !!!***

    double IntrEnResDefault = 0;
    //double getIntrinsicEnergyResolution(int iParticle) const; !!!***

    double SecScintDecayTime;
    QString Comments;

    QVector<QString> Tags; // used in material library     !!!*** to std::vector

    AMaterialComposition ChemicalComposition;

    bool bG4UseNistMaterial = false;
    QString G4NistMaterial;

    /* make it possible to define for diffrent particles!
    double PhYield = 0;         // Photon yield of the primary scintillation
    double IntrEnergyRes = 0; // intrinsic energy resolution
    */

    // !!!*** to std::vector<DPair>
    QVector<double> nWave_lambda;
    QVector<double> nWave;
    QVector<double> nWaveBinned; //regular step (WaveStep step, WaveNodes bins)
    double getRefractiveIndex(int iWave = -1) const;

    QVector<double> absWave_lambda;
    QVector<double> absWave;
    QVector<double> absWaveBinned; //regular step (WaveStep step, WaveNodes bins)
    double getAbsorptionCoefficient(int iWave = -1) const;

    QVector<double> reemisProbWave;
    QVector<double> reemisProbWave_lambda;
    QVector<double> reemissionProbBinned; //regular step (WaveStep step, WaveNodes bins)
    double getReemissionProbability(int iWave = -1) const;

    QVector<double> PrimarySpectrum_lambda;
    QVector<double> PrimarySpectrum;
    TH1D          * PrimarySpectrumHist = 0;
    QVector<double> SecondarySpectrum_lambda;
    QVector<double> SecondarySpectrum;
    TH1D          * SecondarySpectrumHist = 0;

    TGeoMaterial  * GeoMat = nullptr; // handled by TGeoManager
    TGeoMedium    * GeoMed = nullptr; // handled by TGeoManager
    void generateTGeoMat();

    double GeneratePrimScintTime(TRandom2 * RandGen) const;

    void updateRuntimeProperties();

    void clear();

    void writeToJson (QJsonObject & json) const;
    bool readFromJson(const QJsonObject &json);    // !!!*** TODO refactor

    QString checkMaterial() const;

private:
    //run-time properties
    double _PrimScintSumStatWeight_Decay;
    double _PrimScintSumStatWeight__Raise;

private:
    double FT(double td, double tr, double t) const;
};

class APair_ValueAndWeight
{
public:
    double value;
    double statWeight;

    APair_ValueAndWeight(double value, double statWeight) : value(value), statWeight(statWeight) {}
    APair_ValueAndWeight() {}
};

#endif // AMATERIAL_H
