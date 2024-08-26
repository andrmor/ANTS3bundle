#include "aparticleanalyzerhub.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>

size_t AAnalyzerParticle::getNumber() const
{
    return EnergyHistStats[4];
}

double AAnalyzerParticle::getMean() const
{
    //qDebug() << EnergyHistStats[0] << EnergyHistStats[1] << EnergyHistStats[2] << EnergyHistStats[3];
    if (EnergyHistStats[0] == 0) return 0;
    return EnergyHistStats[2] / EnergyHistStats[0];
}

QString AAnalyzerParticle::readFromJson(const QJsonObject & json)
{
    QString name;
    jstools::parseJson(json, "Particle", name);

    jstools::parseJson(json, "EnergyFrom", EnergyFrom);
    jstools::parseJson(json, "EnergyTo",   EnergyTo);

    QJsonArray energyData;
    jstools::parseJson(json, "EnergyData", energyData);
    const size_t num = energyData.size();
    EnergyBins = num - 2;  // first/last are under/overflows

    EnergyHist = new ATH1D("", "", EnergyBins, EnergyFrom, EnergyTo);
    for (size_t i = 0; i < num; i++)
    {
        const QJsonArray el = energyData[i].toArray();
        if (el.size() != 2)
        {
            QString err = "Bad length of Data array element for DepoOverEvent in calorimeter json";
            AErrorHub::addQError(err);
            qWarning() << err;
            return "";
        }
        EnergyHist->SetBinContent(i, el[1].toDouble());
    }
    EnergyHist->BufferEmpty();

    QJsonArray energyStats;
    bool ok = jstools::parseJson(json, "EnergyStats", energyStats);
    if (!ok)
    {
        QString err = "Cannot find EnergyStats array for Energy in particle analyzer json for " + name;
        AErrorHub::addQError(err);
        qWarning() << err;
        return "";
    }
    if (energyStats.size() != 5)
    {
        QString err = "Bad length of EnergyStats array for Energy in particle analyzer json for " + name;
        AErrorHub::addQError(err);
        qWarning() << err;
        return "";
    }
    for (size_t i = 0; i < 5; i++)
        EnergyHistStats[i] = energyStats[i].toDouble();

    EnergyHist->setStats(EnergyHistStats);

    return name;
}

void AAnalyzerParticle::writeToJson(QJsonObject & json) const
{
    json["EnergyFrom"] = EnergyFrom;
    json["EnergyTo"]   = EnergyTo;

    QJsonArray ar;
    for (size_t i = 0; i < EnergyBins+2; i++)
    {
        QJsonArray el;
        el.append(EnergyHist->GetBinCenter(i));
        el.append(EnergyHist->GetBinContent(i));
        ar.append(el);
    }
    json["EnergyData"] = ar;

    QJsonArray statsAr;
    for (size_t i = 0; i < 5; i++)
        statsAr.push_back(EnergyHistStats[i]);

    json["EnergyStats"] = statsAr;
}

void AAnalyzerParticle::mergeFrom(const AAnalyzerParticle & other)
{
    EnergyHist->mergeFrom(other.EnergyHist);

    for (size_t i = 0; i < 5; i++)
        EnergyHistStats[i] += other.EnergyHistStats[i];
}

void AAnalyzerParticle::releaseDynamicResources()
{
    delete EnergyHist; EnergyHist = nullptr;
}

// --------------

bool AAnalyzerData::readFromJson(const QJsonObject & json)
{
    clear();

    //jstools::parseJson(json, "UniqueIndex",    UniqueIndex);
    jstools::parseJson(json, "VolumeBaseName", Name);

    QJsonArray ar;
    jstools::parseJson(json, "ParticleData", ar);
    for (size_t i = 0; i < ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();

        AAnalyzerParticle rec;
        QString particleName = rec.readFromJson(js);
        if (particleName.isEmpty()) return false;

        ParticleMap[particleName] = rec;
    }

    jstools::parseJson(json, "GlobalIndexIfNoMerge", GlobalIndexIfNoMerge);

    return true;
}

void AAnalyzerData::writeToJson(QJsonObject & json) const
{
    //json["UniqueIndex"]          = UniqueIndex;
    json["VolumeBaseName"]       = Name;
    json["GlobalIndexIfNoMerge"] = GlobalIndexIfNoMerge;

    QJsonArray ar;
    for (const auto & pair : ParticleMap)
    {
        QJsonObject js;
            js["Particle"] = pair.first;
            pair.second.writeToJson(js);
        ar.push_back(js);
    }
    json["ParticleData"] = ar;
}

bool AAnalyzerData::mergeFrom(const AAnalyzerData & other)
{
    for (const auto & pairInOther : other.ParticleMap)
    {
        const QString           & particleNameInOther = pairInOther.first;
        const AAnalyzerParticle & particleDataInOther = pairInOther.second;

        auto it = ParticleMap.find(particleNameInOther);
        if (it != ParticleMap.end())
            it->second.mergeFrom(particleDataInOther);
        else
            ParticleMap[particleNameInOther] = particleDataInOther;
    }
    return true;
}

void AAnalyzerData::clear()
{
    for (auto & pair : ParticleMap)
        pair.second.releaseDynamicResources();
    ParticleMap.clear();
}

// --------------

AParticleAnalyzerHub & AParticleAnalyzerHub::getInstance()
{
    static AParticleAnalyzerHub instance;
    return instance;
}

const AParticleAnalyzerHub &AParticleAnalyzerHub::getConstInstance()
{
    return getInstance();
}

AParticleAnalyzerHub::~AParticleAnalyzerHub()
{
    UniqueAnalyzers.clear();
}

void AParticleAnalyzerHub::clear()
{
    UniqueAnalyzers.clear();
}

void AParticleAnalyzerHub::loadAnalyzerFiles(const std::vector<QString> & inFiles)
{
    clear();

    size_t numAnalyzersInFirstFile = 0;
    for (size_t iFile = 0; iFile < inFiles.size(); iFile++)
    {
        const QString fileName = inFiles[iFile];
        QJsonArray arAllAnalyzers;
        bool ok = jstools::loadJsonArrayFromFile(arAllAnalyzers, fileName);
        if (!ok)
        {
            QString err = "Cannot load particle analyzer file " + fileName;
            AErrorHub::addQError(err);
            qWarning() << err;
            return;
        }

        if (iFile == 0)
        {
            numAnalyzersInFirstFile = arAllAnalyzers.size();
            UniqueAnalyzers.resize(numAnalyzersInFirstFile);
            for (size_t i = 0; i < numAnalyzersInFirstFile; i++)
            {
                ok = UniqueAnalyzers[i].readFromJson(arAllAnalyzers[i].toObject());
                if (!ok) return;
            }
        }
        else
        {
            if (arAllAnalyzers.size() != numAnalyzersInFirstFile)
            {
                QString err = "Mismatch in the number of particle analyzers in different files";
                AErrorHub::addQError(err);
                qWarning() << err;
                return;
            }

            for (size_t i = 0; i < numAnalyzersInFirstFile; i++)
            {
                AAnalyzerData thisAnalyzer;
                ok = thisAnalyzer.readFromJson(arAllAnalyzers[i].toObject());
                if (!ok) return;

                UniqueAnalyzers[i].mergeFrom(thisAnalyzer);
            }
        }
    }
}

bool AParticleAnalyzerHub::saveAnalyzerData(const QString & fileName)
{
    QJsonArray ar;
    for (const AAnalyzerData & analyzer : UniqueAnalyzers)
    {
        QJsonObject json;
        analyzer.writeToJson(json);
        ar.push_back(json);
    }

    return jstools::saveJsonArrayToFile(ar, fileName);
}


