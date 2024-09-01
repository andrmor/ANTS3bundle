#ifndef APARTICLEANALYZERHUB_H
#define APARTICLEANALYZERHUB_H

#include "ath.h"
#include "qjsonobject.h"

#include <array>
#include <vector>
#include <map>

#include <QString>

class AAnalyzerParticle
{
public:
    ATH1D                * EnergyHist = nullptr;
    std::array<double,5>   EnergyHistStats;         // last is the number of entries
    size_t                 EnergyBins = 1;
    double                 EnergyFrom = 0;
    double                 EnergyTo = 0;

    size_t  getNumber() const;
    double  getMean() const;

    QString readFromJson(const QJsonObject & json); // returns the name of the particle
    void    writeToJson(QJsonObject & json) const;
    void    mergeFrom(const AAnalyzerParticle & other);
    void    releaseDynamicResources();
};

class AAnalyzerData
{
public:
    //int     UniqueIndex;
    QString Name;
    QString EnergyDataUnits;
    int     GlobalIndexIfNoMerge;

    std::map<QString, AAnalyzerParticle> ParticleMap;

    bool readFromJson(const QJsonObject & json);
    void writeToJson(QJsonObject & json) const;

    bool mergeFrom(const AAnalyzerData & other);

    void clear();
};

class AParticleAnalyzerHub
{
public:
    static AParticleAnalyzerHub & getInstance();
    static const AParticleAnalyzerHub & getConstInstance();

    std::vector<AAnalyzerData> UniqueAnalyzers;

private:
    AParticleAnalyzerHub(){}
    ~AParticleAnalyzerHub();

    AParticleAnalyzerHub(const AParticleAnalyzerHub&)            = delete;
    AParticleAnalyzerHub(AParticleAnalyzerHub&&)                 = delete;
    AParticleAnalyzerHub& operator=(const AParticleAnalyzerHub&) = delete;
    AParticleAnalyzerHub& operator=(AParticleAnalyzerHub&&)      = delete;

public:
    void loadAnalyzerFiles(const std::vector<QString> & inFiles);
    bool saveAnalyzerData(const QString & fileName);

private:
    void clear();
};

#endif // APARTICLEANALYZERHUB_H
