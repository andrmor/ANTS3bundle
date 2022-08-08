#ifndef ASCRIPTOBJSTORE_H
#define ASCRIPTOBJSTORE_H

#include "arootobjcollection.h"

class AScriptObjStore
{
public:
    static AScriptObjStore & getInstance();
    static const AScriptObjStore & getConstInstance();

private:
    AScriptObjStore(){}

    AScriptObjStore(const AScriptObjStore &)            = delete;
    AScriptObjStore(AScriptObjStore &&)                 = delete;
    AScriptObjStore& operator=(const AScriptObjStore &) = delete;
    AScriptObjStore& operator=(AScriptObjStore &&)      = delete;

public:
    ARootObjCollection Graphs;
    ARootObjCollection Hists;
    ARootObjCollection Trees;

};

#endif // ASCRIPTOBJSTORE_H
