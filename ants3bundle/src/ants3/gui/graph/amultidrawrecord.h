#ifndef AMULTIDRAWRECORD_H
#define AMULTIDRAWRECORD_H

#include <vector>

#include <QString>

class QJsonObject;

class AMultidrawRecord
{
public:
    std::vector<int> BasketItems;

    int    NumX = 2;
    int    NumY = 1;

    bool   XY = true; // horizontal, else YX (vertical)

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

    bool   ShowIdentifiers = false;
    QString Identifiers = "A,B,C,D,E,F,G,H";
    double IdentBoxX1 = 0.2;
    double IdentBoxX2 = 0.4;
    double IdentBoxY1 = 0.75;
    double IdentBoxY2 = 0.85;
    int    IdentBoxTextColor = 1;
    int    IdentBoxTextAlign = 1;
    int    IdentBoxTextFont  = 42;
    double IdentBoxTextSize  = 0.15;
    int    IdentBoxBorderColor = 1;
    int    IdentBoxBorderWidth = 0;
    int    IdentBoxBorderStyle = 1;

    void init();

    void writeToJson(QJsonObject & json) const;
    void readFronJson(const QJsonObject & json);
};

#endif // AMULTIDRAWRECORD_H
