#ifndef AMATERIAL_H
#define AMATERIAL_H

#include <QString>
#include <QVector>

#include <vector>
#include <complex>

#include "amaterialcomposition.h"

class QJsonObject;
class TH1D;
class TGeoMaterial;
class TGeoMedium;
class ARandomHub;

class AMaterial
{
public:
    AMaterial();

    QString Name;
    double  Density;              // in g/cm3
    double  Temperature = 298.0;  // in K

    double  RefIndex;             // refractive index for wave=-1 (or wavelength unresolved)
    bool    Dielectric = true;
    std::complex<double> RefIndexComplex = {1.0, 0};
    //double  ReN = 1.0;
    //double  ImN = 0;
    std::vector<std::pair<double,std::complex<double>>> RefIndexComplex_Wave; // {Wave[nm], ComplexRefractiveIndex}

    double  AbsCoeff;             // in mm-1 (I = I0*exp(-AbsCoeff*length[mm]))

    double  RayleighMFP = 0;      // in mm -> 0 - no Rayleigh scattering
    double  RayleighWave;         // in nm

    double  ReemissionProb;       // probability that absorbed photon is reemitted (to implement waveshifters)

    double PhotonYieldDefault = 0;   //make it possible to define different value for different particle names
    double IntrEnResDefault = 0;
    std::vector<std::pair<double,double>> PriScint_Decay; // elements: {value, weight}
    std::vector<std::pair<double,double>> PriScint_Raise; // elements: {value, weight}

    double SecScintDecayTime;

    double e_driftVelocity;
    double W; //default W
    double e_diffusion_T = 0; //in mm2/ns
    double e_diffusion_L = 0; //in mm2/ns
    double SecYield;  // ph per secondary electron

    QString Comments;
    std::vector<QString> Tags; // used in material library

    AMaterialComposition ChemicalComposition;

    bool    UseNistMaterial = false;
    QString NistMaterial;

    /* make it possible to define for diffrent particles!
    double PhYield = 0;         // Photon yield of the primary scintillation
    double IntrEnergyRes = 0; // intrinsic energy resolution
    */

    // !!!*** to std::vector<std::pair>
    QVector<double> nWave_lambda;
    QVector<double> nWave;

    QVector<double> absWave_lambda;
    QVector<double> absWave;

    QVector<double> reemisProbWave;
    QVector<double> reemisProbWave_lambda;

    QVector<double> PrimarySpectrum_lambda;
    QVector<double> PrimarySpectrum;
    QVector<double> SecondarySpectrum_lambda;
    QVector<double> SecondarySpectrum;

    void    clear();
    void    clearDynamicProperties();

    void    updateRuntimeProperties();

    void    generateTGeoMat();
    double  generatePrimScintTime(ARandomHub & Random) const; // use Random as argument just to show that there is external dependence

    double  getRefractiveIndex(int iWave = -1) const;
    const std::complex<double> & getComplexRefractiveIndex(int iWave = -1) const;

    double  getAbsorptionCoefficient(int iWave = -1) const;

    double  getReemissionProbability(int iWave = -1) const;

    static constexpr double c_in_vac = 299.7925;     // speed of light in mm/ns
    double  getSpeedOfLight(int iWave = -1) const;

    void    writeToJson (QJsonObject & json) const;
    bool    readFromJson(const QJsonObject &json);    // !!!*** TODO refactor add error control

    QString checkMaterial() const;

    void    importComposition(TGeoMaterial * mat);

    //run-time properties
    TGeoMaterial  * GeoMat = nullptr; // handled by TGeoManager
    TGeoMedium    * GeoMed = nullptr; // handled by TGeoManager
    double          _PrimScintSumStatWeight_Decay;
    double          _PrimScintSumStatWeight__Raise;
    TH1D          * PrimarySpectrumHist = nullptr;
    TH1D          * SecondarySpectrumHist = nullptr;
    QVector<double> rayleighBinned;//regular step (WaveStep step, WaveNodes bins)
    QVector<double> nWaveBinned; //regular step (WaveStep step, WaveNodes bins)
    QVector<double> absWaveBinned; //regular step (WaveStep step, WaveNodes bins)
    QVector<double> reemissionProbBinned; //regular step (WaveStep step, WaveNodes bins)
      // complex refraction index-related
    //std::complex<double> RefIndex_Complex;                        // also computed for purely real index!
    std::vector<std::complex<double>> RefIndex_Comlex_WaveBinned; // also computed for purely real index!
    //double Abs_FromComplex;                                       // only for complex case
    //std::vector<double> Abs_FromComplex_WaveBinned;               // only for comlex case

private:
    double FT(double td, double tr, double t) const;
};

#endif // AMATERIAL_H
