#include "aconfig.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "ainterfacerulehub.h"
#include "asensorhub.h"
#include "aparticlesimhub.h"
#include "aerrorhub.h"

#include <QDebug>

AConfig & AConfig::getInstance()
{
    static AConfig instance;
    return instance;
}

const AConfig &AConfig::getConstInstance()
{
    return AConfig::getInstance();
}

AConfig::AConfig()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

void AConfig::updateJSONfromConfig()
{
    writeToJson(JSON);
}

QString AConfig::updateConfigFromJSON()
{
    return readFromJson(JSON);  // !!!*** add error control (paranoic)
}

QString AConfig::load(const QString & fileName)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok) return "Cannot open config file: " + fileName;

    return readFromJson(json);
}

QString AConfig::save(const QString & fileName)
{
    updateJSONfromConfig();

    bool ok = jstools::saveJsonToFile(JSON, fileName);
    if (ok) return "";
    else    return "Cannot open file to save config:\n" + fileName;
}

void AConfig::writeToJson(QJsonObject & json) const
{
    json["ConfigName"]        = ConfigName;
    json["ConfigDescription"] = ConfigDescription;

    AMaterialHub::getInstance().writeToJson(json);
    AGeometryHub::getInstance().writeToJson(json);
    AInterfaceRuleHub::getInstance().writeToJson(json);
    ASensorHub::getConstInstance().writeToJson(json);

    APhotonSimHub::getConstInstance().writeToJson(json);
    AParticleSimHub::getConstInstance().writeToJson(json);

    // Reconstruction
    // LRFs
}

QString AConfig::readFromJson(const QJsonObject & json)
{
    // !!!*** restore from JSON if error
    QString err = tryReadFromJson(json);
    if (err.isEmpty())
    {
        JSON = json;
        emit configLoaded();
        return "";
    }
    else
    {
        readFromJson(JSON);
        return err;
    }
}

QString AConfig::tryReadFromJson(const QJsonObject & json)
{
    bool ok = jstools::parseJson(json, "ConfigName",        ConfigName);
    if (!ok) return "Not a configuration file!";
    ok      = jstools::parseJson(json, "ConfigDescription", ConfigDescription);
    if (!ok) return "Not a configuration file!";

    QString Error;

    Error = AMaterialHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;

    Error = AGeometryHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;

    Error = AInterfaceRuleHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;

    Error = ASensorHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;

    Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;

    AParticleSimHub::getInstance().readFromJson(json);
    // error handling! !!!***

    // Reconstruction
    // LRFs

    return "";
}
