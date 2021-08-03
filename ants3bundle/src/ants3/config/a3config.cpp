#include "a3config.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "ainterfacerulehub.h"

#include <QDebug>

A3Config & A3Config::getInstance()
{
    static A3Config instance;
    return instance;
}

A3Config::A3Config()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

void A3Config::writeToJson(QJsonObject & json) const
{
    AMaterialHub::getInstance().writeToJson(json);
    writeGeometry  (json);
    writeInterRules(json);
    // sensors

    // Photon simulation
    {
        QJsonObject js;
        APhotonSimHub::getConstInstance().writeToJson(js);
        json["PhotonSim"] = js;
    }

    // Particle simulation

    // Reconstruction
}

QString A3Config::readFromJson(const QJsonObject & json)
{
    QString ErrorString;

    ErrorString = AMaterialHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;

    // Geometry
    {
        QJsonObject js;
        jstools::parseJson(json, "Geometry", js);
        AGeometryHub::getInstance().readFromJson(js);
        emit requestUpdateGeometryGui();
    }

    // Optical Interface Rules
    {
        QJsonArray ar;
        jstools::parseJson(json, "InterfaceRules", ar);
        QString err = AInterfaceRuleHub::getInstance().readFromJsonAr(ar);

        if (!err.isEmpty())
            ErrorList << "Interface rule errors:\n==>\n" + err + "<==\n";

        emit requestUpdateInterfaceRuleGui();
    }

    // Photon simulation
    {
        QJsonObject js;
        jstools::parseJson(json, "PhotonSim", js);
        APhotonSimHub::getInstance().readFromJson(js);
        emit requestUpdatePhotSimGui();
    }

    return "";
}

void A3Config::formConfigForPhotonSimulation(const QJsonObject & jsSim, QJsonObject & json)
{
    AMaterialHub::getInstance().writeToJson(json);
    writeGeometry  (json);
    writeInterRules(json);
    // sensors

    json["PhotonSim"] = jsSim;
}

void A3Config::writeGeometry(QJsonObject & json) const
{
    QJsonObject js;
    AGeometryHub::getInstance().writeToJson(js);
    json["Geometry"] = js;
}

void A3Config::writeInterRules(QJsonObject & json) const
{
    QJsonArray ar;
    AInterfaceRuleHub::getInstance().writeToJsonAr(ar);
    json["InterfaceRules"] = ar;
}

