#ifndef AMONITORHUB_H
#define AMONITORHUB_H

#include "avector.h"

#include <QString>
#include <QJsonObject>

#include <vector>

class AMonitor;
class QString;
class AGeoObject;

struct AMonitorData
{
    QString      Name;
    AMonitor   * Monitor;
    AGeoObject * GeoObj   = nullptr;
    AVector3     Position = {0, 0, 0};
};

class AMonitorHub
{
public:
    enum EType {Photon, Particle};

    static AMonitorHub & getInstance();
    static const AMonitorHub & getConstInstance();

private:
    AMonitorHub(){}
    ~AMonitorHub();

    AMonitorHub(const AMonitorHub&)            = delete;
    AMonitorHub(AMonitorHub&&)                 = delete;
    AMonitorHub& operator=(const AMonitorHub&) = delete;
    AMonitorHub& operator=(AMonitorHub&&)      = delete;

public:
    std::vector<AMonitorData> PhotonMonitors;
    std::vector<AMonitorData> ParticleMonitors;

    void clear(EType type);
    void clearData(EType type);

    int  countMonitors(EType type) const;
    int  countMonitorsWithHits(EType type) const;

    std::vector<const AMonitorData*> getMonitors(const AGeoObject * obj) const;  // returns nullptr if not found

    void mergePhotonMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile); // !!!*** cosnider config where there are phton and particle monitors simultaneously!
    void mergeParticleMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile); // !!!*** cosnider config where there are phton and particle monitors simultaneously!

    void writeDataToJson(EType type, QJsonObject & json) const;

    QString appendPhotonDataFromJson(const QJsonObject & json);

    static constexpr auto PhotonJsonName   = "ParticleMonitorData";
    static constexpr auto ParticleJsonName = "PhotonMonitorData";

};

#endif // AMONITORHUB_H
