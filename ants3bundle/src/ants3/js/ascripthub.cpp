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
#ifdef ANTS3_PYTHON
    hub.PythonM->abort();
#endif
    hub.JavaScriptM->abort();

    ADispatcherInterface::getInstance().abortTask();

    emit hub.showAbortMessage(message);
}

void AScriptHub::addCommonInterface(AScriptInterface * interface, QString name)
{
    JavaScriptM->registerInterface(interface, name);

#ifdef ANTS3_PYTHON
    AScriptInterface * twin = interface->cloneBase();
    PythonM->registerInterface(twin, name);
#endif
}

void AScriptHub::finalizeInit()
{
#ifdef ANTS3_PYTHON
    PythonM->finalizeInit();
#endif
}

void AScriptHub::outputText(const QString & text, AScriptLanguageEnum lang)
{
    if (lang == AScriptLanguageEnum::JavaScript) emit outputText_JS(text);
    else                                         emit outputText_P(text);
}

void AScriptHub::outputHtml(const QString &text, AScriptLanguageEnum lang)
{
    if (lang == AScriptLanguageEnum::JavaScript) emit outputHtml_JS(text);
    else                                         emit outputHtml_P(text);
}

void AScriptHub::clearOutput(AScriptLanguageEnum lang)
{
    if (lang == AScriptLanguageEnum::JavaScript) emit clearOutput_JS();
    else                                         emit clearOutput_P();
}

AScriptHub::AScriptHub()
{
    //qDebug() << ">Creating AJScriptManager and Generating/registering script units";
    JavaScriptM = new AJScriptManager();
      ACore_SI * coreJS = new ACore_SI(AScriptLanguageEnum::JavaScript);
      JavaScriptM->registerInterface(coreJS, "core");

#ifdef ANTS3_PYTHON
    PythonM = new APythonScriptManager();
      ACore_SI * coreP = new ACore_SI(AScriptLanguageEnum::Python);
      PythonM->registerInterface(coreP, "core");
#endif

    addCommonInterface(new AMath_SI(),         "math");
    addCommonInterface(new AConfig_SI(),       "config");
    addCommonInterface(new AFarm_SI(),         "farm");
    addCommonInterface(new AGeo_SI(),          "geo");
    addCommonInterface(new ASensor_SI(),       "sens");
    addCommonInterface(new APhotonSim_SI(),    "lsim");
    addCommonInterface(new AParticleSim_SI(),  "psim");
    addCommonInterface(new ATrackRec_SI(),     "tracks");
    addCommonInterface(new APartAnalysis_SI(), "partan");
    addCommonInterface(new AGraph_SI(),        "graph");
    addCommonInterface(new AHist_SI(),         "hist");
    addCommonInterface(new ATree_SI(),         "tree");
    addCommonInterface(new ADemo_SI(),         "demo");

    JavaScriptM->registerInterface(new AMiniJS_SI(), "mini");
#ifdef ANTS3_PYTHON
    //PythonM->registerInterface(new AMiniPython_SI(), "mini");
#endif
}

AScriptHub::~AScriptHub()
{
    qDebug() << "Destr for ScriptHub";
#ifdef ANTS3_PYTHON
    delete PythonM; PythonM = nullptr;
#endif
    delete JavaScriptM; JavaScriptM = nullptr;
}
