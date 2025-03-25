#ifndef ABASKETITEM_H
#define ABASKETITEM_H

#include "adrawobject.h"

#include <QString>

#include <vector>

class ABasketItem
{
public:
    ~ABasketItem();

public:
    std::vector<ADrawObject> DrawObjects;
    QString Name;
    QString Type;

    void clearObjects();

    //runtime properties
    bool _flag = false; // used in rearrangment to flag items to remove
};

#endif // ABASKETITEM_H
