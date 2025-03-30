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

    void init();
};

#endif // AMULTIDRAWRECORD_H
