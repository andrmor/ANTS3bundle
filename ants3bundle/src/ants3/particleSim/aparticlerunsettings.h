#ifndef APARTICLERUNSETTINGS_H
#define APARTICLERUNSETTINGS_H

#include "amonitorsettings.h"
#include "acalsettings.h"

#include <string>
#include <vector>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
#endif

class ASaveParticlesSettings
{
public:
    bool        Enabled    = false;
    std::string FileName   = "OutputParticles.dat";
    std::string VolumeName;
    bool        StopTrack = true;

    bool        TimeWindow = false;
    double      TimeFrom   = 0;
    double      TimeTo     = 1e10;

    void clear();

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif
};

class AParticleRunSettings
{
public:
  // ants3 only
    std::string OutputDirectory;

  // ants3 and g4ants3
    int         Seed = 0;

    bool        AsciiOutput    = true;
    int         AsciiPrecision = 6;

    bool        SaveTrackingHistory = false;
    std::string FileNameTrackingHistory = "TrackingData.dat";

    bool        SaveDeposition = false;
    std::string FileNameDeposition = "Deposition.dat";

    ASaveParticlesSettings SaveSettings;

    AMonitorSettings MonitorSettings;  // also contains settings which are only initialized during g4anst3 export!

    ACalSettings CalorimeterSettings;  // also contains settings which are only initialized during g4anst3 export!

  // g4ants3 only
    int         EventFrom = 0;
    int         EventTo   = 0;

    std::vector<std::string> Materials;
    std::vector<std::pair<std::string, std::string>> MaterialsFromNist;
    std::vector<std::pair<std::string, double>> MaterialsMeanExEnergy; // in eV

    std::string GDML;
    std::string Receipt;

    bool GuiMode = false;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool includeG4ants3Set) const;
    void readFromJson(const QJsonObject & json);
#endif

    void clear();
};

#endif // APARTICLERUNSETTINGS_H
