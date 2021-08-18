#ifndef A3PHOTSIMWIN_H
#define A3PHOTSIMWIN_H

#include "aphotonsimsettings.h"

#include <QMainWindow>

namespace Ui {
class A3PhotSimWin;
}

class TObject;

class A3PhotSimWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit A3PhotSimWin(QWidget * parent = nullptr);
    ~A3PhotSimWin();

public slots:
    void updateGui();

private slots:
    void on_pbSimulate_clicked();

    void on_pbdWave_clicked();
    void on_sbMaxNumbPhTransitions_editingFinished();
    void on_cbRndCheckBeforeTrack_clicked();
    void on_pbQEacceleratorHelp_clicked();

    void on_cobSimType_activated(int index);
    void on_cobNumPhotonsMode_activated(int index);
    void on_cobNodeGenerationMode_activated(int index);

    void on_ledSingleX_editingFinished();
    void on_ledSingleY_editingFinished();
    void on_ledSingleZ_editingFinished();

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

    void on_cobMonitor_activated(int index);

    void on_sbMonitorIndex_editingFinished();

    void on_pbNextMonitor_clicked();

    void on_pbChooseMonitorsFile_clicked();

    void on_pbLoadMonitorsData_clicked();

private:
    APhotonSimSettings & SimSet;
    Ui::A3PhotSimWin * ui = nullptr;

    void updatePhotBombGui();
    void updateGeneralSettingsGui();

    void storeGeneralSettings();

    void disableInterface(bool flag);

    void updateMonitorGui();

signals:
    void requestShowGeometry(bool ActivateWindow = true, bool SAME = true, bool ColorUpdateAllowed = true);
    void requestShowTracks();
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // A3PHOTSIMWIN_H
