#ifndef ASENSORMODEL_H
#define ASENSORMODEL_H

#include <QString>
#include <vector>

class QJsonObject;
class TH1D;

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

    double  PDE_effective = 1.0;
    std::vector<std::pair<double,double>> PDE_spectral;
    double  getPDE(int iWave) const;

    std::vector<std::pair<double,double>> AngularFactors;  // should be defined from 0 to 90.0 incidence angle
    double  getAngularFactor(double angle) const;  // incidence angle is [-90.0, 90.0]

    std::vector<std::vector<double>> AreaFactors;
    double  StepX = 1.0;       // in mm
    double  StepY = 1.0;       // in mm
    bool    isAreaSensitive() const {return !AreaFactors.empty();}
    double  getAreaFactor(double x, double y) const;

    double  DarkCountRate = 0;      // counts per second
    double  IntegrationTime = 1e-6; // in seconds

    double  ElectronicNoiseSigma = 0;

    enum    EPhElToSignal {Constant, Normal, Gamma, Custom};
    EPhElToSignal PhElToSignalModel = Constant;
    double  ElectronicGainFactor = 1.0;
    double  AverageSignalPerPhEl = 1.0;
    double  NormalSigma = 0;
    double  GammaShape  = 2.0;
    std::vector<std::pair<double,double>> SinglePhElPHS;
    double  convertHitsToSignal(double phel) const;

    void    updateRuntimeProperties();

    void    clear();

    void    writeToJson(QJsonObject & json) const;
    bool    readFromJson(const QJsonObject & json); // !!!***

    //check-ups !!!*** to combine to code performed before sim start!
    QString checkPDE_spectral() const;
    QString checkAngularFactors() const;
    QString checkAreaFactors() const;
    QString checkPhElToSignals() const;

    //runtime
    double _HalfSensitiveSizeX;
    double _HalfSensitiveSizeY;
    double _PixelPitchX;
    double _PixelPitchY;
    std::vector<double> PDEbinned;
    std::vector<double> AngularBinned; // binned from 0 to 90.0 degrees (91 bins of 1 degree)
    TH1D * _PHS = nullptr;
    double _AverageDarkCounts;
    double _PixelDarkFiringProbability;

};

#endif // ASENSORMODEL_H
