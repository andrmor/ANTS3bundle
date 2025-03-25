#include "ascriptobjstore.h"

AScriptObjStore & AScriptObjStore::getInstance()
{
    static AScriptObjStore instance;
    return instance;
}

const AScriptObjStore & AScriptObjStore::getConstInstance()
{
    return AScriptObjStore::getInstance();
}

AScriptObjStore::~AScriptObjStore()
{
    Graphs.clear();
    Hists.clear();
    Trees.clear(); // if there are trees, was causing crash on exit of ANTS3 (delete of TTRee) --> fixed by adding clear to main.cpp
}
