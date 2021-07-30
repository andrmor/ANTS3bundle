#include "a3config.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"

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
    //Materials
    {
        QJsonArray ar;
        AMaterialHub::getInstance().writeToJsonAr(ar);
        json["Materials"] = ar;
    }

    //Geometry
    {
        QJsonObject js;
        AGeometryHub::getInstance().writeToJson(js);
        json["Geometry"] = js;
    }

    // Photon simulation
    {
        QJsonObject js;
        APhotonSimHub::getConstInstance().writeToJson(js);
        json["PhotonSim"] = js;
    }

    // Particle simulation

    // Reconstruction
}

void A3Config::readFromJson(const QJsonObject & json)
{
    // Materials
    {
        QJsonArray ar;
        jstools::parseJson(json, "Materials", ar);
        AMaterialHub::getInstance().readFromJsonAr(ar);
    }

    // Geometry
    {
        QJsonObject js;
        jstools::parseJson(json, "Geometry", js);
        AGeometryHub::getInstance().readFromJson(js);
        emit requestUpdateGeometryGui();
    }

    // Photon simulation
    {
        QJsonObject js;
        jstools::parseJson(json, "PhotonSim", js);
        APhotonSimHub::getInstance().readFromJson(js);
        emit requestUpdatePhotSimGui();
    }
}

