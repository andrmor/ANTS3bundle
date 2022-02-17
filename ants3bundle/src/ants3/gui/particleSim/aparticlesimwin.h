#ifndef APARTICLESIMWIN_H
#define APARTICLESIMWIN_H

#include <QMainWindow>

class AParticleSimSettings;
class AG4SimulationSettings;
class QListWidgetItem;
class AParticleGun;
class AParticleSimManager;
class QTreeWidgetItem;
class AParticleTrackingRecord;
class TObject;
class AMonitorHub;

namespace Ui {
class AParticleSimWin;
}

class AParticleSimWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit AParticleSimWin(QWidget *parent = nullptr);  // !!!*** add validators
    ~AParticleSimWin();

public slots:
    void updateGui();

private slots:
    void on_pbSimulate_clicked();

    void on_lePhysicsList_editingFinished();
    void on_cobRefPhysLists_activated(int index);
    void on_cbUseTSphys_clicked(bool checked);
    void on_pteCommands_textChanged();
    void on_pteSensitiveVolumes_textChanged();  // redo  !!!***
    void on_pbAddNewStepLimit_clicked(); // !!!*** possible override of step limit with overlapping volume name using wildcard *
    void on_lwStepLimits_itemDoubleClicked(QListWidgetItem *item);
    void on_pbRemoveStepLimit_clicked();

    void on_pbEditParticleSource_clicked();
    void on_pbAddSource_clicked();
    void on_pbCloneSource_clicked();
    void on_pbRemoveSource_clicked();

    void on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem * item);

    void on_pbGunTest_clicked();
    void on_pbGunShowSource_toggled(bool checked);
    void on_pbConfigureOutput_clicked();

    void on_cobParticleGenerationMode_activated(int index);

    void on_sbEvents_editingFinished();

    void on_pbChooseWorkingDirectory_clicked();

    // tracks
    void on_pbChooseFileTrackingData_clicked();
    void on_pbShowTracks_clicked();

    void on_cbGunAllowMultipleEvents_clicked(bool checked);
    void on_cobPartPerEvent_activated(int index);
    void on_ledGunAverageNumPartperEvent_editingFinished();
    void on_cbGunAllowMultipleEvents_toggled(bool checked);
    void on_cobPartPerEvent_currentIndexChanged(int index);

    // event view
    void on_pbShowEventTree_clicked();
    void on_pbEventView_clicked();
    void on_sbEVexpansionLevel_valueChanged(int);
    void on_cbEVhideTrans_clicked();
    void on_cbEVhideTransPrim_clicked();

    // statistics
    void on_pbPTHistRequest_clicked();
    void on_cbPTHistOnlyPrim_clicked(bool checked);
    void on_cbPTHistOnlySec_clicked(bool checked);
    void on_cobPTHistVolRequestWhat_currentIndexChanged(int index);
    void on_twPTHistType_currentChanged(int index);
    void on_cbPTHistBordVs_toggled(bool);
    void on_cbPTHistBordAndVs_toggled(bool);
    void on_cbPTHistBordAsStat_toggled(bool);

    void on_pbGenerateFromFile_Help_clicked();
    void on_pbGenerateFromFile_Change_clicked();
    void on_leGenerateFromFile_FileName_editingFinished();
    void on_pbFilePreview_clicked();
    void on_pbAnalyzeFile_clicked();

    void on_cobMonitor_activated(int index);
    void on_sbMonitorIndex_editingFinished();
    void on_pbNextMonitor_clicked();
    void on_pbChooseMonitorsFile_clicked();
    void on_pbLoadMonitorsData_clicked();

    void on_pbMonitorShowAngle_clicked();
    void on_pbMonitorShowXY_clicked();
    void on_pbMonitorShowTime_clicked();
    void on_pbMonitorShowEnergy_clicked();
    void on_pbShowMonitorHitDistribution_clicked();
    void on_pbShowMonitorTimeOverall_clicked();

    void on_pbLoadAllResults_clicked();

private slots:
    void onMaterialsChanged();

    void on_sbShowEvent_editingFinished();

    void on_pbPreviousEvent_clicked();

    void on_pbNextEvent_clicked();

signals:
    void requestShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed);
    void requestShowTracks();
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);

private:
    AParticleSimSettings  & SimSet;
    AG4SimulationSettings & G4SimSet;
    AParticleSimManager   & SimManager;
    AMonitorHub           & MonitorHub;

    Ui::AParticleSimWin *ui;

    std::vector<bool> ExpandedItems;

    int    binsEnergy = 100;
    double fromEnergy = 0;
    double toEnergy = 0;
    int    selectedModeForEnergyDepo = 0;
    int    selectedModeForProcess = 0;
    int    binsDistance = 100;
    double fromDistance = 0;
    double toDistance = 0;
    int    binsTime = 100;
    double fromTime = 0;
    double toTime = 0;
    int    binsB1 = 100;
    double fromB1 = 0;
    double toB1 = 0;
    int    binsB2 = 100;
    double fromB2 = 0;
    double toB2 = 0;

    bool   bGuiUpdateInProgress = false;

    void updateG4Gui();
    void updateSimGui();
    void updateSourceList();
    void drawSource(int iSource);  // !!!***
    void testParticleGun(AParticleGun * Gun, int numParticles); // !!!***

    void clearResultsGui();

    void disableGui(bool flag);

    void updateMonitorGui();

    //event viewer
    void fillEvTabViewRecord(QTreeWidgetItem * item, const AParticleTrackingRecord * pr, int ExpansionLevel) const;
    void EV_showTree();
    void doProcessExpandedStatus(QTreeWidgetItem *item, int &counter, bool bStore);
    void updatePTHistoryBinControl();
    void updateFileParticleGeneratorGui();
    void showStepLimitDialog(const QString &volName, double limit);
};

#endif // APARTICLESIMWIN_H
