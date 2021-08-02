#ifndef APHOTONSIMHUB_H
#define APHOTONSIMHUB_H

#include "aphotonsimsettings.h"

#include <QObject>

class A3WorkDistrConfig;

class APhotonSimHub final : public QObject
{
    Q_OBJECT

public:
    static       APhotonSimHub & getInstance();
    static const APhotonSimHub & getConstInstance();

private:
    APhotonSimHub(){}
    ~APhotonSimHub(){}

    APhotonSimHub(const APhotonSimHub&)            = delete;
    APhotonSimHub(APhotonSimHub&&)                 = delete;
    APhotonSimHub& operator=(const APhotonSimHub&) = delete;
    APhotonSimHub& operator=(APhotonSimHub&&)      = delete;

public:
    APhotonSimSettings Settings;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

signals:
    void settingsChanged();
};

#endif // APHOTONSIMHUB_H
