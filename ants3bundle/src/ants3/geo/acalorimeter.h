#ifndef ACALORIMETER_H
#define ACALORIMETER_H

#include "acalsettings.h"

#include <QString>
#include <vector>

class ATH3D;
class ATH1D;
class AGeoObject;
class QJsonObject;
class QJsonArray;

class ACalorimeter
{
public:
    ACalorimeter(){}
    ACalorimeter(const AGeoObject * CalorimeterGeoObject);
    ~ACalorimeter();

    bool readFromGeoObject(const AGeoObject * geoObj);

    void clearData();

    void writeDataToJson(QJsonObject & json, int index) const;

    bool appendDataFromJson(const QJsonObject & json);  // It is possible that histograms = nullptr

    double getTotalEnergy() const;

    ACalorimeterProperties Properties;

    ATH3D * DataHistogram = nullptr;
    std::array<double,11> Stats;
    double Entries = 0;

    ATH1D * EventDepoData = nullptr;
    std::array<double,5> EventDepoDataStats; // last is num events

private:
    bool addEventDepoDataFromJson(const QJsonObject & json, const ACalorimeterProperties & loadedProps);
    bool addDepoDoseData(const QJsonObject & json, const ACalorimeterProperties & loadedProps);
    bool loadDepositionFromJsonArr(const QJsonArray & ar, std::vector<std::vector<std::vector<double> > > & data) const;
    bool loadEventDepoFromJsonArr(const QJsonArray & ar, std::vector<std::pair<double,double>> & data) const;
};

#endif // ACALORIMETER_H
