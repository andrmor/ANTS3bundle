#include "a3config.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "ainterfacerulehub.h"
#include "asensorhub.h"

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
    AGeometryHub::getInstance().writeToJson(json);
    AInterfaceRuleHub::getInstance().writeToJson(json);
    ASensorHub::getConstInstance().writeToJson(json); // !!!***

    APhotonSimHub::getConstInstance().writeToJson(json);

    // Particle simulation

    // Reconstruction
}

QString A3Config::readFromJson(const QJsonObject & json)
{
    QString ErrorString;

    ErrorString = AMaterialHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;

    ErrorString = AGeometryHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdateGeometryGui();        // TODO: to the hub? !!!***

    ErrorString = AInterfaceRuleHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdateInterfaceRuleGui();   // TODO: to the hub? !!!***

    ErrorString = APhotonSimHub::getInstance().readFromJson(json);
    if (!ErrorString.isEmpty()) return ErrorString;
    emit requestUpdatePhotSimGui();         // TODO: to the hub? !!!***

    // !!!*** SensorHub

    return "";
}
