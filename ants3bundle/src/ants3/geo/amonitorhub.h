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
    std::vector<AMonitorData> Monitors;

    void clear();
    void clearData();

    int  countMonitors() const {return Monitors.size();}
    int  countMonitorsWithHits() const;

    std::vector<const AMonitorData*> getMonitors(const AGeoObject *obj) const;  // returns nullptr if not found

    void mergePhotonMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile); // !!!*** cosnider config where there are phton and particle monitors simultaneously!
    void mergeParticleMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile); // !!!*** cosnider config where there are phton and particle monitors simultaneously!

    void    writeDataToJson(QJsonObject & json) const;
    QString appendDataFromJson(const QJsonObject & json);

};

#endif // AMONITORHUB_H
