#ifndef A3CONFIG_H
#define A3CONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>

class QJsonObject;

class A3Config final : public QObject
{
    Q_OBJECT

public:
    static A3Config & getInstance();

private:
    A3Config();
    ~A3Config(){}

    A3Config(const A3Config&)            = delete;
    A3Config(A3Config&&)                 = delete;
    A3Config& operator=(const A3Config&) = delete;
    A3Config& operator=(A3Config&&)      = delete;

public:
    // Geometry  config is handled by AGeometryHub  singleton
    // Material  config is handled by AMaterialHub  singleton
    // InterfaceRules -> TODO
    // SensorHub      -> TODO

    // ParticleSim    -> TODO
    // PhotonSim config is handled by APhotonSimHub singleton

    // Reconstruction
    // AReconHub      -> TODO

    QStringList ErrorList;

    // Temporary:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void formConfigForPhotonSimulation(const QJsonObject & jsSim, QJsonObject & json);

private:
    void writeMaterials(QJsonObject & json) const;
    void writeGeometry (QJsonObject & json) const;

signals:
    void requestUpdateGeometryGui();
    void requestUpdatePhotSimGui();

};

#endif // A3CONFIG_H
