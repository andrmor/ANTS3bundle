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

void AParticleSimHub::writeToJson(QJsonObject & json, bool exportSimulation) const
{
    QJsonObject js;
    Settings.writeToJson(js, exportSimulation);
    json["ParticleSim"] = js;
}

#include "aparticleanalyzerhub.h"
void AParticleSimHub::readFromJson(const QJsonObject &json)
{
    AParticleAnalyzerHub::getInstance().clear();

    QJsonObject js;
    jstools::parseJson(json, "ParticleSim", js);
    Settings.readFromJson(js);
}

void AParticleSimHub::clear()
{
    Settings.clearSettings();
}
