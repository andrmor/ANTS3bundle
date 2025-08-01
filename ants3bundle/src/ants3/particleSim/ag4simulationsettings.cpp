#include "ag4simulationsettings.h"

#ifndef JSON11
#include "ajsontools.h"
#endif

#ifndef JSON11
void AG4SimulationSettings::writeToJson(QJsonObject &json) const
{
    json["PhysicsList"] = QString(PhysicsList.data());

    {
        QJsonArray ar;
        for (auto & v : SensitiveVolumes)
            ar.push_back(QString(v.data()));
        json["SensitiveVolumes"] = ar;
    }

    {
        QJsonArray ar;
        for (auto & v : ScintSensitiveVolumes)
            ar.push_back(QString(v.data()));
        json["ScintSensitiveVolumes"] = ar;
    }

    json["AddScintillatorsToSensitiveVolumes"] = AddScintillatorsToSensitiveVolumes;

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

    json["UseTSphys"]     = UseTSphys;
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

#ifdef JSON11
    json11::Json::array arSV;
#else
    QJsonArray arSV;
#endif
    jstools::parseJson(json, "SensitiveVolumes", arSV);
    for (int i = 0; i < (int)arSV.size(); i++)
    {
        std::string sv;
#ifdef JSON11
        sv = arSV[i].string_value();
#else
        sv = arSV.at(i).toString().toLatin1().data();
#endif
        SensitiveVolumes.push_back(sv);
    }

#ifdef JSON11
    json11::Json::array arSSV;
#else
    QJsonArray arSSV;
#endif
    jstools::parseJson(json, "ScintSensitiveVolumes", arSSV);
    for (int i = 0; i < (int)arSSV.size(); i++)
    {
        std::string sv;
#ifdef JSON11
        sv = arSSV[i].string_value();
#else
        sv = arSSV.at(i).toString().toLatin1().data();
#endif
        ScintSensitiveVolumes.push_back(sv);
    }

    jstools::parseJson(json, "AddScintillatorsToSensitiveVolumes", AddScintillatorsToSensitiveVolumes);

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
}

void AG4SimulationSettings::clear()
{
    PhysicsList = "QGSP_BERT_HP";
    UseTSphys = false;
    ScintSensitiveVolumes.clear();
    AddScintillatorsToSensitiveVolumes = true;
    SensitiveVolumes.clear();
    Commands = {"/run/setCut 0.7 mm"};
    StepLimits.clear();
}
