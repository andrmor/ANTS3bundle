#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class A3Config;
class A3ScriptManager;
class A3GeoConWin;
class AGeometryWindow;
class A3MatWin;
class A3PhotSimWin;
class AInterfaceRuleWin;
class GraphWindowClass;
class ARemoteWindow;
class AParticleSimWin;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(A3ScriptManager & SM);
    ~MainWindow();

public slots:
    void onProgressReceived(double progress);

private:
    A3Config          & Config;
    A3ScriptManager   & ScriptManager;

    Ui::MainWindow    * ui = nullptr;

    A3GeoConWin       * GeoConWin  = nullptr;
    AGeometryWindow   * GeoWin     = nullptr;
    A3MatWin          * MatWin     = nullptr;
    A3PhotSimWin      * PhotSimWin = nullptr;
    AInterfaceRuleWin * RuleWin    = nullptr;
    GraphWindowClass  * GraphWin   = nullptr;
    ARemoteWindow     * FarmWin    = nullptr;
    AParticleSimWin   * PartSimWin = nullptr;

private slots:
    void onScriptEvaluationFinished(bool bSuccess);
    void onDemoFinsihed();
    void onRebuildGeometryRequested();

    void on_pbEvaluateScript_clicked();
    void on_pbSimulate_clicked();
    void on_leFrom_editingFinished();
    void on_leTo_editingFinished();
    void on_pbAbort_clicked();
    void on_pbGeometry_clicked();
    void on_pbGeoWin_clicked();
    void on_pbMaterials_clicked();
    void on_actionSave_configuration_triggered();
    void on_actionLoad_configuration_triggered();
    void on_pbPhotSim_clicked();

    void on_pbInterfaceRules_clicked();

    void on_pbGraphWin_clicked();

    void on_pbFarm_clicked();

    void on_pbParticleSim_clicked();

private:
    void disableInterface(bool flag);
};

#endif // MAINWINDOW_H
