#ifndef ASENSORWINDOW_H
#define ASENSORWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class ASensorWindow;
}

class ASensorHub;
class TObject;
class QDoubleValidator;

class ASensorWindow : public AGuiWindow
{
    Q_OBJECT

public:
    ASensorWindow(QWidget * parent = nullptr);
    ~ASensorWindow();

    void updateGui();

private slots:
    //automatic
    void on_cobSensorType_currentIndexChanged(int index);

    //user actions
    void on_cobModel_activated(int index);
    void on_pbAddNewModel_clicked();
    void on_pbCloneModel_clicked();
    void on_pbRemoveModel_clicked();
    void on_leModelName_editingFinished();
    void on_ledEffectivePDE_editingFinished();

    void on_cobSensorType_activated(int index);
    void on_sbPixelsX_editingFinished();
    void on_sbPixelsY_editingFinished();
    void on_lepPixelSizeX_editingFinished();
    void on_lepPixelSizeY_editingFinished();
    void on_lepPixelSpacingY_editingFinished();
    void on_lepPixelSpacingX_editingFinished();

    void on_cobAssignmentMode_activated(int index);

    void on_pbShowSensorsOfThisModel_clicked();

    void on_pbLoadPDE_clicked();
    void on_pbRemovePDE_clicked();
    void on_pbShowPDE_clicked();

    void on_pbShowAngular_clicked();
    void on_pbLoadAngular_clicked();
    void on_pbRemoveAngular_clicked();

    void on_pbShowArea_clicked();
    void on_pbLoadArea_clicked();
    void on_pbRemoveArea_clicked();
    void on_lepAreaStepX_editingFinished();
    void on_lepAreaStepY_editingFinished();
    void on_pbShowPixelMap_clicked();

    void on_lepDarkRate_editingFinished();
    void on_lepIntegrationTime_editingFinished();

    void on_cobSignalModel_currentIndexChanged(int index); // user and automatic action
    void on_cobSignalModel_activated(int index);           // only user action
    void on_lepAverageSignalPerPhE_editingFinished();
    void on_lepNormalSigma_editingFinished();
    void on_lepGammaShape_editingFinished();
    void on_pbLoadCustomPhElSig_clicked();
    void on_pbShowCustomPhElSig_clicked();
    void on_pbRemoveCustomPhElSig_clicked();
    void on_lepElNoiseSigma_editingFinished();
    //void on_lepElGainFactor_editingFinished();
    void on_pbTestPhElSignal_clicked();

    void on_pbCompteEffectivePDE_clicked();

    void on_cbGains_clicked(bool checked);

    void on_pbGains_Clear_clicked();

    void on_pbGains_Randomize_clicked();

    void on_cbGains_ShowTable_toggled(bool checked);

    void onGainCellEditingFinished();

    void on_pbGains_Load_clicked();

    void on_pbGains_Save_clicked();

    void on_pbGains_Save_customContextMenuRequested(const QPoint &pos);

    void on_actionSave_sensor_triggered();
    void on_actionLoad_sensor_triggered();

    void on_pbShowPDE_customContextMenuRequested(const QPoint &pos);

    void on_pbShowAngular_customContextMenuRequested(const QPoint &pos);

    void on_ledAngularWave_editingFinished();

    void on_pbHelpPDEmodeling_clicked();

    void on_cobPDEmodel_activated(int index);

    void on_cobPDEmodel_currentIndexChanged(int index);

    void on_pbCheckTimeFraction_clicked();

    void on_pbCheckTimeFraction_customContextMenuRequested(const QPoint &pos);

private:
    ASensorHub & SensHub;
    Ui::ASensorWindow * ui = nullptr;

    const int RowHeight = 23;

    QDoubleValidator * CellValidator = nullptr;

    void updateModelGui();

    void updateNumPixels();
    void onModelIndexChanged();
    void updateHeader();
    void updatePdeButtons();
    void updateAngularButtons();
    void updateAreaButtons();
    void updatePhElToSigButtons();
    void updateGains();
    void showTableWithGains();

signals:
    void requestShowSensorModels(int iModel);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // ASENSORWINDOW_H
