#ifndef AG4SIMULATIONSETTINGS_H
#define AG4SIMULATIONSETTINGS_H

#include <string>
#include <vector>
#include <map>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
#endif

class AG4SimulationSettings
{
public:
    std::string              PhysicsList = "QGSP_BERT_HP";
    bool                     UseTSphys = false;
    std::vector<std::string> SensitiveVolumes;
    std::vector<std::string> Commands = {"/run/setCut 0.7 mm"};

    std::map<std::string, double> StepLimits;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif

    void clear();
};

#endif // AG4SIMULATIONSETTINGS_H
