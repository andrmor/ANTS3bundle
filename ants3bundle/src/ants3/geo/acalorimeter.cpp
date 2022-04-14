#include "acalorimeter.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ajsontools.h"
#include "ajsontoolsroot.h"
#include "aroothistappenders.h"

#include <QDebug>

#include "TH3D.h"
#include "TString.h"

ACalorimeter::ACalorimeter() : name("Undefined") {}

ACalorimeter::ACalorimeter(const AGeoObject * MonitorGeoObject)
{
    readFromGeoObject(MonitorGeoObject);
}

ACalorimeter::~ACalorimeter()
{
    clearData();
}

void ACalorimeter::clearData()
{
    delete xy;     xy     = nullptr;
}

int ACalorimeter::getHits() const
{
    if (!xy) return 0;
    return xy->GetEntries();
}

void ACalorimeter::fillForParticle(double x, double y, double z, double energy)
{
    if (xy) xy->Fill(x,y,z, energy);

    /*
    if (energy)
    {
        switch (config.energyUnitsInHist)
        {
        case 0: Energy *= 1.0e6; break;
        case 1: Energy *= 1.0e3; break;
        case 2: break;
        case 3: Energy *= 1.0e-3;break;
        }
        energy->Fill(Energy);
    }
    */
}

bool ACalorimeter::readFromGeoObject(const AGeoObject * CalorimeterRecord) // !!!*** TODO
{
    /*
    const ATypeMonitorObject* mon = dynamic_cast<const ATypeMonitorObject*>(CalorimeterRecord->Type);
    if (!mon)
    {
        qWarning() << "This is not a monitor type AGeoObject!";
        return false;
    }
    */

    //config = mon->config;

    name = CalorimeterRecord->Name;

    initXYHist();

    return true;
}

void ACalorimeter::writeDataToJson(QJsonObject & json) const // !!!*** TODO
{
    //json["XY"]     = jstools::regularTh2dToJson(xy);
}

void ACalorimeter::readDataFromJson(const QJsonObject &json) // !!!*** TODO
{
    clearData();

//    jstools::parseJson(json, "XY",     xy);
}

void ACalorimeter::append(const ACalorimeter & from) // !!!*** TODO
{
    //appendTH2D(xy,     from.xy);
}

void ACalorimeter::initXYHist() // !!!*** TODO
{
    /*
    delete xy;
    const double limit2 = ( config.shape == 0 ? config.size2 : config.size1 ); // 0 - rectangular, 1 - round
    xy = new TH2D("", "", config.xbins, -config.size1, config.size1, config.ybins, -limit2, limit2);
    xy->SetXTitle("X, mm");
    xy->SetYTitle("Y, mm");
    */

    /*
    TString title = "";
    switch (config.energyUnitsInHist)
    {
    case 0: title = "Energy, meV"; break;
    case 1: title = "Energy, eV"; break;
    case 2: title = "Energy, keV"; break;
    case 3: title = "Energy, MeV"; break;
    }
    */
}

//#include "ahistogram.h"
#include "ath.h"
#include <QJsonObject>
#include <QJsonArray>
void ACalorimeter::overrideDataFromJson(const QJsonObject & json) // !!!*** TODO
{
    /*
    QJsonObject jSpatial = json["Spatial"].toObject();
    double xfrom = jSpatial["xfrom"].toDouble();
    double xto   = jSpatial["xto"].toDouble();
    double yfrom = jSpatial["yfrom"].toDouble();
    double yto   = jSpatial["yto"].toDouble();

    QJsonArray dataAr = jSpatial["data"].toArray();
    int ybins = dataAr.size();
    std::vector<std::vector<double>> dataVec;
    dataVec.resize(ybins);
    for (int iy=0; iy<ybins; iy++)
    {
        QJsonArray row = dataAr[iy].toArray();
        int xbins = row.size();
        dataVec[iy].resize(xbins);
        for (int ix=0; ix<xbins; ix++)
            dataVec[iy][ix] = row[ix].toDouble();
    }
    QJsonArray statAr = jSpatial["stat"].toArray();
    std::vector<double> statVec;
    for (int i=0; i<statAr.size(); i++)
        statVec.push_back(statAr[i].toDouble());
    ATH2D * hist = new ATH2D("", "", 100, 0, 1.0, 100, 0, 1.0);
    hist->Import(xfrom, xto, yfrom, yto, dataVec, statVec);
    delete xy; xy = hist;
    */
}
