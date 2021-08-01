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

    void writeToJson(QJsonObject & json) const;
    virtual void readFromJson(const QJsonObject & /*json*/) {}
protected:
    virtual void doWriteToJson(QJsonObject & /*json*/) const {};
};

class AGeoSensor : public AGeoSpecial
{
public:
    AGeoSensor(){}

    QString getType() const override {return QStringLiteral("Sensor");};

    void readFromJson(const QJsonObject & json) override;
protected:
    void doWriteToJson(QJsonObject & json) const override;

public:
    int SensorType = -1; // not defined

    // runtime, not saved
    int Index;
};

class AGeoCalorimeter : public AGeoSpecial
{
public:
    AGeoCalorimeter(){}

    QString getType() const override {return QStringLiteral("Calorimeter");};
};

class AGeoSecScint : public AGeoSpecial
{
public:
    AGeoSecScint(){}

    QString getType() const override {return QStringLiteral("SecScint");};
};

#endif // AGEOSPECIAL_H
