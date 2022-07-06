#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "adispatcherinterface.h"

#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
#endif

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

AScriptHub & AScriptHub::getInstance()
{
    static AScriptHub instance;
    return instance;
}

AJScriptManager & AScriptHub::manager()
{
    return getInstance().getJScriptManager();
}

void AScriptHub::abort(const QString & message)
{
    AScriptHub & hub = getInstance();
    hub.JSM->abort();
#ifdef ANTS3_PYTHON
    hub.PyM->abort();
#endif

    ADispatcherInterface::getInstance().abortTask();

    emit hub.showAbortMessage(message);
}

void AScriptHub::addInterface(AScriptInterface * interface, QString name)
{
    JSM->registerInterface(interface, name);
#ifdef ANTS3_PYTHON
    PyM->registerInterface(interface, name);
#endif
}

void AScriptHub::finalizeInit()
{
#ifdef ANTS3_PYTHON
    PyM->finalizeInit();
#endif
}

AScriptHub::AScriptHub()
{
    //qDebug() << ">Creating AJScriptManager and Generating/registering script units";
    JSM = new AJScriptManager();
#ifdef ANTS3_PYTHON
    PyM = new APythonScriptManager();
#endif

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

AScriptHub::~AScriptHub()
{
    delete JSM; JSM = nullptr;
#ifdef ANTS3_PYTHON
    delete PyM; PyM = nullptr;
#endif
}
