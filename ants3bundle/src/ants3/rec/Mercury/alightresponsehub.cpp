#include "alightresponsehub.h"
#include "lrmodel.h"

ALightResponseHub::ALightResponseHub() {}

ALightResponseHub & ALightResponseHub::getInstance()
{
    static ALightResponseHub instance;
    return instance;
}

const ALightResponseHub &ALightResponseHub::getConstInstance()
{
    return ALightResponseHub::getInstance();
}

