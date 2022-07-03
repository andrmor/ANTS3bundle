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
#include "asensor_si.h"
#include "aparticlesim_si.h"

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

    addInterface(new ADemo_SI(),         "demo");
    addInterface(new ACore_SI(),         "core");
    addInterface(new AMath_SI(),         "math");
    addInterface(new AConfig_SI(),       "config");
    addInterface(new AFarm_SI(),         "farm");
    addInterface(new AGeo_SI(),          "geo");
    addInterface(new ASensor_SI(),       "sens");
    addInterface(new APhotonSim_SI(),    "lsim");
    addInterface(new AParticleSim_SI(),  "psim");
    addInterface(new ATrackRec_SI(),     "tracks");
    addInterface(new APartAnalysis_SI(), "partan");
    addInterface(new AMiniJS_SI(),       "mini");
    addInterface(new AGraph_SI(),        "graph");
    addInterface(new AHist_SI(),         "hist");
    addInterface(new ATree_SI(),         "tree");
}

AJScriptHub::~AJScriptHub()
{
    delete SM; SM = nullptr;
}
