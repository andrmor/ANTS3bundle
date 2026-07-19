#include "abasketitem.h"

#include "TObject.h"
#include "TList.h"

#include <QDebug>

ABasketItem::~ABasketItem()
{
    //cannot delete objects here -> destructor is called automatically e.g. on resize of a vector with BasketItems
}

void ABasketItem::clearObjects()
{
    for (ADrawObject & obj : DrawObjects)
    {
        TList * list = dynamic_cast<TList*>(obj.Pointer);
        if (list)
        {
            for (TObject * tobj : *list) delete tobj;
            list->Clear();
        }

        delete obj.Pointer;
    }
    DrawObjects.clear();
}
