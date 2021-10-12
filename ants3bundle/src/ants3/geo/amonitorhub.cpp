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
    clear(Photon);
    clear(Particle);
}

void AMonitorHub::writeDataToJson(EType type, QJsonObject & json) const
{
    const std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);

    QJsonArray ar;
    for (const AMonitorData & md : mons)
    {
        QJsonObject js;
        md.Monitor->writeDataToJson(js);
        ar.push_back(js);
    }
    json[QString(type == Photon ? PhotonJsonName : ParticleJsonName)] = ar;
}

QString AMonitorHub::appendPhotonDataFromJson(const QJsonObject &json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "MonitorData", ar);
    if (!ok) return "json does not contain monitor data";

    if (ar.size() != (int)PhotonMonitors.size()) return "json contain data for wrong number of monitors";

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        AMonitor tmp;
        tmp.readDataFromJson(js);

        PhotonMonitors[i].Monitor->append(tmp);
    }

    return "";
}

void AMonitorHub::clear(EType type)
{
    std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);

    for (AMonitorData & md : mons) delete md.Monitor;
    mons.clear();
}

void AMonitorHub::clearData(EType type)
{
    std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);

    for (AMonitorData & md : mons) md.Monitor->clearData();
}

int AMonitorHub::countMonitors(EType type) const
{
    const std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);
    return mons.size();
}

int AMonitorHub::countMonitorsWithHits(EType type) const
{
    const std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);

    int counter = 0;
    for (const AMonitorData & md : mons)
        if (md.Monitor->getHits() > 0) counter++;
    return counter;
}

std::vector<const AMonitorData *> AMonitorHub::getMonitors(const AGeoObject * obj) const
{
    std::vector<const AMonitorData *> vec;
    for (const AMonitorData & md : PhotonMonitors)
        if (md.GeoObj == obj) vec.push_back(&md);
    for (const AMonitorData & md : ParticleMonitors)
        if (md.GeoObj == obj) vec.push_back(&md);
    return vec;
}

void AMonitorHub::mergePhotonMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile)
{
    clearData(Photon);

    for (const QString & FN : inFiles)
    {
        QJsonObject js;
        bool ok = jstools::loadJsonFromFile(js, FN);
        if (ok) appendPhotonDataFromJson(js);
    }

    QJsonObject json;
        writeDataToJson(Photon, json);
    jstools::saveJsonToFile(json, outFile);
}

void AMonitorHub::mergeParticleMonitorFiles(const std::vector<QString> & inFiles, const QString & outFile)
{
    clearData(Particle);

    const int numMon = ParticleMonitors.size();
    for (const QString & FN : inFiles)
    {
        QJsonArray ar;
        bool ok = jstools::loadJsonArrayFromFile(ar, FN);
        if (!ok)
        {
            // !!!*** errorhub!
            qWarning() << "failed to read particle monitor file:" << FN;
            return;
        }

        for (int i=0; i<ar.size(); i++)
        {
            QJsonObject json = ar[i].toObject();
            int iMon;
            bool bOK = jstools::parseJson(json, "MonitorIndex", iMon);
            if (!bOK)
            {
                // !!!*** errorhub!
                qWarning() << "Failed to read monitor data: Monitor index not found";
                return;
            }
            if (iMon < 0 || iMon >= numMon)
            {
                // !!!*** errorhub!
                qWarning() << "Failed to read monitor data: Bad monitor index";
                return;
            }

            AMonitor tmp;
            tmp.overrideDataFromJson(json);
            ParticleMonitors[iMon].Monitor->append(tmp);
        }
    }

    QJsonObject json;
        writeDataToJson(Particle, json);
    jstools::saveJsonToFile(json, outFile);
}

