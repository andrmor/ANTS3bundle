#ifndef APHOTSIMSETTINGS_H
#define APHOTSIMSETTINGS_H

#include <QVector> // TODO: std::vector !!!***

class QJsonObject;

enum class APhotSinTypeEnum {PhotonBombs, FromEnergyDepo, IndividualPhotons, FromLRFs};

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
    double getWavelength(int index) const;
    int    getIndex(double wavelength) const;   // TODO: compare with fast method!
    int    getIndexFast(double wavelength) const; //not safe
    // TODO: refactor 2 below:
    void   convertToStandardWavelengthes(const QVector<double> *sp_x, const QVector<double> *sp_y, QVector<double>* y) const;
    double getInterpolatedValue(double val, const QVector<double> *X, const QVector<double> *F) const;
};

class APhotSimSettings final
{
public:
    static       APhotSimSettings & getInstance();
    static const APhotSimSettings & getConstInstance();

private:
    APhotSimSettings(){}
    ~APhotSimSettings(){}

    APhotSimSettings(const APhotSimSettings&)            = delete;
    APhotSimSettings(APhotSimSettings&&)                 = delete;
    APhotSimSettings& operator=(const APhotSimSettings&) = delete;
    APhotSimSettings& operator=(APhotSimSettings&&)      = delete;

public:
    APhotSinTypeEnum SimType = APhotSinTypeEnum::PhotonBombs;

    AWaveResSettings WaveSet;

    void   writeToJson(QJsonObject & json) const;
    void   readFromJson(const QJsonObject & json);

};

#endif // APHOTSIMSETTINGS_H
