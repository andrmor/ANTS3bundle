#ifndef APHOTONSIMSETTINGS_H
#define APHOTONSIMSETTINGS_H

#include <QVector> // TODO: std::vector !!!***
#include <QString>

class QJsonObject;

enum class EPhotSimType  {PhotonBombs, FromEnergyDepo, IndividualPhotons, FromLRFs};
enum class EBombPhNumber {Constant, Poisson, Uniform, Normal, Custom};
enum class EBombGen      {Single, Grid, Flood, File, Script};

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
    void   toStandardBins(const QVector<double> *sp_x, const QVector<double> *sp_y, QVector<double>* y) const;

private:
    double getInterpolatedValue(double val, const QVector<double> *X, const QVector<double> *F) const;
};

class APhotOptSettings
{
public:
    int    MaxPhotonTransitions = 500;
    bool   CheckQeBeforeTracking   = false;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json);
};

class APhotonBombsSettings
{
public:

    // TODO reformat!

    // Number of photons
    EBombPhNumber       PhotonNumberMode = EBombPhNumber::Constant;

    // generation type
    EBombGen            GenerationMode   = EBombGen::Single;

    // Single
    double Position[3];

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

class APhotSimRunSettings
{
public:
    int     EventFrom;
    int     EventTo;

    bool    SaveSensorSignals = true;
    QString FileNameSensorSignals = "SensorSignals";

    bool    SaveTracks = true;
    int     MaxTracks;
    QString FileNameTracks;

    bool    SavePhotonLog = true;
    QString FileNamePhotonLog;

    void    writeToJson(QJsonObject & json) const;
    void    readFromJson(const QJsonObject & json);
};

// ===

class APhotonSimSettings
{
public:
    EPhotSimType         SimType = EPhotSimType::PhotonBombs;

    APhotonBombsSettings BombSet;

    AWaveResSettings     WaveSet;
    APhotOptSettings     OptSet;

    APhotSimRunSettings  RunSet;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);
};

#endif // APHOTONSIMSETTINGS_H
