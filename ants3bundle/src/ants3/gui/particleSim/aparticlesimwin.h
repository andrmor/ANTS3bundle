#ifndef APARTICLESIMWIN_H
#define APARTICLESIMWIN_H

#include <QMainWindow>

class AParticleSimSettings;
class AG4SimulationSettings;
class QListWidgetItem;
class AParticleGun;

namespace Ui {
class AParticleSimWin;
}

class AParticleSimWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit AParticleSimWin(QWidget *parent = nullptr);
    ~AParticleSimWin();

public slots:
    void updateGui();

private slots:
    void on_lePhysicsList_editingFinished();
    void on_cobRefPhysLists_activated(int index);
    void on_cbUseTSphys_clicked(bool checked);
    void on_pteCommands_textChanged();
    void on_pteSensitiveVolumes_textChanged();  // redo  !!!***
    void on_pteStepLimits_textChanged();

    void on_pbEditParticleSource_clicked();
    void on_pbAddSource_clicked();
    void on_pbCloneSource_clicked();
    void on_pbRemoveSource_clicked();

    void on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem * item);

    void on_pbGunTest_clicked();
    void on_pbGunShowSource_toggled(bool checked);
    void on_pbConfigureOutput_clicked();

    void on_pbSimulate_clicked();

    void on_cobParticleGenerationMode_activated(int index);

    void on_sbEvents_editingFinished();

signals:
    void requestShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed);
    void requestShowTracks();

private:
    AParticleSimSettings  & SimSet;
    AG4SimulationSettings & G4SimSet;

    Ui::AParticleSimWin *ui;

    void updateG4Gui();
    void updateSimGui();
    void updateSourceList();
    void drawSource(int iSource);  // !!!***
    void testParticleGun(AParticleGun * Gun, int numParticles); // !!!***
};

#endif // APARTICLESIMWIN_H