#ifndef APARTICLESIMWIN_H
#define APARTICLESIMWIN_H

#include <QMainWindow>

class AG4SimulationSettings;

namespace Ui {
class AParticleSimWin;
}

class AParticleSimWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit AParticleSimWin(QWidget *parent = nullptr);
    ~AParticleSimWin();

    void updateGui();

private slots:
    void on_lePhysicsList_editingFinished();
    void on_cobRefPhysLists_activated(int index);
    void on_cbUseTSphys_clicked(bool checked);
    void on_pteCommands_textChanged();
    void on_cbBinaryOutput_clicked(bool checked);
    void on_sbPrecision_editingFinished();
    void on_pteSensitiveVolumes_textChanged();  // redo  !!!***

    void on_pteStepLimits_textChanged();

private:
    AG4SimulationSettings & G4SimSet;

    Ui::AParticleSimWin *ui;
};

#endif // APARTICLESIMWIN_H
