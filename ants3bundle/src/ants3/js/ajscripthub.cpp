#include "ajscripthub.h"
#include "ajscriptmanager.h"
#include "adispatcherinterface.h"

// SI
#include "ademo_si.h"
#include "acore_si.h"
#include "amath_si.h"
#include "agraph_si.h"
#include "ahist_si.h"
#include "afarm_si.h"
#include "aconfig_si.h"
#include "aphotonsim_si.h"
#include "atrackrec_si.h"
#include "apartanalysis_si.h"
#include "aminijs_si.h"
#include "atree_si.h"
#include "ageo_si.h"

AJScriptHub & AJScriptHub::getInstance()
{
    static AJScriptHub instance;
    return instance;
}

AJScriptManager & AJScriptHub::manager()
{
    return getInstance().getJScriptManager();
}

void AJScriptHub::abort(const QString & message)
{
    AJScriptHub & hub = getInstance();
    hub.SM->abort();

    ADispatcherInterface::getInstance().abortTask();

    emit hub.showAbortMessage(message);
}

void AJScriptHub::addInterface(AScriptInterface * interface, QString name)
{
    SM->registerInterface(interface, name);
}

AJScriptHub::AJScriptHub()
{
    //qDebug() << ">Creating AJScriptManager and Generating/registering script units";
    SM = new AJScriptManager();

    SM->registerInterface(new ADemo_SI(),         "demo");
    SM->registerInterface(new ACore_SI(),         "core");
    SM->registerInterface(new AMath_SI(),         "math");
    SM->registerInterface(new AGeo_SI(),          "geo");
    SM->registerInterface(new AGraph_SI(),        "graph");
    SM->registerInterface(new AHist_SI(),         "hist");
    SM->registerInterface(new ATree_SI(),         "tree");
    SM->registerInterface(new AConfig_SI(),       "config");
    SM->registerInterface(new AFarm_SI(),         "farm");
    SM->registerInterface(new APhotonSim_SI(),    "lsim");
    SM->registerInterface(new ATrackRec_SI(),     "tracks");
    SM->registerInterface(new APartAnalysis_SI(), "partan");
    SM->registerInterface(new AMiniJS_SI(),       "mini");
}

AJScriptHub::~AJScriptHub()
{
    delete SM; SM = nullptr;
}
