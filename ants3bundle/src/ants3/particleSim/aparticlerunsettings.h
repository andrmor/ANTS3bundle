#ifndef APARTICLERUNSETTINGS_H
#define APARTICLERUNSETTINGS_H

#include <string>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
#endif

class AParticleRunSettings
{
public:
    int     Seed = 0;

    int     EventFrom = 0;
    int     EventTo   = 0;

    bool    AsciiOutput    = true;
    int     AsciiPrecision = 6;

    std::string OutputDirectory;

    bool    SaveTrackingData = true;
    std::string FileNameTrackingData = "TrackingData.txt";

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif

    void clear();
};

#endif // APARTICLERUNSETTINGS_H
