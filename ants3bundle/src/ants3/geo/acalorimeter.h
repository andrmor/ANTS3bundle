#ifndef ACALORIMETER_H
#define ACALORIMETER_H

#include <QString>
#include <vector>

class TH3D;
class ATH1D;
class AGeoObject;
class QJsonObject;

class ACalorimeter
{
public:
  ACalorimeter();
  ACalorimeter(const AGeoObject * CalorimeterGeoObject);
  ~ACalorimeter();

//runtime functions
  void fillForParticle(double x, double y, double z, double energy);

//configuration
  bool readFromGeoObject(const AGeoObject * CalorimeterRecord); // !!!*** TODO

  void writeDataToJson(QJsonObject & json) const; // !!!*** TODO
  void readDataFromJson(const QJsonObject & json);  // !!!*** TODO

// stat data handling
  void clearData();

  QString name;

  TH3D * xy     = nullptr;

  //AMonitorConfig config;

  int getHits() const;

  void append(const ACalorimeter & from); // !!!*** TODO

  void overrideDataFromJson(const QJsonObject & json); // !!!*** TODO

private:
  void initXYHist(); // !!!*** TODO

};

#endif // ACALORIMETER_H
