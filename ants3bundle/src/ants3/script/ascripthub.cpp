#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "adispatcherinterface.h"

#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
    #include "aminipython_si.h"
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
#include "arootstyle_si.h"

AScriptHub & AScriptHub::getInstance()
{
    static AScriptHub instance;
    return instance;
}

AJScriptManager & AScriptHub::manager()
{
    return getInstance().getJScriptManager();
}

void AScriptHub::abort(const QString & message, EScriptLanguage lang)
{
    AScriptHub & hub = getInstance();
#ifdef ANTS3_PYTHON
    if (lang == EScriptLanguage::Python)     hub.PythonM->abort();
#endif
    if (lang == EScriptLanguage::JavaScript) hub.JavaScriptM->abort();

    ADispatcherInterface::getInstance().abortTask();

#ifdef ANTS3_PYTHON
    if (lang == EScriptLanguage::Python)     emit hub.showAbortMessage_P(message);
#endif
    if (lang == EScriptLanguage::JavaScript) emit hub.showAbortMessage_JS(message);
}

#include "ageowin_si.h"
void AScriptHub::addCommonInterface(AScriptInterface * interface, QString name)
{
    JavaScriptM->registerInterface(interface, name);

    AGeoWin_SI * geoWin = dynamic_cast<AGeoWin_SI*>(interface);
    if (geoWin) geoWinInterfaces.push_back(geoWin);

#ifdef ANTS3_PYTHON
    AScriptInterface * twin = interface->cloneBase();
    PythonM->registerInterface(twin, name);

    if (geoWin) geoWinInterfaces.push_back(dynamic_cast<AGeoWin_SI*>(twin));
#endif
}

void AScriptHub::updateGeoWin(AGeometryWindow * GeoWin)
{
    for (AGeoWin_SI * inter : geoWinInterfaces) inter->updateGeoWin(GeoWin);
}

#include "agui_si.h"
void AScriptHub::addGuiScriptUnit()
{
    JavaScriptM->registerInterface(new AGui_JS_SI(), "gui");
#ifdef ANTS3_PYTHON
    //PythonM->registerInterface(new AGui_SI(EScriptLanguage::Python), "gui");
#endif
}

void AScriptHub::finalizeInit()
{
#ifdef ANTS3_PYTHON
    PythonM->finalizeInit();
#endif
}

void AScriptHub::outputText(const QString & text, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit outputText_JS(text);
    else                                     emit outputText_P(text);
}

void AScriptHub::outputHtml(const QString &text, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit outputHtml_JS(text);
    else                                     emit outputHtml_P(text);
}

void AScriptHub::clearOutput(EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit clearOutput_JS();
    else                                     emit clearOutput_P();
}

AScriptHub::AScriptHub()
{
    //qDebug() << ">Creating AJScriptManager and Generating/registering script units";
    JavaScriptM = new AJScriptManager();
#ifdef ANTS3_PYTHON
    PythonM = new APythonScriptManager();
#endif

    addCommonInterface(new ACore_SI(),         "core");

    //addCommonInterface(new AMath_SI(),         "math");  // conflicts with inbuild Python module "math"
    JavaScriptM->registerInterface(new AMath_SI(), "math");
#ifdef ANTS3_PYTHON
    PythonM->registerInterface(new AMath_SI(),     "Math");
#endif

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
    addCommonInterface(new ARootStyle_SI(),    "root");
    addCommonInterface(new ADemo_SI(),         "demo");

    JavaScriptM->registerInterface(new AMiniJS_SI(), "mini");
#ifdef ANTS3_PYTHON
    PythonM->registerInterface(new AMiniPython_SI(), "mini");
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
