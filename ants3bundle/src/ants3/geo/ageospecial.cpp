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

#include "ageoconsts.h"
bool AGeoCalorimeter::introduceGeoConstValues()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    // TODO: enable error control !!!***

    bool ok;
    QString err;
    bool bErr = false;
    for (int i = 0; i <3 ; i++)
    {
        ok = GC.updateParameter(err, Properties.strOrigin[i], Properties.Origin[i], false, false, false);
        if (!ok) {AErrorHub::addQError(err); bErr = true;}
        ok = GC.updateParameter(err, Properties.strStep[i],   Properties.Step[i],   true,  true,  false);
        if (!ok) {AErrorHub::addQError(err); bErr = true;}
        ok = GC.updateParameter(err, Properties.strBins[i],   Properties.Bins[i],   true,  true);
        if (!ok) {AErrorHub::addQError(err); bErr = true;}
    }
    return bErr;
}

void AGeoCalorimeter::readFromJson(const QJsonObject & json)
{
    Properties.readFromJson(json);
}

void AGeoCalorimeter::doWriteToJson(QJsonObject & json) const
{
    Properties.writeToJson(json);
}

// ---
