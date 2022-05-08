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

    virtual void introduceGeoConstValues(QString & /*errorStr*/) {}

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

#endif // AGEOSPECIAL_H
