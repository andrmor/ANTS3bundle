#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class AConfig;
class A3GeoConWin;
class AGeometryWindow;
class A3MatWin;
class A3PhotSimWin;
class AInterfaceRuleWin;
class GraphWindowClass;
class ARemoteWindow;
class AParticleSimWin;
class AScriptWindow;
class ADemoWindow; // tmp

class MainWindow : public QMainWindow
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
    A3PhotSimWin      * PhotSimWin = nullptr;
    AInterfaceRuleWin * RuleWin    = nullptr;
    GraphWindowClass  * GraphWin   = nullptr;
    ARemoteWindow     * FarmWin    = nullptr;
    AParticleSimWin   * PartSimWin = nullptr;
    AScriptWindow     * JScriptWin = nullptr;
    ADemoWindow       * DemoWin    = nullptr;

private slots:
    void onRebuildGeometryRequested();

    void on_actionSave_configuration_triggered();
    void on_actionLoad_configuration_triggered();

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
};

#endif // MAINWINDOW_H
