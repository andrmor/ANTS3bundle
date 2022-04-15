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

    // g4ants3 side
    void fillForParticle(double x, double y, double z, double energy);

    // data handling
    void clearData();
    void writeDataToJson(QJsonObject & json) const; // !!!*** TODO
    void readDataFromJson(const QJsonObject & json);  // !!!*** TODO
    void append(const ACalorimeter & from); // !!!*** TODO
    void overrideDataFromJson(const QJsonObject & json); // !!!*** TODO

    QString Name = "Undefined";
    ACalorimeterProperties Properties;

    TH3D * xy     = nullptr;

    int getTotalEnergy() const;  // !!!*** TODO

private:
    void initXYHist(); // !!!*** TODO

};

#endif // ACALORIMETER_H
