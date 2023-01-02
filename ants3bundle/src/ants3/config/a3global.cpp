#include "a3global.h"
#include "ajsontools.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

A3Global & A3Global::getInstance()
{
    static A3Global instance;
    return instance;
}

const A3Global &A3Global::getConstInstance()
{
    return getInstance();
}

A3Global::A3Global()
{
    QString TargetDir(TARGET_DIR);
    ExamplesDir  = TargetDir + "/EXAMPLES";
    ResourcesDir = TargetDir + "/DATA";

    ExecutableDir = QDir::currentPath();
    ExchangeDir = ExecutableDir + "/Exchange";
    QDir dir = QDir(ExchangeDir);
    if (!dir.exists()) QDir().mkdir(ExchangeDir);
    //qDebug() << "Executable dir set to:"<< ExecutableDir;
    //qDebug() << "Exchange   dir set to:"<< ExchangeDir;

    QString AntsBaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/ants3";
    if (!QDir(AntsBaseDir).exists()) QDir().mkdir(AntsBaseDir);

    QuicksaveDir = AntsBaseDir + "/Quicksave";
    if (!QDir(QuicksaveDir).exists()) QDir().mkdir(QuicksaveDir);

    ConfigDir = AntsBaseDir + "/Config";
    if (QDir(ConfigDir).exists()) loadConfig();
    else
    {
        qDebug() << "Config dir not found, skipping config load, creating new config dir" ;
        QDir().mkdir(ConfigDir); //dir not found, skipping load config
    }
}

void A3Global::init()
{
    qDebug() << "Init of global config";
/*
#ifdef GUI
    if (!RootStyleScript.isEmpty())
    {
        //running root TStyle script
        AJavaScriptManager* SM = new AJavaScriptManager(0);
        AGStyle_SI* GStyleInterface  = new  AGStyle_SI(); //deleted by the SM
        SM->RegisterInterfaceAsGlobal(GStyleInterface);
        SM->Evaluate(RootStyleScript);
        SM->deleteLater();
    }
    gStyle->SetOptTitle(0);  // disables drawing of the title of ROOT histograms / graphs
#endif
*/

}

#include "aerrorhub.h"
bool A3Global::checkExchangeDir()
{
    if (ExchangeDir.isEmpty())
    {
        AErrorHub::addError("Exchange directory is not set!");
        return false;
    }
    if (!QDir(ExchangeDir).exists())
    {
        AErrorHub::addError("Exchange directory does not exist!");
        return false;
    }
    return true;
}

void A3Global::saveConfig()
{
    QJsonObject js;

    js["ExchangeDir"]   = ExchangeDir;
    js["LastSaveDir"]   = LastSaveDir;
    js["LastLoadDir"]   = LastLoadDir;

    js["AutoCheckGeometry"] = AutoCheckGeometry;
    js["NumSegmentsTGeo"]   = NumSegmentsTGeo;
    js["BinsX"] = BinsX;
    js["BinsY"] = BinsY;
    js["BinsZ"] = BinsZ;
    js["OpenImageExternalEditor"] = OpenImageExternalEditor;

//    js["RootStyleScript"] = RootStyleScript;

/*
    js["RecTreeSave_IncludePMsignals"] = RecTreeSave_IncludePMsignals;
    js["RecTreeSave_IncludeRho"] = RecTreeSave_IncludeRho;
    js["RecTreeSave_IncludeTrue"] = RecTreeSave_IncludeTrue;
    js["SimTextSave_IncludeNumPhotons"] = SimTextSave_IncludeNumPhotons;
    js["SimTextSave_IncludePositions"] = SimTextSave_IncludePositions;
*/

    js["JavaScriptJson"] = JavaScriptJson;
    js["PythonJson"]     = PythonJson;
    js["SW_FontSize"]    = SW_FontSize;
    js["SW_FontFamily"]  = SW_FontFamily;
    js["SW_FontWeight"]  = SW_FontWeight;
    js["SW_Italic"]      = SW_Italic;

    js["TrackVisAttributes"] = TrackVisAttributes;

/*
    js["DefaultWebSocketPort"] = DefaultWebSocketPort;
    js["DefaultWebSocketIP"] = DefaultWebSocketIP;
    js["RootServerPort"] = RootServerPort;
    js["RunRootServerOnStart"] = fRunRootServerOnStart;
    //js["ExternalJSROOT"] = ExternalJSROOT;
*/

    QJsonObject json;
    json["ANTS3config"] = js;
    jstools::saveJsonToFile(json, ConfigDir + '/' + ConfigFileName);
}

void A3Global::loadConfig()
{
    QJsonObject json;
    const QString fileName = ConfigDir + '/' + ConfigFileName;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        qDebug() << "Could not open global config file:" << fileName;
        return;
    }

    QJsonObject js;
    ok = jstools::parseJson(json, "ANTS3config", js);
    if (!ok)
    {
        qDebug() << "Bad format of the config file:" << fileName;
        return;
    }

    jstools::parseJson(js, "ExchangeDir", ExchangeDir);
    jstools::parseJson(js, "LastSaveDir", LastSaveDir);
    jstools::parseJson(js, "LastLoadDir", LastLoadDir);

    jstools::parseJson(js, "AutoCheckGeometry", AutoCheckGeometry);
    jstools::parseJson(js, "NumSegmentsTGeo", NumSegmentsTGeo);
    jstools::parseJson(js, "BinsX", BinsX);
    jstools::parseJson(js, "BinsY", BinsY);
    jstools::parseJson(js, "BinsZ", BinsZ);
    jstools::parseJson(js, "OpenImageExternalEditor", OpenImageExternalEditor);

    jstools::parseJson(js, "JavaScriptJson", JavaScriptJson);
    jstools::parseJson(js, "PythonJson", PythonJson);
    jstools::parseJson(js, "SW_FontSize", SW_FontSize);
    jstools::parseJson(js, "SW_FontFamily", SW_FontFamily);
    jstools::parseJson(js, "SW_FontWeight", SW_FontWeight);
    jstools::parseJson(js, "SW_Italic", SW_Italic);

    jstools::parseJson(js, "TrackVisAttributes", TrackVisAttributes);
}

QString A3Global::getQuickFileName(int index) const
{
    return QString("%0/QuickSave%1.json").arg(QuicksaveDir).arg(index);
}
