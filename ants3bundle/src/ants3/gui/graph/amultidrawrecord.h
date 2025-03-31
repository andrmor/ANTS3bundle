#ifndef AMULTIDRAWRECORD_H
#define AMULTIDRAWRECORD_H

#include <vector>

class QJsonObject;

class AMultidrawRecord
{
public:
    std::vector<int> BasketItems;

    int    NumX = 2;
    int    NumY = 1;

    bool   EnforceMargins = false;
    double MarginLeft   = 0.1;
    double MarginRight  = 0.1;
    double MarginTop    = 0.1;
    double MarginBottom = 0.1;

    double ScaleLabels  = 1.0;

    double ScaleOffsetTitleX = 1.0;
    double ScaleOffsetTitleY = 1.0;
    double ScaleOffsetTitleZ = 1.0;

    double ScaleOffsetAxisX = 1.0;
    double ScaleOffsetAxisY = 1.0;
    double ScaleOffsetAxisZ = 1.0;

    //double ScaleAxesLines = 1.0;
    double ScaleDrawLines = 1.0;
    double ScaleMarkers   = 1.0;

    void init();

    void writeToJson(QJsonObject & json) const;
    void readFronJson(const QJsonObject & json);
};

#endif // AMULTIDRAWRECORD_H
