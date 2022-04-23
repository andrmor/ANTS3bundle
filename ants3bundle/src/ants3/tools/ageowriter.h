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

    enum EDraw {Sensors, PhotMons, PartMons, Calorimeters};
    QString drawText(const std::vector<QString> & textVector, int color, EDraw onWhat); // returns error

private:
    void generateSymbolMap();

    std::map<QString, AGeoSymbol> SymbolMap;
};

#endif // AGEOWRITER_H
