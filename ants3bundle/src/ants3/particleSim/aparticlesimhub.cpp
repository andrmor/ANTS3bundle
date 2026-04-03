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

void AParticleSimHub::updateGeoConstRelatedSimProperties()
{
    Settings.SourceGenSettings.updateGeoConstRelatedSimProperties();
}

QString AParticleSimHub::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    return Settings.SourceGenSettings.isGeoConstInUse(nameRegExp);
}

void AParticleSimHub::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    Settings.SourceGenSettings.replaceGeoConstName(nameRegExp, newName);
}
