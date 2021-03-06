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
    delete Deposition; Deposition = nullptr;
    Stats.fill(0);
    Entries = 0;
}

int ACalorimeter::getTotalEnergy() const
{
    if (!Deposition) return 0;
    return Stats[0];
}

bool ACalorimeter::readFromGeoObject(const AGeoObject * geoObj) // !!!*** TODO
{
    const ACalorimeterProperties * props = geoObj->getCalorimeterProperties();
    if (!props)
    {
        AErrorHub::addQError("Provided geo object is not a calorimeter!");
        return false;
    }

    Properties = *props;

    Name = geoObj->Name;

    return true;
}

void ACalorimeter::writeDataToJson(QJsonObject & json, int index) const // !!!*** TODO
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
                    el.push_back (Deposition->GetBinContent(ix+1, iy+1, iz+1) );
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

    const bool bMergeMode = (Deposition != nullptr);
    if (bMergeMode)
    {
        if (loadedProps != Properties)
        {
            QString err = "Append calorimeter data error: Mismatch in calorimeter properties";
            AErrorHub::addQError(err);
            qWarning() << err;
            return false;
        }
    }
    else
    {
        Properties = loadedProps;
        Deposition = new ATH3D("", "",
                               Properties.Bins[0], Properties.Origin[0], Properties.Origin[0] + Properties.Bins[0]*Properties.Step[0],
                               Properties.Bins[1], Properties.Origin[1], Properties.Origin[1] + Properties.Bins[1]*Properties.Step[1],
                               Properties.Bins[2], Properties.Origin[2], Properties.Origin[2] + Properties.Bins[2]*Properties.Step[2]  );
    }


    QJsonObject djs;
    ok = jstools::parseJson(json, "XYZDepo", djs);
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
        QString err = "Cannot find Data array in calorimeter json";
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
                Deposition->Fill(x, y, z, Data[ix][iy][iz]);
            }
        }
    }

    for (int i = 0; i < 11; i++)
        Stats[i] += statAr[i].toDouble();
    Entries += entries;

    Deposition->setStatistics(Stats);
    Deposition->SetEntries(Entries);

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
