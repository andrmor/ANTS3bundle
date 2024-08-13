#ifndef AGEOSPECIAL_H
#define AGEOSPECIAL_H

#include <QString>

class QJsonObject;
class AGeoSpecial;

namespace GeoRoleFactory
{
    AGeoSpecial * make(const QJsonObject & json);
}

class AGeoSpecial
{
public:
    AGeoSpecial(){}
    virtual ~AGeoSpecial(){}

    virtual QString getType() const = 0; // !!!*** convert to enum

    virtual void introduceGeoConstValues(QString & /*errorStr*/) {}

    virtual bool isGeoConstInUse(const QRegularExpression & /*nameRegExp*/) const {return false;}
    virtual void replaceGeoConstName(const QRegularExpression & /*nameRegExp*/, const QString & /*newName*/) {}

    void writeToJson(QJsonObject & json) const;
    virtual void readFromJson(const QJsonObject & /*json*/) {}
protected:
    virtual void doWriteToJson(QJsonObject & /*json*/) const {}
};

class AGeoSensor : public AGeoSpecial
{
public:
    AGeoSensor(){}
    AGeoSensor(int Model) : SensorModel(Model) {}

    QString getType() const override {return QStringLiteral("Sensor");}

    void readFromJson(const QJsonObject & json) override;
protected:
    void doWriteToJson(QJsonObject & json) const override;

public:
    int SensorModel = 0; // one is always defined (ideal sensor)
};

#include "acalorimeter.h"
class AGeoCalorimeter : public AGeoSpecial
{
public:
    AGeoCalorimeter(){}
    AGeoCalorimeter(const std::array<double, 3> & origin, const std::array<double, 3> & step, const std::array<int, 3> & bins);

    QString getType() const override {return QStringLiteral("Calorimeter");}

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void readFromJson(const QJsonObject & json) override;
protected:
    void doWriteToJson(QJsonObject & json) const override;

public:
    ACalorimeterProperties Properties;
};

class AGeoSecScint : public AGeoSpecial
{
public:
    AGeoSecScint(){}

    QString getType() const override {return QStringLiteral("SecScint");}
};

class AGeoScint : public AGeoSpecial
{
public:
    AGeoScint(){}

    QString getType() const override {return QStringLiteral("Scint");}
};

class APhotonFunctionalModel;
class AGeoPhotonFunctional : public AGeoSpecial
{
public:
    AGeoPhotonFunctional();
    AGeoPhotonFunctional(const APhotonFunctionalModel & model);

    QString getType() const override {return QStringLiteral("PhotonFunctional");}

    APhotonFunctionalModel * DefaultModel = nullptr;

    void readFromJson(const QJsonObject & json) override;
protected:
    void doWriteToJson(QJsonObject & json) const override;
};

class AGeoParticleAnalyzer : public AGeoSpecial
{
public:
    AGeoParticleAnalyzer(){}

    QString getType() const override {return QStringLiteral("ParticleAnalyzer");}

    void readFromJson(const QJsonObject & json) override;
protected:
    void doWriteToJson(QJsonObject & json) const override;

public:
    int     EnergyBins  = 100;
    double  EnergyFrom  = 0;
    double  EnergyTo    = 1.0;
    QString EnergyUnits = "keV";

    bool    UseTimeWindow  = false;
    double  TimeWindowFrom = 0;
    double  TimeWindowTo   = 100.0; // ns

    bool    StopTracking = false;

    bool    SingleInstanceForAllCopies = false;

    static bool isAllowedEnergyUnit(const QString & str);
};

#endif // AGEOSPECIAL_H
