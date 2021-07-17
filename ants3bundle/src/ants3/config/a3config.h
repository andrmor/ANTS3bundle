#ifndef A3CONFIG_H
#define A3CONFIG_H

class QJsonObject;

#include <QString>

class A3Config final
{
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

    void writeDetectorConfig(QJsonObject & json) const;
    void readDetectorConfig(const QJsonObject & json);

    void writeAllConfig(QJsonObject & json) const;
    void readAllConfig(const QJsonObject & json);

};

#endif // A3CONFIG_H
