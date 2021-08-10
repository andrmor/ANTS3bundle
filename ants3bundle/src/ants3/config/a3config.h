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
    // Geometry config is handled by AGeometryHub    singleton
    // Material config is handled by AMaterialHub    singleton
    // Optical interface rules    -> AOpticalRileHub singleton
    // SensorHub      -> TODO

    // ParticleSim    -> TODO
    // PhotonSim config is handled by APhotonSimHub singleton

    // Reconstruction
    // AReconHub      -> TODO

    QStringList ErrorList; // TODO: decide on the implementation!

    // Temporary:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

signals:
    void requestUpdateGeometryGui();
    void requestUpdatePhotSimGui();
    void requestUpdateInterfaceRuleGui();

};

#endif // A3CONFIG_H
