#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class MainWindow;
}
class AConfig;
class A3GeoConWin;
class AGeometryWindow;
class A3MatWin;
class ASensorWindow;
class A3PhotSimWin;
class AInterfaceRuleWin;
class GraphWindowClass;
class ARemoteWindow;
class AParticleSimWin;
class AScriptWindow;
class AGlobSetWindow;
class ADemoWindow; // tmp
class QTimer;

class MainWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

    void updateGui();

private:
    AConfig          & Config;

    Ui::MainWindow    * ui = nullptr;

    A3GeoConWin       * GeoConWin  = nullptr;
    AGeometryWindow   * GeoWin     = nullptr;
    A3MatWin          * MatWin     = nullptr;
    ASensorWindow     * SensWin    = nullptr;
    A3PhotSimWin      * PhotSimWin = nullptr;
    AInterfaceRuleWin * RuleWin    = nullptr;
    GraphWindowClass  * GraphWin   = nullptr;
    ARemoteWindow     * FarmWin    = nullptr;
    AParticleSimWin   * PartSimWin = nullptr;
    AScriptWindow     * JScriptWin = nullptr;
    AGlobSetWindow    * GlobSetWin = nullptr;
    ADemoWindow       * DemoWin    = nullptr;

    QTimer * RootUpdateTimer = nullptr;

private slots:
    void onRebuildGeometryRequested(); // !!!*** refactor?
    void updateAllGuiFromConfig();
    void onRequestSaveGuiSettings();

    // Main menu
    void on_actionSave_configuration_triggered();
    void on_actionLoad_configuration_triggered();
    void on_actionLoad_last_config_triggered();
    void on_actionQuickSave_slot_1_triggered();
    void on_actionQuickSave_slot_2_triggered();
    void on_actionQuickSave_slot_3_triggered();
    void on_actionQuickLoad_slot_1_triggered();
    void on_actionQuickLoad_slot_2_triggered();
    void on_actionQuickLoad_slot_3_triggered();
    void on_actionExit_triggered();

    void on_pbGeometry_clicked();
    void on_pbGeoWin_clicked();
    void on_pbMaterials_clicked();
    void on_pbPhotSim_clicked();
    void on_pbInterfaceRules_clicked();
    void on_pbGraphWin_clicked();
    void on_pbFarm_clicked();
    void on_pbParticleSim_clicked();
    void on_pbJavaScript_clicked();

    void on_pbDemo_clicked();
    void on_pbLoadConfig_clicked();
    void on_pbSaveConfig_clicked();
    void on_leConfigName_editingFinished();
    void on_pteConfigDescription_textChanged();
    void on_pushButton_clicked();

    void rootTimerTimeout();

    void on_pbGlobSet_clicked();

protected:
    void closeEvent(QCloseEvent * event);

private:
    void saveWindowGeometries();
    void loadWindowGeometries();
};

#endif // MAINWINDOW_H
