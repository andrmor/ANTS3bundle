#include "ademo_si.h"
#include "ademomanager.h"

ADemo_SI::ADemo_SI() :
    AScriptInterface(){}

bool ADemo_SI::run(int numLocalProc)
{
    return ADemoManager::getInstance().run(numLocalProc);
}
