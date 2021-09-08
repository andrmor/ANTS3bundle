#ifndef AG4SIMULATIONSETTINGS_H
#define AG4SIMULATIONSETTINGS_H

#include <string>
#include <vector>
#include <map>

class QJsonObject;

class AG4SimulationSettings
{
public:
    std::string              PhysicsList = "QGSP_BERT_HP";
    bool                     UseTSphys = false;
    std::vector<std::string> SensitiveVolumes;
    std::vector<std::string> Commands = {"/run/setCut 0.7 mm"};

    std::map<std::string, double> StepLimits;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();
};

#endif // AG4SIMULATIONSETTINGS_H
