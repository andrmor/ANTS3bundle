#ifndef ACONFIG_H
#define ACONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>

class QJsonObject;

class AConfig final : public QObject
{
    Q_OBJECT

public:
    static AConfig & getInstance();
    static const AConfig & getConstInstance();

private:
    AConfig();
    ~AConfig(){}

    AConfig(const AConfig&)            = delete;
    AConfig(AConfig&&)                 = delete;
    AConfig& operator=(const AConfig&) = delete;
    AConfig& operator=(AConfig&&)      = delete;

public:
    // Geometry config is handled by AGeometryHub    singleton
    // Material config is handled by AMaterialHub    singleton
    // Optical interface rules    -> AOpticalRileHub singleton
    // SensorHub      -> TODO

    // ParticleSim    -> TODO
    // PhotonSim config is handled by APhotonSimHub singleton

    // Reconstruction
    // AReconHub      -> TODO

    QJsonObject JSON;

    QString     ConfigName = "--";
    QString     ConfigDescription = "Description not provided";

    // Temporary:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    QString load(const QString & fileName);

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);  // !!!***

signals:
    // !!!*** remove? The caller of loadConfig() will receive the result, if OK, then request update gui
    void requestUpdateGeometryGui();
    void requestUpdateInterfaceRuleGui();
    void requestUpdatePhotSimGui();
    void requestUpdateParticleSimGui();

};

#endif // ACONFIG_H
