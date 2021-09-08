#include "ag4simulationsettings.h"
#include "ajsontools.h"

void AG4SimulationSettings::writeToJson(QJsonObject &json) const
{
    json["PhysicsList"] = QString(PhysicsList.data());

    QJsonArray arSV;
    for (auto & v : SensitiveVolumes)
        arSV.push_back(QString(v.data()));
    json["SensitiveVolumes"] = arSV;

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

void AG4SimulationSettings::readFromJson(const QJsonObject &json)
{
    clear();

    jstools::parseJson(json, "PhysicsList", PhysicsList);

    QJsonArray arSV;
    SensitiveVolumes.clear();
    jstools::parseJson(json, "SensitiveVolumes", arSV);
    for (int i=0; i<arSV.size(); i++)
        SensitiveVolumes.push_back( arSV.at(i).toString().toLatin1().data() );

    QJsonArray arC;
    Commands.clear();
    jstools::parseJson(json, "Commands", arC);
    for (int i=0; i<arC.size(); i++)
        Commands.push_back( arC.at(i).toString().toLatin1().data() );

    QJsonArray arSL;
    StepLimits.clear();
    jstools::parseJson(json, "StepLimits", arSL);
    for (int i=0; i<arSL.size(); i++)
    {
        QJsonArray el = arSL[i].toArray();
        if (el.size() > 1)
        {
            QString vol = el[0].toString();
            double step = el[1].toDouble();
            StepLimits[vol.toLatin1().data()] = step;
        }
    }

    jstools::parseJson(json, "UseTSphys", UseTSphys);
}

void AG4SimulationSettings::clear()
{
    PhysicsList = "QGSP_BERT_HP";
    SensitiveVolumes.clear();
    Commands = {"/run/setCut 0.7 mm"};
    StepLimits.clear();
    UseTSphys = false;
}
