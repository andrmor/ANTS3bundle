#include "acalsettings.h"

#ifdef JSON11
#else
    #include "acalorimeterhub.h"
#include "acalorimeter.h"
    #include "ajsontools.h"
    #include "aerrorhub.h"
#endif

#ifndef JSON11
void ACalorimeterProperties::writeToJson(QJsonObject & json) const
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
#endif

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
            Step[i] = ar[i].number_value();
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
