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
