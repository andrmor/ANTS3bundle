#ifndef APARTICLESIMWIN_H
#define APARTICLESIMWIN_H

#include "aguiwindow.h"
#include "avector.h"

#include <map>

class AParticleSimSettings;
class AG4SimulationSettings;
class QListWidgetItem;
class AParticleGun;
class AParticleSimManager;
class QTreeWidgetItem;
class AParticleTrackingRecord;
class TObject;
class AMonitorHub;
class ACalorimeterHub;
class TH1D;
class AParticleRecord;
class AParticleSourceRecordBase;
class ATrackingHistoryCrawler;
class AFindRecordSelector;
class AEventTrackingRecord;
class AParticleSourceDialog;        // !!!**** add layer of abstraction?
class AParticleSourceDialog_EcoMug; // !!!**** add layer of abstraction?

namespace Ui {
class AParticleSimWin;
}

class AParticleSimWin : public AGuiWindow
{
    Q_OBJECT

public:
    explicit AParticleSimWin(QWidget * parent = nullptr);
    ~AParticleSimWin();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

public slots:
    void updateGui();
    void updateResultsGui();
    void onBusyStatusChange(bool busy);
    void onMaterialsChanged();
    void onNewConfigStartedInGui();

private slots:
    // auto-updates
    void onRequestShowSource();

    void on_pbSimulate_clicked();

    void on_lePhysicsList_editingFinished();
    //void on_cbUseTSphys_clicked(bool checked);
    void on_cobThermalNeutronModel_activated(int index);
    void on_pteCommands_textChanged();
    void on_pteSensitiveVolumes_textChanged();  // redo  !!!***
    void on_pbAddNewStepLimit_clicked(); // !!!*** possible override of step limit with overlapping volume name using wildcard *
    void on_lwStepLimits_itemDoubleClicked(QListWidgetItem *item);
    void on_pbRemoveStepLimit_clicked();

    void on_pbEditParticleSource_clicked();
    void on_pbAddSource_clicked(); // !!!****
    void on_pbCloneSource_clicked();
    void on_pbRemoveSource_clicked();

    void on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem * item);

    void on_pbGunTest_clicked();
    void on_pbGunShowSource_toggled(bool checked);
    void on_pbConfigureOutput_clicked();

    void on_cobParticleGenerationMode_activated(int index);
    void on_ledEvents_editingFinished();
    void on_pbChooseWorkingDirectory_clicked();

    // tracks
    void on_pbChooseFileTrackingData_clicked();
    void on_pbShowTracks_clicked();
    void on_pbConfigureTrackStyles_clicked();
    void on_cbLimitToParticleTracks_toggled(bool checked);
    void on_cbExcludeParticleTracks_toggled(bool checked);

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
    void on_cbEVhideStepLim_clicked();
    void on_cbEVhideStepLimPrim_clicked();
    void on_cbEVhideIoni_clicked();
    void on_cbEVhideIoniPrim_clicked();
    void on_sbShowEvent_editingFinished();
    void on_pbPreviousEvent_clicked();
    void on_pbNextEvent_clicked();

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

    void on_trwEventView_customContextMenuRequested(const QPoint &pos);

    void onParticleSourceAccepted();

signals:
    void requestShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed);
    void requestShowTracks(bool activateWindow = false);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestAddToBasket(const QString & name);
    void requestShowPosition(double * pos, bool keepTracks);
    void requestCenterView(double * pos);
    void requestPlotELDD(std::vector<std::pair<double,double>> dist);
    void requestClearMarkers(int selector);
    void requestAddMarker(const double *);
    void requestShowGeoObjectDelegate(QString ObjName, bool bShow);
    void requestConfigureExchangeDir();
    void requestBusyStatus(bool flag);

private:
    AParticleSimSettings  & SimSet;
    AG4SimulationSettings & G4SimSet;
    AParticleSimManager   & SimManager;
    AMonitorHub           & MonitorHub;
    ACalorimeterHub       & CalHub;

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

    QString LastDir_Working;
    QString LastFile_Tracking;
    QString LastFile_Monitors;
    QString LastFile_Calorimeters;
    QString LastFile_Analyzers;

    TH1D * histEnergy = nullptr;
    TH1D * histAngle = nullptr;
    bool   CollectAngle = false;
    AVector3 SourceStatDirection = {0,0,1.0};
    TH1D * histTime = nullptr;
    std::map<std::string, int> SeenParticles;

    bool IgnoreWorldSizeWarning = false;

    bool   bFindEventAbortRequested = false;

    AEventTrackingRecord * CurrentEventRecord = nullptr;

    AParticleSourceDialog * ParticleSourceDialog = nullptr;
    AParticleSourceDialog_EcoMug * ParticleSourceDialog_EcoMug = nullptr;

    void updateG4Gui();
    void updateSimGui();
    void updateSourceList(); // !!!****
    void updateGeneralControlInResults();

    //clear
    void clearResults();
    void clearResultsTracking();
    void clearResultsMonitors();
    void clearResultsCalorimeters();

    // clear results gui
    void clearResultsGui();
    void clearResultsGuiTrackig();
    void clearResultsGuiMonitors();
    void clearResultsGuiCalorimeters();

    void disableGui(bool flag);

    void updateMonitorGui();
    void updateCalorimeterGui();
    void updateShowCalorimeterGui();

    //event viewer
    void fillEvTabViewRecord(QTreeWidgetItem * item, const AParticleTrackingRecord * pr, int ExpansionLevel) const;
    void EV_showTree();  // !!!***
    void doProcessExpandedStatus(QTreeWidgetItem *item, int &counter, bool bStore);
    void updatePTHistoryBinControl();
    void updateFileParticleGeneratorGui();
    void showStepLimitDialog(const QString &volName, double limit);
    int  findEventWithFilters(int currentEv, bool bUp);

    void addStatistics(const AParticleRecord & p);
    void configureAngleStat(AParticleGun * gun);
    void checkWorldSize(AParticleSourceRecordBase * ps);
    bool isTrackingDataFileExists();

    void findInBulk(ATrackingHistoryCrawler & crawler, AFindRecordSelector & options, int numThreads, int numEventsPerThread);
    void findInTransitions(ATrackingHistoryCrawler & crawler, AFindRecordSelector & options, int numThreads, int numEventsPerThread);
    void updateCaloRange();
    void updateRangeWarning();
    void updateAnalyzerGui();
    void updateAnalyzerDataGui(bool suppressMessages);
    void onUserChangedAnalyzerIndex();

private slots:
    void testParticleGun(AParticleGun * gun, int numParticles, bool fillStatistics);
    void onProgressReceived(double progress);
    void on_cbPTHistVolVsTime_toggled(bool checked);
    void on_pbUpdateIcon_clicked();
    void on_pbChooseCalorimetersFile_clicked();
    void on_pbLoadCalorimetersData_clicked();
    void on_pbNextCalorimeter_clicked();
    void on_cobCalorimeter_activated(int index);
    void on_sbCalorimeterIndex_editingFinished();
    void on_pbShowCalorimeterSettings_clicked();
    void on_pbShowMonitorProperties_clicked();
    void on_leWorkingDirectory_editingFinished();
    void on_leTrackingDataFile_editingFinished();
    void on_leMonitorsFileName_editingFinished();
    void on_leCalorimetersFileName_editingFinished();

    void on_pbSaveParticleSource_clicked();
    void on_pbLoadParticleSource_clicked();

    void on_pbAbort_clicked();
    void on_ledEventsPerThread_editingFinished();
    void on_cobCaloShowType_currentIndexChanged(int index);
    void on_cobCaloAxes_currentIndexChanged(int index);
    void on_cbCaloAverage_clicked();
    void on_pbCaloShow_clicked();
    void on_pbUpdateCaloRange_clicked();

    void abortFind();
    void on_pbChooseWorkingDirectory_customContextMenuRequested(const QPoint &pos);
    void on_cbIncludeScintillators_clicked(bool checked);
    void on_sbNumThreadsStatistics_valueChanged(int arg1);
    void on_ledPTHistFromX_editingFinished();
    void on_ledPTHistToX_editingFinished();
    void on_ledPTHistFromY_editingFinished();
    void on_ledPTHistToY_editingFinished();
    void on_pbCaloShowDepoOverEvent_clicked();

    void on_pbChooseDepositionFile_clicked();
    void on_pbHelpOnDepositionDataFormat_clicked();
    void on_pbAnalyzeDepositionFile_clicked();

    void on_cbRandomSeed_toggled(bool checked);
    void on_pbLoadFromLibrary_clicked();

    void on_pbLoadAnalyzersData_clicked();
    void on_pbChooseAnalyzersFile_clicked();
    void on_pbAnalyzerShowEnergySpectrum_clicked();
    void on_pbNextAnalyzer_clicked();
    void on_cobAnalyzer_activated(int index);
    void on_sbAnalyzerUnqiueIndex_editingFinished();
    void on_cobAnalyzerNumberOption_activated(int index);
    void on_cobAnalyzerEnergyUnits_activated(int index);
    void on_pbHelpGetParticles_clicked();

    void on_pbOfferPhysLists_clicked();
};

#endif // APARTICLESIMWIN_H
