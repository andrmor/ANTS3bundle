#include "aparticleanalyzerhub.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>

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
        EnergyHist->SetBinContent(i, el[2].toDouble());
    }

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

void AAnalyzerParticle::releaseDynamicResources()
{
    delete EnergyHist; EnergyHist = nullptr;
}

// --------------

bool AAnalyzerData::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "UniqueIndex",    UniqueIndex);
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
    Data.clear();
}

void AParticleAnalyzerHub::clear()
{
    Data.clear();
}

void AParticleAnalyzerHub::mergeAnalyzerFiles(const std::vector<QString> & inFiles, const QString & outFile)
{
    clear();

    if (inFiles.empty()) return;

    QJsonArray ar;
    QString fileName = inFiles.front();
    bool ok = jstools::loadJsonArrayFromFile(ar, fileName);
    if (!ok)
    {
        QString err = "Cannot load particle analyzer file " + fileName;
        AErrorHub::addQError(err);
        qWarning() << err;
        return;
    }

    Data.readFromJson(ar[0].toObject());

    /*
    for (const QString & FN : inFiles)
    {
        QJsonArray ar;
        bool ok = jstools::loadJsonArrayFromFile(ar, FN);
        if (!ok)
        {
            QString err = "Failed to read particle analyzer file: " + FN;
            AErrorHub::addQError(err);
            qWarning() << err;
            return;
        }

        for (int i = 0; i < ar.size(); i++)
        {
            QJsonObject json = ar[i].toObject();
            int iCalo;
            bool bOK = jstools::parseJson(json, "CalorimeterIndex", iCalo);
            if (!bOK)
            {
                QString err = "Failed to read calorimeter data: invalid format of Calorimeter index in file\n" + FN;
                AErrorHub::addQError(err);
                qWarning() << err;
                continue;
            }
            if (iCalo < 0 || iCalo >= (int)numCalorimeters)
            {
                QString err = "Failed to read calorimter data: Bad calorimeter index in file:\n" + FN;
                AErrorHub::addQError(err);
                qWarning() << err;
                return false;
            }

            Calorimeters[iCalo].Calorimeter->appendDataFromJson(json);
        }
    }
    */

    //QJsonArray jsCAr;
    //writeDataToJson(jsCAr);
    //return jstools::saveJsonArrayToFile(jsCAr, outFile);
}


