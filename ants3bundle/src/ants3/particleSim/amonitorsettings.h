#ifndef AMONITORSETTINGS_H
#define AMONITORSETTINGS_H

#include <string>
#include <vector>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
    class AMonitorConfig;
#endif

class AMonSetRecord
{
public:
    std::string Name;  // !!!*** need it? there is Config.Particle
    std::string Particle;
    int         Index;

#ifdef JSON11
    json11::Json::object ConfigJson;
#else
    const AMonitorConfig * Config = nullptr;
#endif

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif
};

class AMonitorSettings
{
public:
    bool Enabled         = false;
    std::string FileName = "ParticleMonitors.json";

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
    void initFromHub();
#endif

    std::vector<AMonSetRecord> Monitors;

    void clear();
};

#endif // AMONITORSETTINGS_H
