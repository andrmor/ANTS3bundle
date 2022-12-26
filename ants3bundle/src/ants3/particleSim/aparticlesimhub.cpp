#include "aparticlesimhub.h"
#include "ajsontools.h"

AParticleSimHub & AParticleSimHub::getInstance()
{
    static AParticleSimHub instance;
    return instance;
}

const AParticleSimHub &AParticleSimHub::getConstInstance()
{
    return getInstance();
}

void AParticleSimHub::writeToJson(QJsonObject &json) const
{
    QJsonObject js;
    Settings.writeToJson(js);
    json["ParticleSim"] = js;
}

void AParticleSimHub::readFromJson(const QJsonObject &json)
{
    QJsonObject js;
    jstools::parseJson(json, "ParticleSim", js);
    Settings.readFromJson(js);
}

void AParticleSimHub::clear()
{
    Settings.clearSettings();
}
