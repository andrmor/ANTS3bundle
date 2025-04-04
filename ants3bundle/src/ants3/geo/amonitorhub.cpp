#include "amonitorhub.h"
#include "amonitor.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ajsontools.h"
#include "aerrorhub.h"

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

QString AMonitorHub::appendDataFromJson(const QJsonObject & json, EType type)
{
    std::vector<AMonitorData> & mons = (type == Photon ? PhotonMonitors : ParticleMonitors);

    QJsonArray ar;
    bool ok = jstools::parseJson(json, QString(type == Photon ? PhotonJsonName : ParticleJsonName), ar);
    if (!ok) return QString("json does not contain %0 monitor data").arg(type == Photon ? "photon" : "particle");

    if (ar.size() != (int)mons.size()) return "json contain data for wrong number of monitors";

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        AMonitor tmp;
        tmp.readDataFromJson(js);

        mons[i].Monitor->append(tmp);
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
        if (ok) appendDataFromJson(js, AMonitorHub::Photon);
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
            AErrorHub::addQError("Failed to read particle monitor file: " + FN);
            qWarning() << "Failed to read particle monitor file:" << FN;
            return;
        }

        for (int i=0; i<ar.size(); i++)
        {
            QJsonObject json = ar[i].toObject();
            int iMon;
            bool bOK = jstools::parseJson(json, "MonitorIndex", iMon);
            if (!bOK)
            {
                AErrorHub::addQError("Failed to read monitor data: MonitorIndex entry in json not found");
                qWarning() << "Failed to read monitor data: Monitor index not found";
                return;
            }
            if (iMon < 0 || iMon >= numMon)
            {
                AErrorHub::addQError( QString("Failed to read monitor data: invalid monitor index %0").arg(iMon) );
                qWarning() << "Failed to read monitor data: Invalid monitor index" << iMon;
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
