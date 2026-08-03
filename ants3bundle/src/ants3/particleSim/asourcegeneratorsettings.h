#ifndef ASOURCEGENERATORSETTINGS_H
#define ASOURCEGENERATORSETTINGS_H

#include "aparticlesourcerecord.h"

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
#endif

class ASourceGeneratorSettings
{
public:
    enum EMultiMode {Constant = 0, Poisson = 1};

    std::vector<AParticleSourceRecordBase*> SourceData;

    bool       MultiEnabled = false;
    EMultiMode MultiMode    = Constant;
    double     MultiNumber  = 1.0;

    void        clear();

    int         getNumSources() const {return SourceData.size();}
    double      calculateTotalActivity() const;

    bool        check() const;

    bool        clone(int iSource);
    bool        replace(int iSource, AParticleSourceRecordBase * source);
    void        remove(int iSource);

#ifdef JSON11
    bool        readFromJson(const json11::Json::object & json); // Error handling !!!***
#else
    void        writeToJson(QJsonObject & json) const;
    bool        readFromJson(const QJsonObject & json); // Error handling !!!***

    void        updateGeoConstRelatedSimProperties();
    QString     isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void        replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);
#endif

    // only GUI - used to draw source during editing its settings
    int IndexSourceEdit = -1;
    AParticleSourceRecordBase * SourceEdit = nullptr;

};

#endif // ASOURCEGENERATORSETTINGS_H
