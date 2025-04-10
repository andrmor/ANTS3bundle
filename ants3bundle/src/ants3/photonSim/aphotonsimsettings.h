#ifndef APHOTONSIMSETTINGS_H
#define APHOTONSIMSETTINGS_H

#include "afilesettingsbase.h"

#include <QString>
#include <QDateTime>

#include <vector>
#include <complex>
#include <set>

#include "TString.h"

// !!!*** AErrorHub

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

    void   clear();

    int    countNodes() const;
    double toWavelength(int index) const;
    void   toWavelength(std::vector<std::pair<double, double>> & waveIndex_I_pairs) const;
    int    toIndex(double wavelength) const;   // TODO: compare with fast method!
    int    toIndexFast(double wavelength) const; //not safe

    void   toStandardBins(const std::vector<double> & wavelength, const std::vector<double> & value, std::vector<double> & binnedValue) const;
    void   toStandardBins(const std::vector<std::pair<double,double>> & waveAndData, std::vector<double> & binnedValue) const;
    void   toStandardBins(const std::vector<std::pair<double,std::complex<double>>> & waveReIm, std::vector<std::complex<double>> & reIm) const;

    void   getWavelengthBins(std::vector<double> & wavelength) const;
    std::vector<double> getVectorOfIndexes() const;

public:
    static double getInterpolatedValue(double val, const std::vector<double> & X, const std::vector<double> & F);  // !!!*** consider moving to some tool header
    static double getInterpolatedValue(double val, const std::vector<std::pair<double,double>> & F);  // !!!*** consider moving to some tool header
};

class APhotOptSettings
{
public:
    int    MaxPhotonTransitions  = 500;
    bool   CheckQeBeforeTracking = false;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json);

    void   clear();
};

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
    std::vector<std::pair<int,double>> CustomDist;

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
    //int       WaveIndex     = -1;
    double    FixedWavelength = 550.0;

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

    EBombGen          GenerationMode = EBombGen::Single;

    ASingleSettings   SingleSettings;
    AGridSettings     GridSettings;
    AFloodSettings    FloodSettings;
    ABombFileSettings BombFileSettings;

    APhotonAdvancedSettings AdvancedSettings;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    clear();
};

class AVolumeIndexPair
{
public:
    AVolumeIndexPair(TString volume, int index) : Volume(volume), Index(index) {}
    AVolumeIndexPair() = default;

    TString Volume;
    int Index;
};

class APhotonLogSettings
{
public:
    bool    Enabled  = false;
    QString FileName = "PhotonLog.txt";

    int MaxNumber = 1000;

    std::set<int>                 MustNotInclude_Processes; // v.fast
    std::vector<int>              MustInclude_Processes;    // slow
    std::set<TString>             MustNotInclude_Volumes;   // fast
    std::vector<AVolumeIndexPair> MustInclude_Volumes;      // v.slow

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();
};

class APhotSimRunSettings
{
public:
    int     Seed = 0;

    int     EventFrom = 0;
    int     EventTo   = 0;

    QString OutputDirectory;
    bool    BinaryFormat = false;

    QString FileNameReceipt       = "DummyReceipt.txt";

    bool    SaveSensorSignals     = false;
    QString FileNameSensorSignals = "SensorSignals.txt";

    bool    SaveSensorLog         = false;
    QString FileNameSensorLog     = "SensorLog.dat";
    bool    SensorLogTime         = true;
    bool    SensorLogXY           = false;
    bool    SensorLogAngle        = false;
    bool    SensorLogWave         = false;

    bool    SavePhotonBombs       = false;
    QString FileNamePhotonBombs   = "PhotonBombs.txt";

    bool    SaveTracks            = false;
    int     MaxTracks             = 1000;
    QString FileNameTracks        = "PhotonTracks.txt";

    bool    SaveStatistics        = true;
    QString FileNameStatistics    = "PhotonStatistics.json";
    double  UpperTimeLimit        = 1e9;

    bool    SaveMonitors          = false;
    QString FileNameMonitors      = "PhotonMonitors.txt";

    bool    SaveConfig            = false;
    QString FileNameConfig        = "Config_OpticalSim.json";

    APhotonLogSettings PhotonLogSet;

    void writeToJson(QJsonObject & json, bool addRuntimeExport) const;
    void readFromJson(const QJsonObject & json);

    void clear();
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

    void    writeToJson(QJsonObject & json, bool addRuntimeExport) const;
    QString readFromJson(const QJsonObject & json);

    void    clear();
};

#endif // APHOTONSIMSETTINGS_H
