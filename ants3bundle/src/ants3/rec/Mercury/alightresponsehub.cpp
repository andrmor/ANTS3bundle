#include "alightresponsehub.h"
#include "alrfplotter.h"
#include "lrmodel.h"

ALightResponseHub::ALightResponseHub() :
    LrfPlotter(new ALrfPlotter()) {}

ALightResponseHub & ALightResponseHub::getInstance()
{
    static ALightResponseHub instance;
    return instance;
}

const ALightResponseHub &ALightResponseHub::getConstInstance()
{
    return ALightResponseHub::getInstance();
}

