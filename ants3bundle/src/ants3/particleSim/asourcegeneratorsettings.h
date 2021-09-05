#ifndef ASOURCEGENERATORSETTINGS_H
#define ASOURCEGENERATORSETTINGS_H

#include "aparticlesourcerecord.h"

class ASourceGeneratorSettings
{
public:
    enum EMultiMode {Constant = 0, Poisson = 1};

    std::vector<AParticleSourceRecord> SourceData;

    bool       MultiEnabled = false;
    EMultiMode MultiMode    = Constant;
    double     MultiNumber  = 1.0;

    void        clear();

    int         getNumSources() const {return SourceData.size();}
    double      calculateTotalActivity() const;

    std::string check() const;

    bool        clone(int iSource);
    bool        replace(int iSource, AParticleSourceRecord & source);
    void        remove(int iSource);

    void        writeToJson(QJsonObject & json) const;
    bool        readFromJson(const QJsonObject & json); // Error handling !!!***

};

#endif // ASOURCEGENERATORSETTINGS_H
