#ifndef AGEOWRITER_H
#define AGEOWRITER_H

#include <map>
#include <vector>

#include <QString>

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

    void setOrientationRoot(double latitude, double longitude) {Latitude = latitude; Longitude = longitude;}

    enum EDraw {Sensors, PhotMons, PartMons, Calorimeters};
    QString drawText(const std::vector<QString> & textVector, int color, EDraw onWhat); // returns error

private:
    void generateSymbolMap();

    std::map<QString, AGeoSymbol> SymbolMap;

    double Latitude = 60.0;
    double Longitude = -120.0;
};

#endif // AGEOWRITER_H
