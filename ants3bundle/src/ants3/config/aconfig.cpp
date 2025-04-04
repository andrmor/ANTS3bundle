#include "aconfig.h"
#include "ajsontools.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "aphotonsimhub.h"
#include "ainterfacerulehub.h"
#include "asensorhub.h"
#include "aparticlesimhub.h"
#include "aerrorhub.h"
#include "a3global.h"
#include "aphotonfunctionalhub.h"
#include "aparticleanalyzerhub.h"

#include <QDebug>
#include <QFile>

AConfig & AConfig::getInstance()
{
    static AConfig instance;
    return instance;
}

const AConfig &AConfig::getConstInstance()
{
    return AConfig::getInstance();
}

AConfig::AConfig()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

void AConfig::updateJSONfromConfig()
{
    // if gui is present, save gui settings
    emit requestSaveGuiSettings();

    writeToJson(JSON, false);
}

QString AConfig::updateConfigFromJSON(bool updateGui)
{
    return readFromJson(JSON, updateGui);
}

QString AConfig::load(const QString & fileName, bool bUpdateGui)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok) return "Cannot open config file: " + fileName;

    invalidateUndo();
    invalidateRedo();

    return readFromJson(json, bUpdateGui);
}

QString AConfig::save(const QString & fileName)
{
    updateJSONfromConfig();

    bool ok = jstools::saveJsonToFile(JSON, fileName);
    if (ok) return "";
    else    return "Cannot open file to save config:\n" + fileName;
}

void AConfig::writeToJson(QJsonObject & json, bool addRuntimeExport) const
{
    json["ConfigName"]        = ConfigName;
    json["ConfigDescription"] = ConfigDescription;

    AMaterialHub::getInstance().writeToJson(json);
    AGeometryHub::getInstance().writeToJson(json);
    AInterfaceRuleHub::getInstance().writeToJson(json);
    ASensorHub::getConstInstance().writeToJson(json);
    APhotonFunctionalHub::getConstInstance().writeToJson(json);

    AParticleSimHub::getConstInstance().writeToJson(json, addRuntimeExport);
    APhotonSimHub::getConstInstance().writeToJson(json, addRuntimeExport);
}

QString AConfig::readFromJson(const QJsonObject & json, bool updateGui)
{
    QString err = tryReadFromJson(json);
    if (err.isEmpty())
    {
        JSON = json;
        replaceEmptyOutputDirsWithTemporary();
        if (updateGui) emit configLoaded();
        return "";
    }
    else
    {
        readFromJson(JSON, updateGui);
        return err;
    }
}

void AConfig::replaceEmptyOutputDirsWithTemporary()
{
    // examples have output dir empty, replace them with a temporary one
    QString & photonSimOutputDir = APhotonSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (photonSimOutputDir.isEmpty()) photonSimOutputDir = A3Global::getConstInstance().TmpOutputDir;
    std::string & particleSimOutputDir = AParticleSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (particleSimOutputDir.empty()) particleSimOutputDir = A3Global::getConstInstance().TmpOutputDir.toLatin1().data();
}

QString AConfig::tryReadFromJson(const QJsonObject & json)
{
    AErrorHub::clear();

    bool ok = jstools::parseJson(json, "ConfigName",        ConfigName);
    if (!ok) return "Not a configuration file!";
    ok      = jstools::parseJson(json, "ConfigDescription", ConfigDescription);
    if (!ok) return "Not a configuration file!";

    QString Error;

    Error = AMaterialHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;
    if (AErrorHub::isError()) return AErrorHub::getQError();

    Error = ASensorHub::getInstance().readFromJson(json); // Sensors should be read before geometry
    if (!Error.isEmpty()) return Error;
    if (AErrorHub::isError()) return AErrorHub::getQError();

    Error = AGeometryHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;
    if (AErrorHub::isError()) return AErrorHub::getQError();

    Error = AInterfaceRuleHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;
    if (AErrorHub::isError()) return AErrorHub::getQError();

    Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) return Error;
    if (AErrorHub::isError()) return AErrorHub::getQError();

    AParticleSimHub::getInstance().readFromJson(json);
    if (AErrorHub::isError()) return AErrorHub::getQError();

    APhotonFunctionalHub::getInstance().readFromJson(json);
    if (AErrorHub::isError()) return AErrorHub::getQError();

    AParticleAnalyzerHub::getInstance().clear(); // only need to clear loaded data, the config is in the geometry tree

    return "";
}

void AConfig::createUndo()
{
    updateJSONfromConfig();

    A3Global & GS = A3Global::getInstance();
    QString fn = GS.QuicksaveDir + "/undo.json";
    jstools::saveJsonToFile(JSON, fn);
}

bool AConfig::isUndoAvailable() const
{
    A3Global & GS = A3Global::getInstance();
    QFile qf(GS.QuicksaveDir + "/undo.json");
    return qf.exists();
}

bool AConfig::isRedoAvailable() const
{
    A3Global & GS = A3Global::getInstance();
    QFile qf(GS.QuicksaveDir + "/redo.json");
    return qf.exists();
}

void AConfig::invalidateUndo()
{
    A3Global & GS = A3Global::getInstance();
    QFile qf(GS.QuicksaveDir + "/undo.json");
    qf.remove();
}

void AConfig::invalidateRedo()
{
    A3Global & GS = A3Global::getInstance();
    QFile qf(GS.QuicksaveDir + "/redo.json");
    qf.remove();
}

QString AConfig::doUndo()
{
    A3Global & GS = A3Global::getInstance();
    save(GS.QuicksaveDir + "/redoTmp.json");
    QString ErrorString = load(GS.QuicksaveDir + "/undo.json", true);
    QFile qf(GS.QuicksaveDir + "/redoTmp.json");
    qf.rename(GS.QuicksaveDir + "/redo.json");
    return ErrorString;
}

QString AConfig::doRedo()
{
    A3Global & GS = A3Global::getInstance();
    QString ErrorString = load(GS.QuicksaveDir + "/redo.json", true);
    return ErrorString;
}
