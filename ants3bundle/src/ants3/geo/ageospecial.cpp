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
    if (Type == "Scint")
    {
        AGeoScint * scint = new AGeoScint();
        scint->readFromJson(json);
        return scint;
    }
    if (Type == "PhotonFunctional")
    {
        AGeoPhotonFunctional * th = new AGeoPhotonFunctional();
        th->readFromJson(json);
        return th;
    }
    if (Type == "ParticleAnalyzer")
    {
        AGeoParticleAnalyzer * pa = new AGeoParticleAnalyzer();
        pa->readFromJson(json);
        return pa;
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
void AGeoCalorimeter::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    for (int i = 0; i <3 ; i++)
    {
        ok = GC.updateDoubleParameter(errorStr, Properties.strOrigin[i], Properties.Origin[i], false, false, false);
        if (!ok) errorStr += QString(" in Origin[%0]\n").arg(i);

        ok = GC.updateDoubleParameter(errorStr, Properties.strStep[i],   Properties.Step[i],   true,  true,  false);
        if (!ok) errorStr += QString(" in Step[%0]\n").arg(i);

        ok = GC.updateIntParameter(errorStr, Properties.strBins[i],   Properties.Bins[i],   true,  true);
        if (!ok) errorStr += QString(" in Bins[%0]\n").arg(i);
    }

    if (Properties.CollectDepoOverEvent)
    {
        ok = GC.updateIntParameter(errorStr, Properties.strEventDepoBins, Properties.EventDepoBins,  true, true);
        if (!ok) errorStr += " in event energy depo bins";

        ok = GC.updateDoubleParameter(errorStr, Properties.strEventDepoFrom, Properties.EventDepoFrom,  false, true, false);
        if (!ok) errorStr += " in event energy depo from";

        ok = GC.updateDoubleParameter(errorStr, Properties.strEventDepoTo,   Properties.EventDepoTo,    false, true, false);
        if (!ok) errorStr += " in event energy depo to";
    }
}

bool AGeoCalorimeter::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    for (size_t i = 0; i < 3; i++)
    {
        if (Properties.strOrigin[i].contains(nameRegExp)) return true;
        if (Properties.strStep[i]  .contains(nameRegExp)) return true;
        if (Properties.strBins[i]  .contains(nameRegExp)) return true;
    }
    if (Properties.strEventDepoBins.contains(nameRegExp)) return true;
    if (Properties.strEventDepoFrom.contains(nameRegExp)) return true;
    if (Properties.strEventDepoTo.contains(nameRegExp)) return true;
    return false;
}

void AGeoCalorimeter::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    for (size_t i = 0; i < 3; i++)
    {
        Properties.strOrigin[i].replace(nameRegExp, newName);
        Properties.strStep[i]  .replace(nameRegExp, newName);
        Properties.strBins[i]  .replace(nameRegExp, newName);
    }
    Properties.strEventDepoBins.replace(nameRegExp, newName);
    Properties.strEventDepoFrom.replace(nameRegExp, newName);
    Properties.strEventDepoTo.replace(nameRegExp, newName);
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

#include "aphotonfunctionalmodel.h"

AGeoPhotonFunctional::AGeoPhotonFunctional() :
    AGeoSpecial(), DefaultModel(new APFM_Dummy()) {}

AGeoPhotonFunctional::AGeoPhotonFunctional(const APhotonFunctionalModel & model) :
    AGeoSpecial()
{
    QJsonObject js;
    model.writeToJson(js);
    DefaultModel = APhotonFunctionalModel::factory(js);
}

void AGeoPhotonFunctional::readFromJson(const QJsonObject & json)
{
    QJsonObject js;
    jstools::parseJson(json, "Model", js);
    DefaultModel = APhotonFunctionalModel::factory(js);
}

void AGeoPhotonFunctional::doWriteToJson(QJsonObject & json) const
{
    QJsonObject js;
    DefaultModel->writeToJson(js);
    json["Model"] = js;
}

// ---

void AGeoParticleAnalyzer::readFromJson(const QJsonObject & json)
{
    Properties.readFromJson(json);
}

void AGeoParticleAnalyzer::doWriteToJson(QJsonObject & json) const
{
    Properties.writeToJson(json);
}

// ---
