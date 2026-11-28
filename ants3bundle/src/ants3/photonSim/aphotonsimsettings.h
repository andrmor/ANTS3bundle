#ifndef APHOTONSIMSETTINGS_H
#define APHOTONSIMSETTINGS_H

#include "afilesettingsbase.h"
#include "afloodruntime.h"

#include <QString>
#include <QDateTime>

#include <vector>
#include <complex>
#include <set>

#include "TString.h"

// !!!*** AErrorHub

class QJsonObject;

enum class EPhotSimType  {PhotonBombs, FromEnergyDepo, IndividualPhotons};
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

    enum   EConverterOptions {ExpandWithZero, ExpandWithLastValues}; // policy to fill binned values outside the defined range in wavelength
    // "contract" is the following:
    // probability-related parameters (e.g. emission spectra, PDE, reemission probability) expand with zeros
    // all other types of parameters  (e.g. index, coefficient, angle) expand with last values
    void   toStandardBins(const std::vector<double> & wavelength, const std::vector<double> & value, std::vector<double> & binnedValue, EConverterOptions options) const;
    void   toStandardBins(const std::vector<std::pair<double,double>> & waveAndData, std::vector<double> & binnedValue, EConverterOptions options) const;
    void   toStandardBins(const std::vector<std::pair<double,std::complex<double>>> & waveReIm, std::vector<std::complex<double>> & reIm, EConverterOptions options) const;

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

    enum EPhotonTracingMode {Normal, CheckQeBefore, LRF};
    EPhotonTracingMode TracingMode = Normal;

    int    LRF_photonsPerNode = 20000;
    double LRF_photoElectrons = 1.0;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json); // !!!*** error reporting

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
    QString PositionStr[3];

    void    clearSettings();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    updateGeoConstRelatedSimProperties();
    QString isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);
};

struct APhScanRecord
{
    bool   bEnabled  = false;
    bool   bBiDirect = false;
    int    Nodes     = 10;
    double DX        = 10.0;    QString DXStr;
    double DY        = 0;       QString DYStr;
    double DZ        = 0;       QString DZStr;
};
class AGridSettings
{
public:
    AGridSettings();

    double X0 = 0;  QString X0Str;
    double Y0 = 0;  QString Y0Str;
    double Z0 = 0;  QString Z0Str;

    APhScanRecord ScanRecords[3];

    int     getNumEvents() const;

    void    clearSettings();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    updateGeoConstRelatedSimProperties();
    QString isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);
};

struct AFloodProfile
{
    enum EType          {Uniform = 0, Exponential = 1, Gauss = 2, Formula = 3, Table = 4};
    enum EDirection     {Plus = 0, Minus = 1};
    enum ETableMode     {Points = 0, Bins = 1};
    enum ERadialMeaning {Density = 0, Distribution = 1};

    EType          Type              = Uniform;
    double         AttenuationLength = 10.0;     QString AttenuationLengthStr;
    EDirection     Direction         = Plus;     // JSON "+" / "-": beam travel direction along the axis
    double         Mean              = 0;        QString MeanStr;  // local coordinate
    double         Sigma             = 1.0;      QString SigmaStr;
    QString        FormulaText;                  // TFormula in x (local coordinate, r for radial profiles)
    int            SamplingBins      = 0;        // 0 = automatic grid refinement
    std::vector<std::pair<double,double>> Data;  // x = local coordinate
    ETableMode     TableMode         = Points;
    ERadialMeaning RadialMeaning     = Density;

    void    clear();
    void    writeToJson(QJsonObject & json, bool radial) const;
    QString readFromJson(const QJsonObject & json);   // returns error, "" if ok

    void    updateGeoConstRelatedSimProperties(const QString & name);
    QString isGeoConstInUse(const QRegularExpression & nameRegExp, const QString & name) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);

    static QString typeToString(EType type);
};

class AFloodSettings
{
public:
    enum EShape {Box = 0, Cylinder = 1, Sphere = 2}; // 0 and 1 keep the indices of the old Rectangular / Ring
    enum EAxis  {AxisX = 0, AxisY = 1, AxisZ = 2};

    int     Number = 100;
    EShape  Shape  = Box;
    EAxis   Axis   = AxisZ;   // cylinder

    // Position = geometric centre and origin of the local coordinates (JSON keys CenterX/Y/Z)
    double  X0 = 0;           QString X0Str;
    double  Y0 = 0;           QString Y0Str;
    double  Z0 = 0;           QString Z0Str;

    // box
    double  SizeX = 30.0;     QString SizeXStr;
    double  SizeY = 30.0;     QString SizeYStr;
    double  SizeZ = 0;        QString SizeZStr;

    // cylinder (Length is along Axis) and sphere
    double  Length = 0;              QString LengthStr;
    double  OuterDiameter = 300.0;   QString OuterDiameterStr;
    double  InnerDiameter = 0;       QString InnerDiameterStr;

    AFloodProfile ProfileX, ProfileY, ProfileZ;   // box
    AFloodProfile ProfileRadial, ProfileAxial;    // cylinder (both) and sphere (radial)

    void    clearSettings();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    updateGeoConstRelatedSimProperties();
    QString isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);

    // runtime
    QString updateRuntimeProperties(QString * warnings = nullptr);   // validates, builds samplers; returns error, "" if ok
    bool    generatePosition(double * r) const;                       // global coordinates, no material/volume check
    const AFloodRuntime & getRuntime() const {return _Runtime;}

    static QString shapeToString(EShape shape);
    static QString axisToString(EAxis axis);

private:
    QString readLegacyJson(const QJsonObject & json, const QString & shapeStr);

    AFloodRuntime _Runtime;  // never serialized
};

class ABombFileSettings : public AFileSettingsBase
{
public:
    //void           clearStatistics(){}
};

class APhotonBombAdvancedSettings
{
public:
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

    APhotonBombAdvancedSettings AdvancedSettings;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    clear();

    void    updateGeoConstRelatedSimProperties();
    QString isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);
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
    QString FileNameMonitors      = "PhotonMonitors.json";
    bool    SaveMonitorLog        = false;
    QString FileNameMonitorLog    = "PhotonMonitorLog.dat";

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
    //bool   Primary   = true;
    //bool   Secondary = false;
    void   clear() override;

protected:
    void   doWriteToJson(QJsonObject & json) const override;
    void   doReadFromJson(const QJsonObject & json) override;
};

class APhotonFileSettings : public AFileSettingsBase
{
public:
    // there are no specific properties --> delegate to the base class
};

class APhGenOverrideSettings
{
public:
    enum AModeEnum {Isotropic, Fixed, Cone};

    AModeEnum DirectionMode = Isotropic;
    double    DirDX         = 0;
    double    DirDY         = 0;
    double    DirDZ         = 1.0;
    double    ConeAngle     = 10.0;

    bool      bFixWave      = false;
    double    FixedWavelength = 550.0;

    bool      bFixDecay     = false;
    double    DecayTime     = 0; // in ns

    void clear();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

// ===

class APhotonSimSettings
{
public:
    bool PrimaryScint   = true;
    bool SecondaryScint = false;

    EPhotSimType         SimType = EPhotSimType::PhotonBombs;

    APhotonBombsSettings BombSet;
    APhotonDepoSettings  DepoSet;
    APhotonFileSettings  PhotFileSet;

    AWaveResSettings     WaveSet;
    APhotOptSettings     OptSet;

    APhotSimRunSettings  RunSet;

    APhGenOverrideSettings PhGenOverrideSet;

    void    writeToJson(QJsonObject & json, bool addRuntimeExport) const;
    QString readFromJson(const QJsonObject & json);

    void    clear();
};

#endif // APHOTONSIMSETTINGS_H
