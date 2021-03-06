#include "acalsettings.h"

#ifdef JSON11
#else
    #include "acalorimeterhub.h"
#include "acalorimeter.h"
    #include "ajsontools.h"
    #include "aerrorhub.h"
#endif

#include <cmath>

bool ACalorimeterProperties::operator ==(const ACalorimeterProperties & other) const
{
    for (int i = 0; i < 3; i++)
    {
        if (Origin[i] != other.Origin[i]) return false;
        if (Step[i]   != other.Step[i])   return false;
        if (Bins[i]   != other.Bins[i])   return false;

#ifndef JSON11
        if (strOrigin[i] != other.strOrigin[i]) return false;
        if (strStep[i]   != other.strStep[i])   return false;
        if (strBins[i]   != other.strBins[i])   return false;
#endif
    }
    return true;
}

bool ACalorimeterProperties::operator !=(const ACalorimeterProperties & other) const
{
    return !operator==(other);
}

int ACalorimeterProperties::getNumDimensions() const
{
    int numDim = 3;
    for (int i = 0; i < 3; i++)
        if (Bins[i] == 1) numDim--;
    return numDim;
}

bool ACalorimeterProperties::isAxisOff(int index) const
{
    if (index < 0 || index > 2) return false;

    return (Origin[index] == -1e10 && Step[index] == 2e10 && Bins[index] == 1);
}

#ifdef JSON11
void ACalorimeterProperties::writeToJson(json11::Json::object & json) const
#else
void ACalorimeterProperties::writeToJson(QJsonObject & json) const
#endif
{
#ifdef JSON11
    json11::Json::array arO;
    json11::Json::array arS;
    json11::Json::array arB;
#else
    QJsonArray arO;
    QJsonArray arS;
    QJsonArray arB;
#endif
    for (int i=0; i<3; i++)
    {
        arO.push_back(Origin[i]);
        arS.push_back(Step[i]);
        arB.push_back(Bins[i]);
    }
    json["Origin"] = arO;
    json["Step"] = arS;
    json["Bins"] = arB;

#ifndef JSON11
    QJsonArray toAr, tsAr, tbAr;
    for (int i=0; i<3; i++)
    {
        toAr.push_back(strOrigin[i]);
        tsAr.push_back(strStep[i]);
        tbAr.push_back(strBins[i]);
    }
    if (strOrigin != std::array<QString,3>{"", "", ""}) json["strOrigin"] = toAr;
    if (strStep   != std::array<QString,3>{"", "", ""}) json["strStep"]   = tsAr;
    if (strBins   != std::array<QString,3>{"", "", ""}) json["strBins"]   = tbAr;
#endif
}

#ifdef JSON11
void ACalorimeterProperties::readFromJson(const json11::Json::object & json)
{
    {
        json11::Json::array ar;
        jstools::parseJson(json, "Origin", ar);
        for (int i=0; i<3; i++)
            Origin[i] = ar[i].number_value();
    }

    {
        json11::Json::array ar;
        jstools::parseJson(json, "Step", ar);
        for (int i=0; i<3; i++)
            Step[i] = std::fabs(ar[i].number_value());
    }

    {
        json11::Json::array ar;
        jstools::parseJson(json, "Bins", ar);
        for (int i=0; i<3; i++)
            Bins[i] = ar[i].int_value();
    }
}
#else
void ACalorimeterProperties::readFromJson(const QJsonObject & json)
{
    {
        QJsonArray ar;
        jstools::parseJson(json, "Origin", ar);
        if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json Origin");
        else
            for (int i=0; i<3; i++)
                Origin[i] = ar[i].toDouble();
    }

    {
        QJsonArray ar;
        jstools::parseJson(json, "Step", ar);
        if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json Step");
        else
            for (int i=0; i<3; i++)
                Step[i] = std::fabs(ar[i].toDouble());
    }

    {
        QJsonArray ar;
        jstools::parseJson(json, "Bins", ar);
        if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json Bins");
        else
            for (int i=0; i<3; i++)
                Bins[i] = ar[i].toInt(1);
    }

    // str for Geo Consts
    strOrigin = {"", "", ""};
    strStep   = {"", "", ""};
    strBins   = {"", "", ""};
    {
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "strOrigin", ar);
        if (ok)
        {
            if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json strOrigin");
            else
            {
                for (int i=0; i<3; i++)
                    strOrigin[i] = ar[i].toString();
            }
        }
    }
    {
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "strStep", ar);
        if (ok)
        {
            if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json strStep");
            else
            {
                for (int i=0; i<3; i++)
                    strStep[i] = ar[i].toString();
            }
        }
    }
    {
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "strBins", ar);
        if (ok)
        {
            if (ar.size() < 3) AErrorHub::addQError("Invalid size for AGeoCalorimeter json strBins");
            else
            {
                for (int i=0; i<3; i++)
                    strBins[i] = ar[i].toString();
            }
        }
    }
}
#endif

// ---

#ifndef JSON11
void ACalSetRecord::writeToJson(QJsonObject &json) const
{
    json["Name"]     = Name.data();
    //json["Particle"] = Particle.data();
    json["Index"]    = Index;

    QJsonObject js;
        Properties.writeToJson(js);
    json["Properties"] = js;
}
#endif

#ifdef JSON11
void ACalSetRecord::readFromJson(const json11::Json::object & json)
#else
void ACalSetRecord::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "Name",     Name);
    //jstools::parseJson(json, "Particle", Particle);
    jstools::parseJson(json, "Index",    Index);

#ifdef JSON11
    json11::Json::object js;
    jstools::parseJson(json, "Properties", js);
    Properties.readFromJson(js);
#endif
    //no need to read Properties on ANTS3 side
}

// ---

#ifndef JSON11
void ACalSettings::writeToJson(QJsonObject & json) const
{
    json["Enabled"] = Enabled;
    json["FileName"] = FileName.data();

    QJsonArray ar;
    for (const ACalSetRecord & c : Calorimeters)
    {
        QJsonObject js;
        c.writeToJson(js);
        ar.append(js);
    }
    json["Calorimeters"] = ar;
}
#endif

#ifdef JSON11
void ACalSettings::readFromJson(const json11::Json::object & json)
#else
void ACalSettings::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "Enabled",  Enabled);
    jstools::parseJson(json, "FileName", FileName);

    Calorimeters.clear();
#ifdef JSON11
    json11::Json::array calArray;
    jstools::parseJson(json, "Calorimeters", calArray);
    for (size_t i = 0; i < calArray.size(); i++)
    {
        json11::Json::object mjs = calArray[i].object_items();

        ACalSetRecord r;
            r.readFromJson(mjs);
        Calorimeters.push_back(r);
    }
#endif
    // no need to read configs on ANTS3 side
}

#ifndef JSON11
void ACalSettings::initFromHub()
{
    Calorimeters.clear();
    const std::vector<ACalorimeterData> & CalorimeterRecords = ACalorimeterHub::getConstInstance().Calorimeters;
    for (int iCal = 0; iCal < (int)CalorimeterRecords.size(); iCal++)
    {
        const ACalorimeterData & cal = CalorimeterRecords[iCal];

        ACalSetRecord r;
        r.Name       = cal.Name.toLatin1().data();
        //r.Particle = mc.Particle.toLatin1().data();
        r.Index      = iCal;
        r.Properties = cal.Calorimeter->Properties;
        Calorimeters.push_back(r);
    }
}
#endif
