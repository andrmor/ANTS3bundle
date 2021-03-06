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
    // SensorHub   config is handled by ASensorHub   singleton

    // ParticleSim config is handled by AParticleSimHub singleton
    // PhotonSim   config is handled by APhotonSimHub   singleton

    // Reconstruction
    // AReconHub      -> TODO

    QJsonObject JSON;

    QString     ConfigName = "--";
    QString     ConfigDescription = "Description is empty";

    // Temporary:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    void    updateJSONfromConfig();
    QString updateConfigFromJSON();

    QString load(const QString & fileName);
    QString save(const QString & fileName);

    void    writeToJson(QJsonObject & json) const;  // !!!*** privat?
    QString readFromJson(const QJsonObject & json); // !!!*** privat?

private:
    QString tryReadFromJson(const QJsonObject & json); // !!!***

signals:
    void configLoaded();
    void requestSaveGuiSettings();

};

#endif // ACONFIG_H
