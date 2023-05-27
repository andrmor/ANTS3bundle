#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class MainWindow;
}
class AConfig;
class AGeoTreeWin;
class AGeometryWindow;
class AMatWin;
class ASensorWindow;
class APhotSimWin;
class AInterfaceRuleWin;
class GraphWindowClass;
class ARemoteWindow;
class AParticleSimWin;
class AScriptWindow;
class AGlobSetWindow;
class ADemoWindow; // tmp
class QTimer;
class A3Global;

class MainWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

    void updateGui();

private:
    AConfig           & Config;
    A3Global          & GlobSet;

    Ui::MainWindow    * ui = nullptr;

    AGeoTreeWin       * GeoTreeWin  = nullptr;
    AGeometryWindow   * GeoWin     = nullptr;
    AMatWin           * MatWin     = nullptr;
    ASensorWindow     * SensWin    = nullptr;
    APhotSimWin       * PhotSimWin = nullptr;
    AInterfaceRuleWin * RuleWin    = nullptr;
    GraphWindowClass  * GraphWin   = nullptr;
    ARemoteWindow     * FarmWin    = nullptr;
    AParticleSimWin   * PartSimWin = nullptr;
    AScriptWindow     * JScriptWin = nullptr;
#ifdef ANTS3_PYTHON
    AScriptWindow     * PythonWin = nullptr;
#endif
    AGlobSetWindow    * GlobSetWin = nullptr;
    ADemoWindow       * DemoWin    = nullptr;

    QTimer * RootUpdateTimer = nullptr;

private slots:
    void onRebuildGeometryRequested(); // !!!*** refactor?
    void updateAllGuiFromConfig();
    void onRequestSaveGuiSettings();
    void onRequestChangeGeoViewer(bool useJSRoot);

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
    void on_actionClose_ants3_triggered();

    // Window buttons
    void on_pbGeometry_clicked();
    void on_pbGeometry_customContextMenuRequested(const QPoint & pos);
    void on_pbGeoWin_clicked();
    void on_pbGeoWin_customContextMenuRequested(const QPoint & pos);
    void on_pbMaterials_clicked();
    void on_pbMaterials_customContextMenuRequested(const QPoint & pos);
    void on_pbPhotSim_clicked();
    void on_pbPhotSim_customContextMenuRequested(const QPoint & pos);
    void on_pbInterfaceRules_clicked();
    void on_pbInterfaceRules_customContextMenuRequested(const QPoint & pos);
    void on_pbSensors_clicked();
    void on_pbSensors_customContextMenuRequested(const QPoint & pos);
    void on_pbGraphWin_clicked();
    void on_pbGraphWin_customContextMenuRequested(const QPoint & pos);
    void on_pbFarm_clicked();
    void on_pbFarm_customContextMenuRequested(const QPoint & pos);
    void on_pbParticleSim_clicked();
    void on_pbParticleSim_customContextMenuRequested(const QPoint & pos);
    void on_pbGlobSet_clicked();
    void on_pbGlobSet_customContextMenuRequested(const QPoint & pos);
    void on_pbJavaScript_clicked();  // !!!*** need update? note: config json could have changed!
    void on_pbJavaScript_customContextMenuRequested(const QPoint & pos);
    void on_pbPython_clicked();  // !!!*** need update? note: config json could have changed!
    void on_pbPython_customContextMenuRequested(const QPoint & pos);
    void on_pbDemo_clicked();
    void on_pbDemo_customContextMenuRequested(const QPoint & pos);

    // Other buttons
    void on_pbLoadConfig_clicked();
    void on_pbSaveConfig_clicked();
    void on_pbNew_clicked();

    void on_leConfigName_editingFinished();
    void on_pteConfigDescription_textChanged();

    void on_actionQuickLoad_slot_1_hovered();
    void on_actionQuickLoad_slot_2_hovered();
    void on_actionQuickLoad_slot_3_hovered();
    void on_actionLoad_last_config_hovered();

    void rootTimerTimeout();

protected:
    void closeEvent(QCloseEvent * event);

private:
    void saveWindowGeometries();
    void loadWindowGeometries();
    QString getQuickLoadMessage(int index);
    void changeGeoViewer(bool useJSRoot);
    void connectSignalSlotsForGeoWin();
};

#endif // MAINWINDOW_H
