#ifndef ASENSORMODEL_H
#define ASENSORMODEL_H

#include <QString>
#include <vector>

class QJsonObject;
class TH1D;

class AInterfaceAwareRuntimeProps
{
public:
    double              EffectivePDE = 1.0; // used if spectralPDE and angular data ARE NOT provided
                                            //   OR for photons with -1 waveindex when spectralPDE is provided but angular not
    std::vector<double> PDEbinned;          // used if spectralPDE data are provided, but not angular
    std::vector<double> AngularBinned;      // used if spectralPDE data and angular data ARE provided
};

class ASensorModel
{
public:
    ASensorModel(const QString & Name = "NoName") : Name(Name) {}

    QString Name;

    bool    SiPM = false;
    int     PixelsX = 50;
    int     PixelsY = 50;
    double  PixelSizeX = 3.0;
    double  PixelSizeY = 3.0;
    double  PixelSpacingX = 0;
    double  PixelSpacingY = 0;
    bool    getPixelHit(double x, double y, size_t & binX, size_t & binY) const; // returns false if none was hit
    int     getPixelIndex(int binX, int binY) const {return PixelsX * binY + binX;}

    int     PDE_model = 0; // 0 - simplistic, 1 - interface-aware
    double  PDE_effective = 1.0;
    std::vector<std::pair<double,double>> PDE_spectral;
    double  getPDE(int iWave, int iSensMat) const;

    std::vector<std::pair<double,double>> AngularFactors;  // should be defined from 0 to 90.0 incidence angle
    double  getAngularFactor(double angle, int iSensorMat) const;  // incidence angle is [-90.0, 90.0]
    double  Angular_Wavelength = 600.0;

    std::vector<std::vector<double>> AreaFactors;
    double  StepX = 1.0;       // in mm
    double  StepY = 1.0;       // in mm
    bool    isAreaSensitive() const {return !AreaFactors.empty();}
    double  getAreaFactor(double x, double y) const;

    double  getMaxQE(bool bWaveRes) const;

    double  DarkCountRate = 0;      // counts per second
    double  IntegrationTime = 1e-6; // in seconds

    double  ElectronicNoiseSigma = 0;

    enum    EPhElToSignal {Constant, Normal, Gamma, Custom};
    EPhElToSignal PhElToSignalModel = Constant;
    //double  ElectronicGainFactor = 1.0;  // no more a part of sensor model --> standalone in SensorHub
    double  AverageSignalPerPhEl = 1.0;
    double  NormalSigma = 0;
    double  GammaShape  = 2.0;
    std::vector<std::pair<double,double>> SinglePhElPHS;
    double  convertHitsToSignal(double phel) const;
    //double  simulateDigitalization(double signal) const;  // not implemented

    QString updateRuntimeProperties(const std::vector<int> & seenSensorMats);

    void    clear();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    QString checkPDE_spectral() const;
    QString checkAngularFactors() const;
    QString checkAreaFactors() const;
    QString checkPhElToSignals() const;

    // --- runtime ---

    double _HalfSensitiveSizeX;
    double _HalfSensitiveSizeY;
    double _PixelPitchX;
    double _PixelPitchY;
    std::vector<double> PDEbinned;
    std::vector<double> AngularBinned; // binned from 0 to 90.0 degrees (91 bins of 1 degree)
    TH1D * _PHS = nullptr;
    double _AverageDarkCounts;
    double _PixelDarkFiringProbability;

    std::vector<std::pair<int, AInterfaceAwareRuntimeProps>> _InterfaceAwarePDEfactors; // {iMatSensor, data} --> cannot limit to one sensor material in the model: this is a property of each individual sensor

    double _MaxPDE_spectral = 1.0;
    double _MaxAngularFactor = 1.0;
    double _MaxAreaFactor = 1.0;

private:
    void updateInterfaceAwareRuntimeProps(const std::vector<int> & seenSensorMats);
};

#endif // ASENSORMODEL_H
