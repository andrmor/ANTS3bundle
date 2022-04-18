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

    virtual QString getType() const = 0;

    virtual bool introduceGeoConstValues() = 0;

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

    bool introduceGeoConstValues() override {return true;}

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

    bool introduceGeoConstValues() override;

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

    bool introduceGeoConstValues() override {return true;}
};

#endif // AGEOSPECIAL_H
