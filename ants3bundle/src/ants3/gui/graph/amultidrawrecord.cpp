#include "amultidrawrecord.h"

void AMultidrawRecord::init()
{
    const int size = BasketItems.size();

    if      (size == 2) { NumX = 2; NumY = 1;}
    else if (size == 3) { NumX = 3; NumY = 1;}
    else if (size == 4) { NumX = 2; NumY = 2;}
    else if (size == 5) { NumX = 2; NumY = 3;}
    else if (size == 6) { NumX = 2; NumY = 3;}
    else                { NumX = 3; NumY = 3;}
}
