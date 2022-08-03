#ifndef APHOTONSIMSETTINGS_H
#define APHOTONSIMSETTINGS_H

#include "afilesettingsbase.h"

#include <QString>
#include <QDateTime>

#include <vector>
#include <complex>
#include <set>

#include "TString.h"

// !!!*** implement usage of AErrorHub

class QJsonObject;

enum class EPhotSimType  {PhotonBombs, FromEnergyDepo, IndividualPhotons, FromLRFs};
enum class EBombGen      {Single, Grid, Flood, File};

class AWaveResSettings
{
public:
    bool   Enabled = false;

    double From = 200.0;  // in nm
    double To   = 800.0;
    double Step = 5.0;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json);

    int    countNodes() const;
    double toWavelength(int index) const;
    int    toIndex(double wavelength) const;   // TODO: compare with fast method!
    int    toIndexFast(double wavelength) const; //not safe
    // TODO: refactor:
    void   toStandardBins(const QVector<double> *wavelength, const QVector<double> *value, QVector<double>* binnedValue) const;
    void   toStandardBins(const std::vector<double> & wavelength, const std::vector<double> & value, std::vector<double> & binnedValue) const;
    void   toStandardBins(const std::vector<std::pair<double,std::complex<double>>> & waveReIm, std::vector<std::complex<double>> & reIm) const;

private:
    double getInterpolatedValue(double val, const QVector<double> *X, const QVector<double> *F) const;
    double getInterpolatedValue(double val, const std::vector<double> & X, const std::vector<double> & F) const;
};

class APhotOptSettings
{
public:
    int    MaxPhotonTransitions = 500;
    bool   CheckQeBeforeTracking   = false;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json);
};

typedef std::pair<double, double> ADPair;
class APhotonsPerBombSettings
{
public:
    enum EBombPhNumber {Constant, Poisson, Uniform, Normal, Custom};

    EBombPhNumber   Mode        = Constant;
    int             FixedNumber = 10;
    int             UniformMin  = 10;
    int             UniformMax  = 12;
    double          NormalMean  = 100.0;
    double          NormalSigma = 10.0;
    double          PoissonMean = 10.0;
    std::vector<ADPair> CustomDist;

    void    clearSettings();
    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

class ASingleSettings
{
public:
    double  Position[3];

    void    clearSettings();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

struct APhScanRecord
{
    bool   bEnabled  = false;
    bool   bBiDirect = false;
    int    Nodes     = 10;
    double DX        = 10.0;
    double DY        = 0;
    double DZ        = 0;
};
class AGridSettings
{
public:
    AGridSettings();

    double X0 = 0;
    double Y0 = 0;
    double Z0 = 0;
    APhScanRecord ScanRecords[3];

    int     getNumEvents() const;
    void    clearSettings();
    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

class AFloodSettings
{
public:
    enum AShapeEnum {Rectangular = 0, Ring = 1};
    enum AZEnum     {Fixed = 0, Range = 1};

    int        Number   = 100;
    AShapeEnum Shape    = Rectangular;
    double     Xfrom    = -15.0;
    double     Xto      =  15.0;
    double     Yfrom    = -15.0;
    double     Yto      =  15.0;
    double     X0       = 0;
    double     Y0       = 0;
    double     OuterDiameter   = 300.0;
    double     InnerDiameter   = 0;
    AZEnum     Zmode    = Fixed;
    double     Zfixed   = 0;
    double     Zfrom    = 0;
    double     Zto      = 0;

    void    clearSettings();
    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

class ABombFileSettings : public AFileSettingsBase
{
public:
    //void           clearStatistics(){}
};

class APhotonAdvancedSettings
{
public:
    enum AModeEnum {Isotropic, Fixed, Cone};

    AModeEnum DirectionMode = Isotropic;
    double    DirDX         = 0;
    double    DirDY         = 0;
    double    DirDZ         = 1.0;
    double    ConeAngle     = 10.0;

    bool      bFixWave      = false;
    int       WaveIndex     = -1;

    bool      bFixDecay     = false;
    double    DecayTime     = 5.0; // in ns

    bool      bOnlyVolume   = false;
    QString   Volume;
    bool      bOnlyMaterial = false;
    QString   Material;
    int       MaxNodeAttempts = 1000;

    void clear();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class APhotonBombsSettings
{
public:
    APhotonsPerBombSettings PhotonsPerBomb;

    EBombGen         GenerationMode = EBombGen::Single;

    ASingleSettings   SingleSettings;
    AGridSettings     GridSettings;
    AFloodSettings    FloodSettings;
    ABombFileSettings BombFileSettings;

    APhotonAdvancedSettings AdvancedSettings;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

class APhotonLogSettings   // !!!*** to RunSettings?
{
public:
    bool Save        = true;
    QString FileName = "PhotonLog.txt";

    std::set<int>        MustNotInclude_Processes; // v.fast
    std::vector<int>     MustInclude_Processes;    // slow
    std::set<TString>    MustNotInclude_Volumes;   // fast
    std::vector<TString> MustInclude_Volumes;      // v.slow
};

class APhotSimRunSettings
{
public:
    int     Seed = 0;

    int     EventFrom = 0;
    int     EventTo   = 0;

    QString OutputDirectory;

    bool    SaveSensorSignals     = true;
    QString FileNameSensorSignals = "SensorSignals.txt";

    bool    SaveSensorLog         = false;
    QString FileNameSensorLog     = "SensorLog.dat";
    bool    SensorLogTime         = true;
    bool    SensorLogXY           = false;
    bool    SensorLogAngle        = false;
    bool    SensorLogWave         = false;

    bool    SavePhotonBombs       = true;
    QString FileNamePhotonBombs   = "PhotonBombs.txt";

    bool    SaveTracks            = true;
    int     MaxTracks             = 1000;
    QString FileNameTracks        = "PhotonTracks.txt";

    bool    SaveStatistics        = true;
    QString FileNameStatistics    = "PhotonStatistics.json";
    double  UpperTimeLimit        = 100;

    bool    SavePhotonLog         = true;
    QString FileNamePhotonLog     = "PhotonLog.txt";

    bool    SaveMonitors          = true;
    QString FileNameMonitors      = "PhotonMonitors.txt";

    APhotonLogSettings LogSet;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class APhotonDepoSettings : public AFileSettingsBase
{
public:
    bool   Primary   = true;
    bool   Secondary = false;

    void   clear() override;

protected:
    void   doWriteToJson(QJsonObject & json) const override;
    void   doReadFromJson(const QJsonObject & json) override;
};

class APhotonFileSettings : public AFileSettingsBase
{
public:
    // so far no specific properties, so completely delegate to the base class!
};

// ===

class APhotonSimSettings
{
public:
    EPhotSimType         SimType = EPhotSimType::PhotonBombs;

    APhotonBombsSettings BombSet;
    APhotonDepoSettings  DepoSet;
    APhotonFileSettings  PhotFileSet;

    AWaveResSettings     WaveSet;
    APhotOptSettings     OptSet;

    APhotSimRunSettings  RunSet;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

#endif // APHOTONSIMSETTINGS_H
