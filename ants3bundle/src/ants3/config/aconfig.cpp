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

    clearUndo();

    QString res = readFromJson(json, bUpdateGui);
    createUndo();
    return res;
}

QString AConfig::save(const QString & fileName)
{
    clearTemporaryOutputDirs();
    updateJSONfromConfig();
    replaceEmptyOutputDirsWithTemporary();

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
    QString & photonSimOutputDir = APhotonSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (photonSimOutputDir.isEmpty()) photonSimOutputDir = A3Global::getConstInstance().TmpOutputDir;

    std::string & particleSimOutputDir = AParticleSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (particleSimOutputDir.empty()) particleSimOutputDir = A3Global::getConstInstance().TmpOutputDir.toLatin1().data();
}

void AConfig::clearTemporaryOutputDirs()
{
    QString & photonSimOutputDir = APhotonSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (photonSimOutputDir == A3Global::getConstInstance().TmpOutputDir) photonSimOutputDir.clear();

    std::string & particleSimOutputDir = AParticleSimHub::getInstance().Settings.RunSet.OutputDirectory;
    if (particleSimOutputDir == std::string(A3Global::getConstInstance().TmpOutputDir.toLatin1().data())) particleSimOutputDir.clear();
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

 // ---- undo related ----

void AConfig::createUndo()
{
    if (A3Global::getConstInstance().UndoMaxDepth == 0)
    {
        qDebug() << "-> Undo/redo system is desabled, nothing to do";
        return;
    }

    updateJSONfromConfig();

    UndoConfigs.insert(UndoConfigs.begin(), JSON);
    qDebug() << "> Undo created, UndoConfig length:" << UndoConfigs.size();
    if (UndoConfigs.size() > A3Global::getConstInstance().UndoMaxDepth)
    {
        qDebug() << "> UndoConfigs max reached, resizing to UndoMaxDepth of " << A3Global::getConstInstance().UndoMaxDepth;
        UndoConfigs.resize(A3Global::getConstInstance().UndoMaxDepth);
    }

    UndoCurrentPosition = 0;
}

bool AConfig::isUndoAvailable() const
{
    if (UndoCurrentPosition < 0) return false;
    return (UndoCurrentPosition < UndoConfigs.size() - 1);
}

bool AConfig::isRedoAvailable() const
{
    if (UndoCurrentPosition < 0) return false;
    return (UndoCurrentPosition > 0);
}

void AConfig::clearUndo()
{
    UndoCurrentPosition = -1;
    UndoConfigs.clear();
}

QString AConfig::doUndo()
{
    if (UndoCurrentPosition < 0) return "No undo configurations are available";
    if (UndoCurrentPosition >= UndoConfigs.size()-1) return "Cannot undo: already at the last remembered config";

    qDebug() << "-->Doing undo...";
    UndoCurrentPosition++;
    qDebug() << "> New Undo current position:" << UndoCurrentPosition;
    readFromJson(UndoConfigs[UndoCurrentPosition], true);
    qDebug() << "> JSON loaded";
    return "";
}

QString AConfig::doRedo()
{
    if (UndoCurrentPosition < 0) return "No undo/redo configurations are available";
    if (UndoCurrentPosition == 0) return "Cannot redo: Already at the last available configuration";

    qDebug() << "-->Doing undo...";
    UndoCurrentPosition--;
    qDebug() << "> New Undo current position:" << UndoCurrentPosition;
    readFromJson(UndoConfigs[UndoCurrentPosition], true);
    qDebug() << "> JSON loaded";
    return "";
}

void AConfig::updateUndoMaxDepth(int newDepth)
{
    if (newDepth == A3Global::getConstInstance().UndoMaxDepth) return;

    if (newDepth == 0)
    {
        UndoConfigs.clear();
        UndoCurrentPosition = -1;
    }
    else if (newDepth > A3Global::getConstInstance().UndoMaxDepth)
    {
        // nothing special to do
    }
    else // newDepth < UndoMaxDepth,  but not zero
    {
        if (UndoConfigs.size() > newDepth) UndoConfigs.resize(newDepth);
        if (UndoCurrentPosition > newDepth-1) UndoCurrentPosition = newDepth-1;
    }
    A3Global::getInstance().UndoMaxDepth = newDepth;
}

// ----
