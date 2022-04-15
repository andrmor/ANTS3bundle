#include "ageospecial.h"
#include "ajsontools.h"
#include "aerrorhub.h"

AGeoSpecial * GeoRoleFactory::make(const QJsonObject & json)
{
    QString Type;
    jstools::parseJson(json, "Type", Type);

    if (Type == "Sensor")
    {
        AGeoSensor * sens = new AGeoSensor();
        sens->readFromJson(json);
        return sens;
    }
    if (Type == "Calorimeter")
    {
        AGeoCalorimeter * cal = new AGeoCalorimeter();
        cal->readFromJson(json);
        return cal;
    }
    if (Type == "SecScint")
    {
        AGeoSecScint * sec = new AGeoSecScint();
        sec->readFromJson(json);
        return sec;
    }

    return nullptr;
}

// ---

void AGeoSpecial::writeToJson(QJsonObject & json) const
{
    json["Type"] = getType();

    doWriteToJson(json);
}

// ---

void AGeoSensor::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "SensorModel", SensorModel);
}

void AGeoSensor::doWriteToJson(QJsonObject & json) const
{
    json["SensorModel"] = SensorModel;
}

// ---

AGeoCalorimeter::AGeoCalorimeter(const std::array<double, 3> & origin, const std::array<double, 3> & step, const std::array<int, 3> & bins) :
    Properties(origin, step, bins){}

void AGeoCalorimeter::readFromJson(const QJsonObject & json)
{
    Properties.readFromJson(json);
}

void AGeoCalorimeter::doWriteToJson(QJsonObject & json) const
{
    Properties.writeToJson(json);
}

// ---
