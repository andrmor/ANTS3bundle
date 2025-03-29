#ifndef AMULTIDRAWRECORD_H
#define AMULTIDRAWRECORD_H

#include <vector>

class AMultidrawRecord
{
public:
    std::vector<int> BasketItems;

    size_t NumX = 2;
    size_t NumY = 1;

    void init();
};

#endif // AMULTIDRAWRECORD_H
