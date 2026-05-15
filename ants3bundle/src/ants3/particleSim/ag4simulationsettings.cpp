#include "ag4simulationsettings.h"

#ifndef JSON11
#include "ajsontools.h"
#endif

#ifndef JSON11
void AG4SimulationSettings::writeToJson(QJsonObject &json) const
{
    json["PhysicsList"] = QString(PhysicsList.data());

    QJsonArray arC;
    for (auto & c : Commands)
        arC.push_back(QString(c.data()));
    json["Commands"] = arC;

    QJsonArray arSL;
    for (const auto & it : StepLimits)
    {
        QJsonArray el;
            el << QString(it.first.data()) << it.second;
        arSL.push_back(el);
    }
    json["StepLimits"] = arSL;

    json["UseTSphys"] = UseTSphys;
    json["UseNCrystal"] = UseNCrystal;

    json["SimulateAnnihilAcolinearity"] = SimulateAnnihilAcolinearity;
    json["AcolinearityModel"] = AcolinearityModel;
    json["AcolinearityFWHM"] = AcolinearityFWHM;
    {
        QJsonArray ar;
        for (const std::string & str : AcolinearityVolumes)
            ar.push_back(QString(str.data()));
        json["AcolinearityVolumes"] = ar;
    }
}
#endif

#ifdef JSON11
void AG4SimulationSettings::readFromJson(const json11::Json::object & json)
#else
void AG4SimulationSettings::readFromJson(const QJsonObject &json)
#endif
{
    clear();

    jstools::parseJson(json, "PhysicsList", PhysicsList);
    jstools::parseJson(json, "UseTSphys",   UseTSphys);
    jstools::parseJson(json, "UseNCrystal", UseNCrystal);

    Commands.clear();
#ifdef JSON11
    json11::Json::array arC;
#else
    QJsonArray arC;
#endif
    jstools::parseJson(json, "Commands", arC);
    for (int i = 0; i < (int)arC.size(); i++)
    {
        std::string com;
#ifdef JSON11
        com = arC[i].string_value();
#else
        com = arC.at(i).toString().toLatin1().data();
#endif
        Commands.push_back(com);
    }

#ifdef JSON11
    json11::Json::array arSL;
#else
    QJsonArray arSL;
#endif
    jstools::parseJson(json, "StepLimits", arSL);
    for (int i = 0; i < (int)arSL.size(); i++)
    {
#ifdef JSON11
        json11::Json::array el = arSL[i].array_items();
        if (el.size() > 1)
        {
            const std::string vol = el[0].string_value();
            const double step = el[1].number_value();
            StepLimits[vol] = step;
        }

#else
        QJsonArray el = arSL[i].toArray();
        if (el.size() > 1)
        {
            const QString vol = el[0].toString();
            const double step = el[1].toDouble();
            StepLimits[vol.toLatin1().data()] = step;
        }
#endif
    }

    jstools::parseJson(json, "SimulateAnnihilAcolinearity", SimulateAnnihilAcolinearity);
    jstools::parseJson(json, "AcolinearityModel", AcolinearityModel);
    jstools::parseJson(json, "AcolinearityFWHM", AcolinearityFWHM);
    {
        AcolinearityVolumes.clear();
#ifdef JSON11
        json11::Json::array ar;
#else
        QJsonArray ar;
#endif
        jstools::parseJson(json, "AcolinearityVolumes", ar);
        for (int i = 0; i < (int)ar.size(); i++)
        {
            std::string vol;
#ifdef JSON11
            vol = ar[i].string_value();
#else
            vol = ar.at(i).toString().toLatin1().data();
#endif
            AcolinearityVolumes.push_back(vol);
        }
    }
}

void AG4SimulationSettings::clear()
{
    PhysicsList = "QGSP_BIC_HP";
    UseTSphys = false;

    Commands = {"/run/setCut 0.7 mm"};
    StepLimits.clear();

    SimulateAnnihilAcolinearity = false;
    AcolinearityModel = 0;
    AcolinearityFWHM = 0.5;
    AcolinearityVolumes.clear();
}
