#ifndef APHOTONSIMHUB_H
#define APHOTONSIMHUB_H

#include "aphotonsimsettings.h"
#include "a3farmnoderecord.h"

#include <QObject>
#include <QString>

#include <vector>

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

    QString            ErrorString;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

public slots:
    bool simulate(int numLocalProc = -1);

protected:
    bool configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);

signals:
    void settingsChanged();
    void simFinished();
};

#endif // APHOTONSIMHUB_H
