#include "a3config.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "ainterfacerulehub.h"
#include "asensorhub.h"
#include "aparticlesimhub.h"
#include "aerrorhub.h"

#include <QDebug>

A3Config & A3Config::getInstance()
{
    static A3Config instance;
    return instance;
}

const A3Config &A3Config::getConstInstance()
{
    return A3Config::getInstance();
}

A3Config::A3Config()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

QString A3Config::load(const QString & fileName)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok) return "Cannot open config file: " + fileName;

    return readFromJson(json);
}

void A3Config::writeToJson(QJsonObject & json) const
{
    json["ConfigName"]        = ConfigName;
    json["ConfigDescription"] = ConfigDescription;

    AMaterialHub::getInstance().writeToJson(json);
    AGeometryHub::getInstance().writeToJson(json);
    AInterfaceRuleHub::getInstance().writeToJson(json);
    ASensorHub::getConstInstance().writeToJson(json); // !!!***

    APhotonSimHub::getConstInstance().writeToJson(json);
    AParticleSimHub::getConstInstance().writeToJson(json);

    // Reconstruction
    // LRFs
}

QString A3Config::readFromJson(const QJsonObject & json)
{
    // !!!*** restore from JSON if error


    jstools::parseJson(json, "ConfigName",        ConfigName);
    jstools::parseJson(json, "ConfigDescription", ConfigDescription);

    QString ErrorString;

    ErrorString = AMaterialHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;

    ErrorString = AGeometryHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdateGeometryGui();        // TODO: to the hub? !!!***

    ErrorString = AInterfaceRuleHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdateInterfaceRuleGui();   // TODO: to the hub? !!!***

    // !!!*** SensorHub

    ErrorString = APhotonSimHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdatePhotSimGui();         // TODO: to the hub? !!!***

    AParticleSimHub::getInstance().readFromJson(json);
    // error handling!
    emit requestUpdateParticleSimGui();         // TODO: to the hub? !!!***

    // Reconstruction
    // LRFs

    JSON = json;
    return "";
}
