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

    size_t  getNumber() const {return EnergyHistStats[4];}

    QString readFromJson(const QJsonObject & json); // returns the name of the particle
    void releaseDynamicResources();
};

class AAnalyzerData
{
public:
    int     UniqueIndex;
    QString Name;

    std::map<QString, AAnalyzerParticle> ParticleMap;

    bool readFromJson(const QJsonObject & json);

    void clear();
};

class AParticleAnalyzerHub
{
public:
    static AParticleAnalyzerHub & getInstance();
    static const AParticleAnalyzerHub & getConstInstance();

    AAnalyzerData Data;

private:
    AParticleAnalyzerHub(){}
    ~AParticleAnalyzerHub();

    AParticleAnalyzerHub(const AParticleAnalyzerHub&)            = delete;
    AParticleAnalyzerHub(AParticleAnalyzerHub&&)                 = delete;
    AParticleAnalyzerHub& operator=(const AParticleAnalyzerHub&) = delete;
    AParticleAnalyzerHub& operator=(AParticleAnalyzerHub&&)      = delete;

public:
    //bool load(const QString & fileName);
    void mergeAnalyzerFiles(const std::vector<QString> & inFiles, const QString & outFile);

private:
    void clear();
};

#endif // APARTICLEANALYZERHUB_H
