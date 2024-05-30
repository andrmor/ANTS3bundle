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
    ResourcesDir = TargetDir + "/files";
    ExamplesDir  = TargetDir + "/files/examples";

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

#include "TStyle.h"
void A3Global::init()
{
    qDebug() << "Init of global config";
    gStyle->SetOptTitle(0);  // disables drawing of the title of ROOT histograms / graphs
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

#include "afarmhub.h"
#include "aroothttpserver.h"
void A3Global::saveConfig()
{
    QJsonObject json;

    json["ExchangeDir"] = ExchangeDir;
    json["LastSaveDir"] = LastSaveDir;
    json["LastLoadDir"] = LastLoadDir;

    json["AutoCheckGeometry"] = AutoCheckGeometry;
    json["NumSegmentsTGeo"]   = NumSegmentsTGeo;
    json["BinsX"] = BinsX;
    json["BinsY"] = BinsY;
    json["BinsZ"] = BinsZ;
    json["OpenImageExternalEditor"] = OpenImageExternalEditor;

    QJsonObject jsMa;
    DefaultDrawMargins.writeToJson(jsMa);
    json["DefaultDrawMargins"] = jsMa;

/*
    js["RecTreeSave_IncludePMsignals"] = RecTreeSave_IncludePMsignals;
    js["RecTreeSave_IncludeRho"] = RecTreeSave_IncludeRho;
    js["RecTreeSave_IncludeTrue"] = RecTreeSave_IncludeTrue;
    js["SimTextSave_IncludeNumPhotons"] = SimTextSave_IncludeNumPhotons;
    js["SimTextSave_IncludePositions"] = SimTextSave_IncludePositions;
*/

    json["JavaScriptJson"] = JavaScriptJson;
    json["PythonJson"]     = PythonJson;
    json["SW_FontSize"]    = SW_FontSize;
    json["SW_FontFamily"]  = SW_FontFamily;
    json["SW_FontWeight"]  = SW_FontWeight;
    json["SW_Italic"]      = SW_Italic;
    json["TabInSpaces"]    = TabInSpaces;

    json["TrackVisAttributes"] = TrackVisAttributes;

    // Web server
    //js["DefaultWebSocketPort"] = DefaultWebSocketPort;
    //js["DefaultWebSocketIP"] = DefaultWebSocketIP;

    // Root server
    {
        QJsonObject js;
            ARootHttpServer::getInstance().writeToJson(js);
        json["RootServer"] = js;
    }

    // Workload
    {
        QJsonObject js;
            AFarmHub::getConstInstance().writeToJson(js);
        json["Workload"] = js;
    }

    QJsonObject mainjson;
        mainjson["ANTS3config"] = json;
    jstools::saveJsonToFile(mainjson, ConfigDir + '/' + ConfigFileName);
}

void A3Global::loadConfig()
{
    QJsonObject mainjson;
    const QString fileName = ConfigDir + '/' + ConfigFileName;
    bool ok = jstools::loadJsonFromFile(mainjson, fileName);
    if (!ok)
    {
        qDebug() << "Could not open global config file:" << fileName;
        return;
    }

    QJsonObject json;
    ok = jstools::parseJson(mainjson, "ANTS3config", json);
    if (!ok)
    {
        qDebug() << "Bad format of the config file:" << fileName;
        return;
    }

    jstools::parseJson(json, "ExchangeDir", ExchangeDir);
    jstools::parseJson(json, "LastSaveDir", LastSaveDir);
    jstools::parseJson(json, "LastLoadDir", LastLoadDir);

    jstools::parseJson(json, "AutoCheckGeometry", AutoCheckGeometry);
    jstools::parseJson(json, "NumSegmentsTGeo", NumSegmentsTGeo);
    jstools::parseJson(json, "BinsX", BinsX);
    jstools::parseJson(json, "BinsY", BinsY);
    jstools::parseJson(json, "BinsZ", BinsZ);
    jstools::parseJson(json, "OpenImageExternalEditor", OpenImageExternalEditor);

    QJsonObject jsMa;
    jstools::parseJson(json, "DefaultDrawMargins", jsMa);
    DefaultDrawMargins.readFromJson(jsMa);

    jstools::parseJson(json, "JavaScriptJson", JavaScriptJson);
    jstools::parseJson(json, "PythonJson", PythonJson);
    jstools::parseJson(json, "SW_FontSize", SW_FontSize);
    jstools::parseJson(json, "SW_FontFamily", SW_FontFamily);
    jstools::parseJson(json, "SW_FontWeight", SW_FontWeight);
    jstools::parseJson(json, "SW_Italic", SW_Italic);
    jstools::parseJson(json, "TabInSpaces", TabInSpaces);

    jstools::parseJson(json, "TrackVisAttributes", TrackVisAttributes);

    // Root server
    {
        QJsonObject js;
            jstools::parseJson(json, "RootServer", js);
        ARootHttpServer::getInstance().readFromJson(js);
    }

    // Workload
    {
        QJsonObject js;
            jstools::parseJson(json, "Workload", js);
        AFarmHub::getInstance().readFromJson(js);
    }
}

QString A3Global::getQuickFileName(int index) const
{
    return QString("%0/QuickSave%1.json").arg(QuicksaveDir).arg(index);
}
