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

void AGeoCalorimeter::readFromJson(const QJsonObject & json)
{
    {
        QJsonArray ar;
        jstools::parseJson(json, "Origin", ar);
        if (ar.size() < 3) AErrorHub::addQError("Bad dimension for AGeoCalorimeter json Origin");
        else
            for (int i=0; i<3; i++)
                Origin[i] = ar[i].toDouble();
    }

    {
        QJsonArray ar;
        jstools::parseJson(json, "Step", ar);
        if (ar.size() < 3) AErrorHub::addQError("Bad dimension for AGeoCalorimeter json Step");
        else
            for (int i=0; i<3; i++)
                Step[i] = ar[i].toDouble();
    }

    {
        QJsonArray ar;
        jstools::parseJson(json, "Bins", ar);
        if (ar.size() < 3) AErrorHub::addQError("Bad dimension for AGeoCalorimeter json Bins");
        else
            for (int i=0; i<3; i++)
                Bins[i] = ar[i].toInt(1);
    }
}

void AGeoCalorimeter::doWriteToJson(QJsonObject & json) const
{
    {
        QJsonArray ar;
        for (int i=0; i<3; i++) ar << Origin[i];
        json["Origin"] = ar;
    }

    {
        QJsonArray ar;
        for (int i=0; i<3; i++) ar << Step[i];
        json["Step"] = ar;
    }

    {
        QJsonArray ar;
        for (int i=0; i<3; i++) ar << Bins[i];
        json["Bins"] = ar;
    }
}

// ---
