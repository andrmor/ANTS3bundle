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
    std::string              PhysicsList = "QGSP_BIC_HP";
    bool                     UseTSphys = false;
    bool                     UseNCrystal = false;

    std::vector<std::string> Commands = {"/run/setCut 0.7 mm"};
    std::map<std::string, double> StepLimits;

    bool   SimulateAnnihilAcolinearity = false;
    int    AcolinearityModel = 0; // 0 - magnitude (e.g. Gate), 1 - 2D - see https://doi.org/10.1088/1361-6560/ad70f1
    double AcolinearityFWHM  = 0.5; // in deg
    std::vector<std::string> AcolinearityVolumes;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif

    void clear();
};

#endif // AG4SIMULATIONSETTINGS_H
