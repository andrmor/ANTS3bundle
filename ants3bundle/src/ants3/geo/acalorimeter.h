#ifndef ACALORIMETER_H
#define ACALORIMETER_H

#include "acalsettings.h"

#include <QString>
#include <vector>

class TH3D;
class ATH1D;
class AGeoObject;
class QJsonObject;

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

    void writeDataToJson(QJsonObject & json) const; // !!!*** TODO
    void readDataFromJson(const QJsonObject & json);  // !!!*** TODO
    void append(const ACalorimeter & from); // !!!*** TODO
    void overrideDataFromJson(const QJsonObject & json); // !!!*** TODO

 bool appendDataFromJson(const QJsonObject & json);  // It is possible that Deposition hist is nullptr!

    QString Name = "Undefined";
    ACalorimeterProperties Properties;

    TH3D * Deposition = nullptr;

    int getTotalEnergy() const;  // !!!*** TODO

private:
    void initXYHist(); // !!!*** TODO

};

#endif // ACALORIMETER_H
