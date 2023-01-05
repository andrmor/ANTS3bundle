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

    void writeDataToJson(QJsonObject & json, int index) const; // !!!*** TODO

    bool appendDataFromJson(const QJsonObject & json);  // It is possible that Deposition hist is nullptr!

    int getTotalEnergy() const;

    //QString Name = "Undefined";
    ACalorimeterProperties Properties;

    ATH3D * Deposition = nullptr;
    std::array<double,11> Stats;
    double Entries = 0;

private:
    bool loadDepositionFromJsonArr(const QJsonArray & ar, std::vector<std::vector<std::vector<double> > > & data) const;
};

#endif // ACALORIMETER_H
