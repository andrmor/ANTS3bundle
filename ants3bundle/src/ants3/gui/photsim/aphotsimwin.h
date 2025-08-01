#ifndef APHOTSIMWIN_H
#define APHOTSIMWIN_H

#include "aphotonsimsettings.h"
#include "aguiwindow.h"

#include <vector>

namespace Ui {
class APhotSimWin;
}

class TObject;
class AMonitorHub;
class APhotonBombFileHandler; // tmp ?
class ANodeRecord; // tmp ?
class ASensorDrawWidget;
class AFileHandlerBase;
class APhotonLogHandler;
class APhotonLogSettingsForm;

class APhotSimWin : public AGuiWindow
{
    Q_OBJECT

public:
    explicit APhotSimWin(QWidget * parent = nullptr);
    ~APhotSimWin();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void onNewConfigStartedInGui();

public slots:
    void updateGui();

private slots:
    void onProgressReceived(double progress);
    void showSimulationResults();

    void on_pbSimulate_clicked();

    void on_pbdWave_clicked();
    void on_sbMaxNumbPhTransitions_editingFinished();
    void on_cbRndCheckBeforeTrack_clicked();
    void on_cbRndCheckBeforeTrack_toggled(bool checked);
    void on_pbQEacceleratorHelp_clicked();

    void on_cobSimType_activated(int index);
    void on_cobNumPhotonsMode_activated(int index);
    void on_cobNodeGenerationMode_activated(int index);

    void on_ledSingleX_editingFinished();
    void on_ledSingleY_editingFinished();
    void on_ledSingleZ_editingFinished();
    void on_pbSingleSourceShow_clicked();

    void on_sbFloodNumber_editingFinished();
    void on_cobFloodShape_activated(int index);
    void on_ledFloodXfrom_editingFinished();
    void on_ledFloodXto_editingFinished();
    void on_ledFloodYfrom_editingFinished();
    void on_ledFloodYto_editingFinished();
    void on_ledFloodCenterX_editingFinished();
    void on_ledFloodCenterY_editingFinished();
    void on_ledFloodOuterDiameter_editingFinished();
    void on_ledFloodInnerDiameter_editingFinished();
    void on_cobFloodZmode_activated(int index);
    void on_ledFloodZ_editingFinished();
    void on_ledFloodZfrom_editingFinished();
    void on_ledFloodZto_editingFinished();

    void on_sbNumPhotons_editingFinished();
    void on_ledPoissonMean_editingFinished();
    void on_sbNumMin_editingFinished();
    void on_sbNumMax_editingFinished();
    void on_ledGaussSigma_editingFinished();
    void on_ledGaussMean_editingFinished();
    void on_pbNumDistShow_clicked();
    void on_pbNumDistLoad_clicked();
    void on_pbNumDistDelete_clicked();

    void on_pbConfigureOutput_clicked();

    void on_pbSelectTracksFile_clicked();
    void on_pbLoadAndShowTracks_clicked();
    void on_pbSelectStatisticsFile_clicked();
    void on_pbLoadAndShowStatistics_clicked();
    void on_pbShowTransitionDistr_clicked();
    void on_pbShowWaveDistr_clicked();
    void on_pbShowTimeDistr_clicked();
    void on_pbShowAngleDistr_clicked();

    // monitors
    void on_cobMonitor_activated(int index);
    void on_sbMonitorIndex_editingFinished();
    void on_pbNextMonitor_clicked();
    void on_pbChooseMonitorsFile_clicked();
    void on_pbLoadMonitorsData_clicked();
    //
    void on_pbMonitorShowAngle_clicked();
    void on_pbMonitorShowXY_clicked();
    void on_pbMonitorShowTime_clicked();
    void on_pbMonitorShowWaveIndex_clicked();
    void on_pbMonitorShowWavelength_clicked();
    void on_pbShowMonitorHitDistribution_clicked();
    void on_pbShowMonitorTimeOverall_clicked();

    // deposition from file
    void on_pbChangeDepositionFile_clicked();
    void on_leDepositionFile_editingFinished();
    void on_cbPrimaryScint_clicked(bool checked);
    void on_cbSecondaryScint_clicked(bool checked);
    void on_pbAnalyzeDepositionFile_clicked();
    void on_pbCollectDepoFileStatistics_clicked();
    void on_pbAdvancedBombSettings_clicked();
    void on_pbViewDepositionFile_clicked();
    void on_pbHelpDepositionFile_clicked();

    // photon bombs from file
    void on_pbSelectBombsFile_clicked();
    void on_leNodeFileName_editingFinished();
    void on_pbNodeFileChange_clicked();
    void on_pbBombFileCheck_clicked();
    void on_pbBombFileStatistics_clicked();
    void on_pbNodeFilePreview_clicked();
    void on_pbNodeFileHelp_clicked();
    void on_pbShowBombsMultiple_clicked();
    void on_cobNodeGenerationMode_currentIndexChanged(int index);
    void on_pbdUpdateScanSettings_clicked();

    // single photons from file
    void on_leSinglePhotonsFile_editingFinished();
    void on_pbChangeSinglePhotonsFile_clicked();
    void on_pbCheckSinglePhotonsFile_clicked();
    void on_pbSeeStatisticsSinglePhotonsFile_clicked();
    void on_pbViewSinglePhotFile_clicked();
    void on_pbSinglePhotonsHelp_clicked();

    void on_pbChangeWorkingDir_clicked();
    void on_pbLoadAllResults_clicked();

    void on_tbwResults_currentChanged(int index);

    void on_sbEvent_editingFinished();
    void on_pbEventNumberLess_clicked();
    void on_pbEventNumberMore_clicked();
    void on_pbShowEvent_clicked();

    void on_pbChooseSensorSigFile_clicked();
    void on_sbSensorTableColumns_editingFinished();
    void on_pbUpdateSensorIndication_clicked();
    void on_pbSensorStatIndividual_clicked();
    void on_pbSensorStatGroup_clicked();

    void on_pbSelectLogFile_clicked();
    void on_pbPhotonLog_first_clicked();
    void on_pbPhotonLog_next_clicked();
    void on_pbPhotonLog_ShowAll_clicked();

    void on_pbShowWaveDistr_customContextMenuRequested(const QPoint &pos);

    void on_pbAbort_clicked();

    void on_tbwResults_tabBarClicked(int index);

    void on_twSensors_tabBarClicked(int index);

private:
    APhotonSimSettings & SimSet;
    const AMonitorHub  & MonitorHub;

    Ui::APhotSimWin    * ui        = nullptr;
    ASensorDrawWidget  * gvSensors = nullptr;

    ABombFileSettings      * BombFileSettings = nullptr; // !!!*** tmp, move to simMamager to be accessible from scripts ?
    APhotonBombFileHandler * BombFileHandler  = nullptr; // !!!*** tmp, move to simMamager to be accessible from scripts ?

    AFileSettingsBase * SignalsFileSettings = nullptr;
    AFileHandlerBase  * SignalsFileHandler  = nullptr;

    APhotonLogHandler * LogHandler = nullptr;

    APhotonLogSettingsForm * LogForm = nullptr;

    QPixmap YellowCircle;

    bool bFreshDataLoaded = false;

    std::vector<std::vector<float>> SensorSignals;

    void updatePhotBombGui();
    void updateDepoGui();
    void updateBombFileGui();
    void updatePhotonFileGui();
    void updateGeneralSettingsGui();
    void updateMonitorGui();

    void storeGeneralSettings();

    void disableInterface(bool flag);

    void updateAdvancedBombIndicator();

    void setGuiEnabled(bool flag); // make it public to use in global disable/enable?

    void doShowEvent();

    void showSensorSignals(bool suppressMessage);
    void showSensorSignalDraw(const std::vector<float> & signalArray, const std::vector<int> & enabledSensors);
    void showSensorSignalTable(const std::vector<float> & signalArray, const std::vector<int> & enabledSensors);
    void fillListEnabledSensors(std::vector<int> & enabledSensors);
    void reshapeSensorSignalTable();
    void loadAllSensorSignals();

    void showBombSingleEvent();
    bool updateBombHandler();

    void loadStatistics(bool suppressMessage);
    void loadMonitorsData(bool suppressMessage);
    void loadAndShowTracks(bool suppressMessage, int selectedEvent = -1);
    void showTracksSingleEvent();

    QString initPhotonLogHandler(); // returns error if any
    void showLogRecord();
    void resetViewportOnNewData();

signals:
    void requestShowGeometry(bool ActivateWindow = true, bool SAME = true, bool ColorUpdateAllowed = true);
    void requestShowTracks(bool activateWindow = false);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);

    void requestClearGeoMarkers(int All_Rec_True);
    void requestAddPhotonNodeGeoMarker(const ANodeRecord & record);
    void requestShowGeoMarkers();
    void requestShowPosition(double * pos, bool keepTracks);
    void requestConfigureExchangeDir();
};

#endif // APHOTSIMWIN_H
