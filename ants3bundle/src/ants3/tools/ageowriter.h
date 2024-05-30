#ifndef AGEOWRITER_H
#define AGEOWRITER_H

#include <map>
#include <vector>

#include <QString>

class QJsonObject;

struct AGeoSymbol
{
    AGeoSymbol(std::vector<double> x, std::vector<double> y);
    AGeoSymbol(){}

    QString Symbol;
    std::vector<std::pair<double, double>> Coordinates;
};

class AGeoWriter
{
public:
    AGeoWriter();

    double SizeForSensors      = 10.0;
    double SizeForMonitors     = 10.0;
    double SizeForCalorimeters = 10.0;
    double SizeForPhotFuncts   = 10.0;

    void setOrientationRoot(double latitude, double longitude) {Latitude = latitude; Longitude = longitude;}

    enum EDraw {Sensors, PhotMons, PartMons, Calorimeters, PhotonFunctional};
    void drawText(const std::vector<QString> & textVector, int color, EDraw onWhat);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

private:
    void generateSymbolMap();

    std::map<QString, AGeoSymbol> SymbolMap;

    double Latitude  = 60.0;
    double Longitude = -120.0;
};

#endif // AGEOWRITER_H
