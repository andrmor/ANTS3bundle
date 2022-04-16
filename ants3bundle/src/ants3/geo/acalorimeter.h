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

    //configuration
    bool readFromGeoObject(const AGeoObject * geoObj);

    // data handling
    void clearData();

 void writeDataToJson(QJsonObject & json, int index) const; // !!!*** TODO
    void readDataFromJson(const QJsonObject & json);  // !!!*** TODO
    void append(const ACalorimeter & from); // !!!*** TODO
    void overrideDataFromJson(const QJsonObject & json); // !!!*** TODO

 bool appendDataFromJson(const QJsonObject & json);  // It is possible that Deposition hist is nullptr!

    QString Name = "Undefined";
    ACalorimeterProperties Properties;

    ATH3D * Deposition = nullptr;
    std::array<double,11> Stats;
    int Entries = 0;

    int getTotalEnergy() const;  // !!!*** TODO

private:
    void initXYHist(); // !!!*** TODO

    bool loadDepositionFromJsonArr(const QJsonArray & ar, std::vector<std::vector<std::vector<double> > > & data) const;
};

#endif // ACALORIMETER_H
