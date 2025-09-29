#ifndef ACONFIG_H
#define ACONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>

class QJsonObject;

class AConfig final : public QObject
{
    Q_OBJECT

public:
    static AConfig & getInstance();
    static const AConfig & getConstInstance();

private:
    AConfig();
    ~AConfig(){}

    AConfig(const AConfig&)            = delete;
    AConfig(AConfig&&)                 = delete;
    AConfig& operator=(const AConfig&) = delete;
    AConfig& operator=(AConfig&&)      = delete;

public:
    // Geometry config is handled by AGeometryHub    singleton
    // Material config is handled by AMaterialHub    singleton
    // Optical interface rules    -> AOpticalRileHub singleton
    // SensorHub   config is handled by ASensorHub   singleton

    // ParticleSim config is handled by AParticleSimHub singleton
    // PhotonSim   config is handled by APhotonSimHub   singleton

    QJsonObject JSON;

    QString     ConfigName = "--";
    QString     ConfigDescription = "Description is empty";

    // Temporary (demo)
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    std::vector<QJsonObject> UndoConfigs;
    int                      UndoCurrentPosition = -1;

    void    updateJSONfromConfig();
    QString updateConfigFromJSON(bool updateGui);

    QString load(const QString & fileName, bool bUpdateGui);
    QString save(const QString & fileName);

    void    writeToJson(QJsonObject & json, bool addRuntimeExport) const;
    QString readFromJson(const QJsonObject & json, bool updateGui);

    // undo / redo
    void createUndo();
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    void clearUndo();
    QString doUndo();
    QString doRedo();
    void updateUndoMaxDepth(int newDepth);

    void replaceEmptyOutputDirsWithTemporary();
    void clearTemporaryOutputDirs();

private:
    QString tryReadFromJson(const QJsonObject & json); // resets error hub on call

signals:
    void configLoaded();  // signal to GUI
    void requestSaveGuiSettings();

};

#endif // ACONFIG_H
