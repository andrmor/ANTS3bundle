#ifndef A3CONFIG_H
#define A3CONFIG_H

#include <QObject>
#include <QString>

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
    // Detector
    // ->Geometry config is handled by A3Geometry singleton
    // ->Material config is handled by A3MatHub   singleton

    // Simulation
    // ->Particle sim
    // ->Photon   sim

    // Reconstruction
    // ...

    // Temporary:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

signals:
    void requestUpdateMaterialGui();
    void requestUpdateGeometryGui();
    void requestUpdatePhotSimGui();

};

#endif // A3CONFIG_H
