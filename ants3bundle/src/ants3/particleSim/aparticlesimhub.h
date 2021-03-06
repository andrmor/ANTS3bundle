#ifndef APARTICLESIMHUB_H
#define APARTICLESIMHUB_H

#include "aparticlesimsettings.h"

#include <QObject>

class QJsonObject;

class AParticleSimHub : public QObject
{
    Q_OBJECT

public:
    static AParticleSimHub & getInstance();
    static const AParticleSimHub & getConstInstance();

private:
    AParticleSimHub(){}
    ~AParticleSimHub(){}

    AParticleSimHub(const AParticleSimHub&)            = delete;
    AParticleSimHub(AParticleSimHub&&)                 = delete;
    AParticleSimHub& operator=(const AParticleSimHub&) = delete;
    AParticleSimHub& operator=(AParticleSimHub&&)      = delete;

public:
    AParticleSimSettings  Settings;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // APARTICLESIMHUB_H
