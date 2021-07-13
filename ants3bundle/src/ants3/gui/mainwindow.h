#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class A3Config;
class A3ScriptManager;
class A3ScriptRes;
class A3GeoConWin;
class GeometryWindowClass;
class MaterialInspectorWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(A3ScriptManager & SM, A3ScriptRes & ScrRes);
    ~MainWindow();

public slots:
    void onProgressReceived(double progress);

private:
    A3Config        & Config;
    A3ScriptManager & ScriptManager;
    A3ScriptRes     & ScrRes;

    Ui::MainWindow  * ui = nullptr;

    A3GeoConWin             * GeoConWin = nullptr;
    GeometryWindowClass     * GeoWin    = nullptr;
    MaterialInspectorWindow * MatWin    = nullptr;

private slots:
    void onScriptEvaluationFinished(bool bSuccess);
    void onParticleSimulationFinsihed();
    void onRebuildGeometryRequested();

    void on_pbEvaluateScript_clicked();
    void on_pbSimulate_clicked();
    void on_leFrom_editingFinished();
    void on_leTo_editingFinished();

    void on_pbAbort_clicked();

    void on_pbGeometry_clicked();

    void on_pbGeoWin_clicked();

    void on_pbMaterials_clicked();

private:
    void disableInterface(bool flag);
};

#endif // MAINWINDOW_H
