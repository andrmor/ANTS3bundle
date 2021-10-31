#include "ajscripthub.h"
#include "ajscriptmanager.h"

// SI
#include "ademo_si.h"
#include "amath_si.h"
#include "afarm_si.h"
#include "aphotonsim_si.h"

AJScriptHub &AJScriptHub::getInstance()
{
    static AJScriptHub instance;
    return instance;
}

AJScriptManager &AJScriptHub::manager()
{
    return getInstance().getJScriptManager();
}

void AJScriptHub::abort(const QString & message)
{
    AJScriptHub & hub = getInstance();
    hub.SM->abort();
    emit hub.showAbortMessage(message);
}

AJScriptHub::AJScriptHub()
{
    SM = new AJScriptManager();

    SM->registerInterface(new ADemo_SI(),      "demo");
    SM->registerInterface(new AMath_SI(),      "math");
    SM->registerInterface(new AFarm_SI(),      "farm");
    SM->registerInterface(new APhotonSim_SI(), "lsim");
}

AJScriptHub::~AJScriptHub()
{
    delete SM; SM = nullptr;
}
