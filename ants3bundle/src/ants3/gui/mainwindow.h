#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class A3Config;
class A3ScriptManager;
class A3ScriptRes;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(A3ScriptManager & SM, A3ScriptRes & ScrRes, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onProgressReceived(double progress);

private:
    A3Config        & Config;
    A3ScriptManager & ScriptManager;
    A3ScriptRes     & ScrRes;

    Ui::MainWindow  * ui = nullptr;

private slots:
    void onScriptEvaluationFinished(bool bSuccess);
    void onParticleSimulationFinsihed();

    void on_pbEvaluateScript_clicked();
    void on_pbSimulate_clicked();
    void on_leFrom_editingFinished();
    void on_leTo_editingFinished();

private:
    void disableInterface(bool flag);
};

#endif // MAINWINDOW_H
