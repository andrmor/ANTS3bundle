#include "amonitorsettings.h"

#ifndef JSON11
    #include "amonitorhub.h"
    #include "amonitor.h"
    #include "ajsontools.h"
#endif

#ifndef JSON11
void AMonSetRecord::writeToJson(QJsonObject &json) const
{
    json["Name"]     = Name.data();
    json["Particle"] = Particle.data();
    json["Index"]    = Index;

    QJsonObject js;
        Config->writeToJson(js);
    json["Config"] = js;
}
#endif

#ifdef JSON11
void AMonSetRecord::readFromJson(const json11::Json::object & json)
#else
void AMonSetRecord::readFromJson(const QJsonObject &json)
#endif
{
    jstools::parseJson(json, "Name",     Name);
    jstools::parseJson(json, "Particle", Particle);
    jstools::parseJson(json, "Index",    Index);

#ifdef JSON11
    jstools::parseJson(json, "Config", ConfigJson);
#endif
    //no need to read config on ANTS3 side
}

// ----------

#ifndef JSON11
void AMonitorSettings::initFromHub()
{
    Monitors.clear();
    const std::vector<AMonitorData> & MonitorsRecords = AMonitorHub::getConstInstance().ParticleMonitors;
    for (int iMon = 0; iMon < (int)MonitorsRecords.size(); iMon++)
    {
        const AMonitorData & mon = MonitorsRecords[iMon];
        const AMonitorConfig & mc = mon.Monitor->config;

        AMonSetRecord r;
        r.Name     = mon.Name.toLatin1().data();
        r.Particle = mc.Particle.toLatin1().data();
        r.Index    = iMon;
        r.Config   = &mon.Monitor->config;
        Monitors.push_back(r);
    }
}

void AMonitorSettings::writeToJson(QJsonObject & json, bool includeG4ants3Set) const
{
    json["Enabled"] = Enabled;
    json["FileName"] = FileName.data();

    if (includeG4ants3Set)
    {
        QJsonArray arMon;
        for (const AMonSetRecord & m : Monitors)
        {
            QJsonObject mjs;
            m.writeToJson(mjs);
            arMon.append(mjs);
        }
        json["Monitors"] = arMon;
    }
}
#endif

void AMonitorSettings::clear()
{
    Enabled         = false;
    FileName = "ParticleMonitors.json";

    Monitors.clear();
}

#ifdef JSON11
void AMonitorSettings::readFromJson(const json11::Json::object & json)
#else
void AMonitorSettings::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "Enabled",  Enabled);
    jstools::parseJson(json, "FileName", FileName);

    Monitors.clear();
#ifdef JSON11
    json11::Json::array MonitorArray;
    jstools::parseJson(json, "Monitors", MonitorArray);
    for (size_t i = 0; i < MonitorArray.size(); i++)
    {
        json11::Json::object mjs = MonitorArray[i].object_items();

        AMonSetRecord r;
            r.readFromJson(mjs);
        Monitors.push_back(r);
    }
#endif
    // no need to read configs on ANTS3 side
}
