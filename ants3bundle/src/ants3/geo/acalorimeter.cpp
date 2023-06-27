#include "acalorimeter.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ajsontools.h"
#include "ajsontoolsroot.h"
#include "aroothistappenders.h"
#include "aerrorhub.h"
#include "ath.h"

#include <QDebug>

#include "TString.h"

ACalorimeter::ACalorimeter(const AGeoObject * CalorimeterGeoObject)
{
    readFromGeoObject(CalorimeterGeoObject);
}

ACalorimeter::~ACalorimeter()
{
    clearData();
}

void ACalorimeter::clearData()
{
    delete DataHistogram; DataHistogram = nullptr;
    Stats.fill(0);
    Entries = 0;

    delete EventDepoData; EventDepoData = nullptr;
    EventDepoDataStats.fill(0);
}

double ACalorimeter::getTotalEnergy() const
{
    if (!DataHistogram) return 0;
    return Stats[0];
}

bool ACalorimeter::readFromGeoObject(const AGeoObject * geoObj)
{
    const ACalorimeterProperties * props = geoObj->getCalorimeterProperties();
    if (!props)
    {
        AErrorHub::addQError("Provided geo object is not a calorimeter!");
        return false;
    }

    Properties = *props;

    //Name = geoObj->Name;

    return true;
}

void ACalorimeter::writeDataToJson(QJsonObject & json, int index) const
{
    json["CalorimeterIndex"] = index;

    QJsonObject jsProps;
    Properties.writeToJson(jsProps);
    json["Properties"] = jsProps;

    QJsonObject jsDepo;
    {
        QJsonArray ar;
        for (int iz = 0; iz < Properties.Bins[2]; iz++)
            for (int iy = 0; iy < Properties.Bins[1]; iy++)
            {
                QJsonArray el;
                for (int ix = 0; ix < Properties.Bins[0]; ix++)
                    el.push_back (DataHistogram->GetBinContent(ix+1, iy+1, iz+1) );
                ar.push_back(el);
            }
        jsDepo["Data"] = ar;

        QJsonArray sjs;
        for (const double & d : Stats)
            sjs.push_back(d);
        jsDepo["Stat"] = sjs;

        jsDepo["Entries"] = Entries;
    }
    json["XYZDepo"] = jsDepo;

    if (EventDepoData)
    {
        QJsonObject js;

        QJsonArray ar;
        const int size = EventDepoData->GetNbinsX();
        for (int i = 0; i < size+2; i++)  // starts with underflow and ends with overflow
        {
            QJsonArray el;
            el.push_back(EventDepoData->GetBinCenter(i));
            el.push_back(EventDepoData->GetBinContent(i));
            ar.push_back(el);
        }
        js["Data"] = ar;

        QJsonArray sjs;
        for (const double & d : EventDepoDataStats)
            sjs.push_back(d);
        js["Stat"] = sjs;

        json["DepoOverEvent"] = js;
    }
}

bool ACalorimeter::appendDataFromJson(const QJsonObject & json)
{
    QJsonObject pjs;
    bool ok = jstools::parseJson(json, "Properties", pjs);
    if (!ok)
    {
        QString err = "Cannot find Properties object in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    ACalorimeterProperties loadedProps;
    loadedProps.readFromJson(pjs);

    ok = addDepoDoseData(json, loadedProps);
    if (!ok) return false;

    if (Properties.CollectDepoOverEvent) addEventDepoDataFromJson(json, loadedProps);

    return true;
}

bool ACalorimeter::addDepoDoseData(const QJsonObject & json, const ACalorimeterProperties & loadedProps)
{
    if (DataHistogram == nullptr)
    {
        Properties.copyDepoDoseProperties(loadedProps);

        DataHistogram = new ATH3D("", "",
                                  Properties.Bins[0], Properties.Origin[0], Properties.Origin[0] + Properties.Bins[0]*Properties.Step[0],
                                  Properties.Bins[1], Properties.Origin[1], Properties.Origin[1] + Properties.Bins[1]*Properties.Step[1],
                                  Properties.Bins[2], Properties.Origin[2], Properties.Origin[2] + Properties.Bins[2]*Properties.Step[2]  );
    }
    else
    {
        if (!Properties.isSameDepoDoseProperties(loadedProps))
        {
            QString err = "Append calorimeter data error: Mismatch in calorimeter properties";
            AErrorHub::addQError(err);
            qWarning() << err;
            return false;
        }
    }

    QJsonObject djs;
    bool ok = jstools::parseJson(json, "XYZDepo", djs);
    if (!ok)
    {
        QString err = "Cannot find XYZDepo object in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }

    QJsonArray dataAr;
    ok = jstools::parseJson(djs, "Data", dataAr);
    if (!ok)
    {
        QString err = "Cannot find Data array for XYZDepo in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    QJsonArray statAr;
    ok = jstools::parseJson(djs, "Stat", statAr);
    if (!ok)
    {
        QString err = "Cannot find Stat array in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    if (statAr.size() != 11)
    {
        QString err = "Bad length of Stat array in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    double entries = 0;
    jstools::parseJson(djs, "Entries", entries);


    std::vector< std::vector< std::vector<double> > > Data;
    ok = loadDepositionFromJsonArr(dataAr, Data);
    if (!ok) return false;

    // --- load is OK ---

    for (int ix = 0; ix < Properties.Bins[0]; ix++)
    {
        const double x = Properties.Origin[0] + (0.5 + ix) * Properties.Step[0];
        for (int iy = 0; iy < Properties.Bins[1]; iy++)
        {
            const double y = Properties.Origin[1] + (0.5 + iy) * Properties.Step[1];
            for (int iz = 0; iz < Properties.Bins[2]; iz++)
            {
                const double z = Properties.Origin[2] + (0.5 + iz) * Properties.Step[2];
                DataHistogram->Fill(x, y, z, Data[ix][iy][iz]);
            }
        }
    }

    for (int i = 0; i < 11; i++)
        Stats[i] += statAr[i].toDouble();
    Entries += entries;

    DataHistogram->setStatistics(Stats);
    DataHistogram->SetEntries(Entries);

    return true;
}

bool ACalorimeter::addEventDepoDataFromJson(const QJsonObject & json, const ACalorimeterProperties & loadedProps)
{
    bool bIdenticalBinning;
    if (EventDepoData == nullptr)
    {
        Properties.copyEventDepoProperties(loadedProps);
        EventDepoData = new ATH1D("", "", loadedProps.EventDepoBins, loadedProps.EventDepoFrom, loadedProps.EventDepoTo);
        bIdenticalBinning = true;
    }
    else
    {
        bIdenticalBinning = (Properties.isSameEventDepoProperties(loadedProps));
    }

    QJsonObject djs;
    bool ok = jstools::parseJson(json, "DepoOverEvent", djs);
    if (!ok)
    {
        QString err = "Cannot find DepoOverEvent object in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }

    QJsonArray dataAr;
    ok = jstools::parseJson(djs, "Data", dataAr);
    if (!ok)
    {
        QString err = "Cannot find Data array for DepoOverEvent in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    QJsonArray statAr;
    ok = jstools::parseJson(djs, "Stat", statAr);
    if (!ok)
    {
        QString err = "Cannot find Stat array for DepoOverEvent in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    if (statAr.size() != 5)
    {
        QString err = "Bad length of Stat array for DepoOverEvent in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }

    std::vector<std::pair<double,double>> data;
    ok = loadEventDepoFromJsonArr(dataAr, data);
    if (!ok) return false;

    for (size_t i = 0; i < data.size(); i++)
        if (bIdenticalBinning)
            EventDepoData->AddBinContent(i, data[i].second);
        else
            EventDepoData->Fill(data[i].first, data[i].second);

    if (bIdenticalBinning)
    {
        for (int i = 0; i < 5; i++)
            EventDepoDataStats[i] += statAr[i].toDouble();
    }
    else
    {
        for (int i = 0; i < 4; i++) EventDepoDataStats[i] = 0;
        EventDepoDataStats[4] += statAr[4].toDouble();
    }

    EventDepoData->setStats(EventDepoDataStats);

    return true;
}

bool ACalorimeter::loadDepositionFromJsonArr(const QJsonArray & ar, std::vector< std::vector< std::vector<double> > > & data) const
{
    data.resize(Properties.Bins[0]);
    for (std::vector<std::vector<double>> & ary : data)
    {
        ary.resize(Properties.Bins[1]);
        for (std::vector<double> & arz : ary)
            arz = std::vector<double>(Properties.Bins[2], 0);
    }

    if (ar.size() != Properties.Bins[2] * Properties.Bins[1])
    {
        QString err = "Bad length of Data array in calorimeter json";
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }

    int iCounter = 0;
    for (int iz = 0; iz < Properties.Bins[2]; iz++)
        for (int iy = 0; iy < Properties.Bins[1]; iy++)
        {
            QJsonArray el = ar[iCounter].toArray();
            iCounter++;

            if (el.size() != Properties.Bins[0])
            {
                QString err = "Bad length of Data array element in calorimeter json";
                AErrorHub::addQError(err);
                qWarning() << err;
                return false;
            }

            for (int ix = 0; ix < Properties.Bins[0]; ix++)
                data[ix][iy][iz] = el[ix].toDouble();
        }

    return true;
}

bool ACalorimeter::loadEventDepoFromJsonArr(const QJsonArray & ar, std::vector<std::pair<double, double>> & data) const
{
    data.reserve(Properties.EventDepoBins+2);

    for (int i = 0; i < Properties.EventDepoBins+2; i++)
    {
        QJsonArray el = ar[i].toArray();

        if (el.size() != 2)
        {
            QString err = "Bad length of Data array element for DepoOverEvent in calorimeter json";
            AErrorHub::addQError(err);
            qWarning() << err;
            return false;
        }

        double pos = el[0].toDouble();
        double val = el[1].toDouble();

        data.push_back({pos, val});
    }
    return true;
}
