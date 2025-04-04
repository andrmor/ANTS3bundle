#include "abasketitem.h"

#include "TObject.h"

ABasketItem::~ABasketItem()
{
    //cannot delete objects here -> destructor is called automatically e.g. on resize of a vector with BasketItems
}

void ABasketItem::clearObjects()
{
   for (ADrawObject & obj : DrawObjects)
       delete obj.Pointer;
   DrawObjects.clear();
}
