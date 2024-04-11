#ifndef AMATWIN_H
#define AMATWIN_H

#include "aguiwindow.h"
#include "amaterial.h"

class AGeometryHub;
class AMaterialHub;
class A3Global;
class AChemicalElement;
class TObject;

namespace Ui {
class AMatWin;
}

class AMatWin : public AGuiWindow
{
    Q_OBJECT

public:
    explicit AMatWin(QWidget * parent);
    ~AMatWin();

    void initWindow();
    void updateGui();
    void setMaterial(int index);
//    void AddMaterialFromLibrary(QWidget * parentWidget);   !!!***

private slots:
    //on user input
    void on_pbRemove_clicked();
    void on_pbAddNew_clicked();
    void on_pbClone_clicked();
    void on_pbAcceptChanges_clicked();
    void on_pbCancel_clicked();
    void on_leName_textChanged(const QString &arg1);
    void on_leName_editingFinished();
    void on_cobActiveMaterials_activated(int index);
    void on_pbUpdateTmpMaterial_clicked();
    void on_pbLoadPrimSpectrum_clicked();
    void on_pbShowPrimSpectrum_clicked();
    void on_pbDeletePrimSpectrum_clicked();
    void on_pbLoadSecSpectrum_clicked();
    void on_pbShowSecSpectrum_clicked();
    void on_pbDeleteSecSpectrum_clicked();
    void on_pbLoadNlambda_clicked();
    void on_pbShowNlambda_clicked();
    void on_pbDeleteNlambda_clicked();
    void on_pbLoadABSlambda_clicked();
    void on_pbShowABSlambda_clicked();
    void on_pbDeleteABSlambda_clicked();
    void on_pbWasModified_clicked();
    void on_ledRayleighWave_editingFinished();
    void on_ledRayleigh_editingFinished();
    void on_pbRemoveRayleigh_clicked();
    void on_pbShowUsage_clicked();
    void on_ledIntEnergyRes_editingFinished();
    void on_lePriT_raise_editingFinished();
    void on_pbShowReemProbLambda_clicked();
    void on_pbLoadReemisProbLambda_clicked();
    void on_pbDeleteReemisProbLambda_clicked();
    void on_lePriT_editingFinished();
    void on_pbPriThelp_clicked();
    void on_pbPriT_test_clicked();
    void on_pbSecScintHelp_clicked();
    void on_pbListGeant4Materials_clicked();
    void on_pbShowComplexN_clicked();
    void on_pbLoadComplexN_clicked();
    void on_pbDeleteComplexN_clicked();
    void on_leComposition_editingFinished();
    void on_pbHelpComposition_clicked();
    void on_pbInspectG4Material_clicked();

    //user or code controlled change - safe or only GUI
    void on_ledRayleigh_textChanged(const QString &arg1);
    void on_pteComments_textChanged();
    void on_cobCompositionType_currentIndexChanged(int index);
    void on_cobMeanExcitationEnergy_currentIndexChanged(int index);

    //menu actions
    void on_actionSave_material_triggered();
    void on_actionLoad_material_triggered();
//    void on_actionLoad_from_material_library_triggered();   !!!***
    void on_actionAdd_default_material_triggered();
    void on_actionRemove_selected_material_triggered();

    void on_cbGas_toggled(bool checked);
    void on_cbGas_clicked(bool checked);
    void on_ledPressure_editingFinished();
    void on_cobPressureUnits_activated(int index);

    void on_ledT_editingFinished();

    void on_ledCustoimScatterMFP_editingFinished();

private:
    AGeometryHub & Geometry;
    AMaterialHub & MatHub;
    A3Global     & GlobSet;

    Ui::AMatWin * ui = nullptr;

    QString DefaultPBStyle;

    AMaterial tmpMaterial;

    bool bMaterialWasModified = false;
    bool flagDisreguardChange = false;
    bool bMessageLock         = false;
    bool bClearInProgress     = false;

    int  LastShownMaterial    = -1;

private:
    void updateTmpMaterialGui();   // yield / EnRes  !!!***
    void switchToMaterial(int index);
    void setWasModified(bool flag);
    void updateActionButtons();
    void updateWaveButtons();
    bool parseDecayOrRaiseTime(bool decay_or_raise);
    void updateWarningIcons();   // !!!***
    void updateG4RelatedGui();   // !!!*** empty
    void configureG4Materials();
    bool checkCurrentMaterial();
    void fillElementInfo();

signals:
    void requestRebuildDetector();
    void requestShowGeometry(bool activateWindow, bool same, bool colorUpdateAllowed);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // AMATWIN_H
