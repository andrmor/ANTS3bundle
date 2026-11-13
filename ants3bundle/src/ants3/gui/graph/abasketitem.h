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

    QString ParentGroup;

    bool    isGroup = false;
    bool    Collapsed = false;

    void    clearObjects();

    QString getGroupFullPath() const; // for group: returns "ParentGroup/Name", otherwise ""

    //runtime properties
    bool _toErase = false; // used in rearrangment to flag items to remove
};

#endif // ABASKETITEM_H
