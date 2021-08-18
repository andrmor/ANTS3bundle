#include "amonitorhub.h"
#include "amonitor.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ajsontools.h"

AMonitorHub & AMonitorHub::getInstance()
{
    static AMonitorHub instance;
    return instance;
}

const AMonitorHub & AMonitorHub::getConstInstance()
{
    return getInstance();
}

AMonitorHub::~AMonitorHub()
{
    clear();
}

void AMonitorHub::writeDataToJson(QJsonObject & json) const
{
    QJsonArray ar;

    for (const AMonitorData & md : Monitors)
    {
        QJsonObject js;
        md.Monitor->writeDataToJson(js);
        ar.push_back(js);
    }

    json["MonitorData"] = ar;
}

QString AMonitorHub::appendDataFromJson(const QJsonObject &json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "MonitorData", ar);
    if (!ok) return "json does not contain monitor data";

    if (ar.size() != (int)Monitors.size()) return "json contain data for wrong number of monitors";

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        AMonitor tmp;
        tmp.readDataFromJson(js);

        Monitors[i].Monitor->append(tmp);
    }

    return "";
}

void AMonitorHub::clear()
{
    for (AMonitorData & md : Monitors) delete md.Monitor;
    Monitors.clear();
}

void AMonitorHub::clearData()
{
    for (AMonitorData & md : Monitors) md.Monitor->clearData();
}

