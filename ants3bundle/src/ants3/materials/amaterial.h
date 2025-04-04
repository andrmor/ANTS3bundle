#ifndef AMATERIAL_H
#define AMATERIAL_H

#include <QString>
#include <QJsonObject>

#include <vector>
#include <complex>

#include "amatcomposition.h"

class TH1D;
class TGeoMaterial;
class TGeoMedium;
class ARandomHub;

class AMaterial
{
public:
    AMaterial();

    QString Name;

    // composition
    AMatComposition      Composition;
    bool                 UseG4Material = false;
    QString              G4MaterialName;

    // optics
    bool    Dielectric = true;    // not dielectric => metal => use complex refractive index on reflection from dielectric
    double                                              RefIndex;                   // for wave=-1 or wavelength unresolved sim
    std::vector<std::pair<double,double>>               RefIndex_Wave;              // {wave[nm], RefIndex}
    std::complex<double>                                RefIndexComplex = {1.0, 0};
    std::vector<std::pair<double,std::complex<double>>> RefIndexComplex_Wave;       // {Wave[nm], ComplexRefractiveIndex}

    double                                AbsCoeff;       // in mm-1 (I = I0*exp(-AbsCoeff*length[mm]))
    std::vector<std::pair<double,double>> AbsCoeff_Wave;

    double  RayleighMFP = 0;      // in mm    if=0 => no Rayleigh scattering
    double  RayleighWave;         // in nm

    double CustomScatterMFP = 0;  // in mm    if=0 => Custom scattering is disabled

    double  ReemissionProb;       // probability that absorbed photon is reemitted (to implement waveshifters, uses PrimaryScint data)
    bool    IgnoreEnergyConservationInReemission = false; // if true, sample wavelength from entire spectrum, otherwise only longer wavelengths
    std::vector<std::pair<double,double>> ReemissionProb_Wave;

    QJsonObject RefIndexImporter;

    // primary scintillation
    double PhotonYield = 0;
    double IntrEnergyRes = 0;
    std::vector<std::pair<double,double>> PrimarySpectrum;
    std::vector<std::pair<double,double>> PriScint_Decay; // elements: {value, weight}
    std::vector<std::pair<double,double>> PriScint_Raise; // elements: {value, weight}

    // secondary scintillation
    double W;                     // energy per pair in keV
    double ElDriftVelocity;
    double ElDiffusionT = 0;      // in mm2/ns
    double ElDiffusionL = 0;      // in mm2/ns
    double SecScintPhotonYield;   // ph per secondary electron
    std::vector<std::pair<double,double>> SecondarySpectrum;
    double SecScintDecayTime;     // !!!*** reuse system of prim scint timing

    // misc
    QString Comments;
    std::vector<QString> Tags; // used in material library

    void    clear();
    void    clearDynamicProperties();

    QString convertPressureToDensity();
    void    updateRuntimeOpticalProperties();

    void    generateTGeoMat();
    double  generatePrimScintTime(ARandomHub & Random) const; // use Random as argument just to show that there is external dependence

    double  getRefractiveIndex(int iWave = -1) const;
    const std::complex<double> & getComplexRefractiveIndex(int iWave = -1) const;

    double  getAbsorptionCoefficient(int iWave = -1) const;

    double  getReemissionProbability(int iWave = -1) const;

    static constexpr double c_in_vac = 299.7925;     // speed of light in mm/ns
    double  getSpeedOfLight(int iWave = -1) const;

    void    writeToJson (QJsonObject & json) const;
    bool    readFromJson(const QJsonObject & json);    // !!!*** TODO refactor add error control

    QString checkMaterial() const; // !!!***

    void    importComposition(TGeoMaterial * mat); //   // !!!*** error handling

  // --- run-time properties ---
    TGeoMaterial  * _GeoMat = nullptr;
    TGeoMedium    * _GeoMed = nullptr;
    TH1D          * _PrimarySpectrumHist = nullptr;
    TH1D          * _SecondarySpectrumHist = nullptr;
    double          _PrimScintSumStatWeight_Decay = 0;
    double          _PrimScintSumStatWeight__Raise = 0;

    //regular step (WaveStep step, WaveNodes bins)
    std::vector<double> _Rayleigh_WaveBinned;
    std::vector<double> _RefIndex_WaveBinned;
    std::vector<double> _AbsCoeff_WaveBinned;
    std::vector<double> _ReemissionProb_WaveBinned;

    std::vector<std::complex<double>> _RefIndex_Comlex_WaveBinned; // also computed for purely real index! - still need? !!!***
  // ---

private:
    double FT(double td, double tr, double t) const;
};

#endif // AMATERIAL_H
