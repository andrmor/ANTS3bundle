#ifndef AMULTIDRAWRECORD_H
#define AMULTIDRAWRECORD_H

#include <vector>

class AMultidrawRecord
{
public:
    std::vector<int> BasketItems;

    size_t NumX = 2;
    size_t NumY = 1;

    bool   EnforceMargins = false;
    double MarginLeft   = 0.1;
    double MarginRight  = 0.1;
    double MarginTop    = 0.1;
    double MarginBottom = 0.1;

    bool   ScaleLabels = false;
    double ScaleLabelsBy = 1.0;

    bool   ScaleXoffset = false;
    double ScaleXoffsetBy = 1.0;
    bool   ScaleYoffset = false;
    double ScaleYoffsetBy = 1.0;
    bool   ScaleZoffset = false;
    double ScaleZoffsetBy = 1.0;

    void init();
};

#endif // AMULTIDRAWRECORD_H
