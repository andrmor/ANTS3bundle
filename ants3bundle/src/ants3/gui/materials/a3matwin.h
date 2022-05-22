#ifndef A3MATWIN_H
#define A3MATWIN_H

#include "aguiwindow.h"
#include "amaterial.h"

class AGeometryHub;
class AMaterialHub;
class A3Global;
class AChemicalElement;
class TObject;

namespace Ui {
class A3MatWin;
}

class A3MatWin : public AGuiWindow
{
    Q_OBJECT

public:
    explicit A3MatWin(QWidget * parent);
    ~A3MatWin();

    void initWindow();
    void updateGui();
    void setMaterial(int index);
//    void AddMaterialFromLibrary(QWidget * parentWidget);   !!!***

private slots:
    void onMaterialsChanged(); //sent by A3MatHub

    //on signals from delegates
    void onAddIsotope(AChemicalElement *element);
    void onRemoveIsotope(AChemicalElement* element, int isotopeIndexInElement);
    void IsotopePropertiesChanged(const AChemicalElement* element, int isotopeIndexInElement);
    void onRequestDraw(const QVector<double> & x, const QVector<double> & y, const QString & titleX, const QString & titleY); // !!!***

    //on user input    
    void on_leName_textChanged(const QString &arg1);
    void on_leName_editingFinished();
    void on_cobActiveMaterials_activated(int index);
    void on_pbUpdateTmpMaterial_clicked();
    void on_pbLoadPrimSpectrum_clicked();
    void on_pbShowPrimSpectrum_clicked();  // !!!***
    void on_pbDeletePrimSpectrum_clicked();
    void on_pbLoadSecSpectrum_clicked();
    void on_pbShowSecSpectrum_clicked();   // !!!***
    void on_pbDeleteSecSpectrum_clicked();
    void on_pbLoadNlambda_clicked();
    void on_pbShowNlambda_clicked();       // !!!***
    void on_pbDeleteNlambda_clicked();
    void on_pbLoadABSlambda_clicked();
    void on_pbShowABSlambda_clicked();    // !!!***
    void on_pbDeleteABSlambda_clicked();
    void on_pbWasModified_clicked();
    void on_ledRayleighWave_editingFinished();
    void on_ledRayleigh_editingFinished();
    void on_pbRemoveRayleigh_clicked();
    void on_pbShowUsage_clicked();       // !!!***
    void on_ledIntEnergyRes_editingFinished();
    void on_lePriT_raise_editingFinished();
    void on_pbModifyChemicalComposition_clicked();
    void on_cbShowIsotopes_clicked();
    void on_pbMaterialInfo_clicked();
    void on_trwChemicalComposition_doubleClicked(const QModelIndex &index);
    void on_pbShowReemProbLambda_clicked();     // !!!***
    void on_pbLoadReemisProbLambda_clicked();
    void on_pbDeleteReemisProbLambda_clicked();
    void on_lePriT_editingFinished();
    void on_pbPriThelp_clicked();
    void on_pbPriT_test_clicked(); // !!!***
    void on_pbSecScintHelp_clicked();
    void on_pbModifyByWeight_clicked();
    void on_pbListGeant4Materials_clicked();

    //user or code controlled change - safe or only GUI
    void on_ledRayleigh_textChanged(const QString &arg1);
    void on_pteComments_textChanged();
    void on_cbG4Material_toggled(bool checked);

    //menu actions
    void on_actionSave_material_triggered();
    void on_actionLoad_material_triggered();
//    void on_actionLoad_from_material_library_triggered();   !!!***
    void on_actionAdd_default_material_triggered();
    void on_actionRemove_selected_material_triggered();

    void on_pbRemove_clicked();

    void on_pbAddNew_clicked();

    void on_pbClone_clicked();

    void on_pbAcceptChanges_clicked();

    void on_pbCancel_clicked();


private:
    AGeometryHub & Geometry;
    AMaterialHub & MatHub;
    A3Global     & GlobSet;

    Ui::A3MatWin * ui = nullptr;

    QString DefaultPBStyle;

    AMaterial tmpMaterial;

    bool bMaterialWasModified = false;
    bool flagDisreguardChange = false;
    bool bLockTmpMaterial     = false;   // need?
    bool bMessageLock         = false;
    bool bClearInProgress     = false;

    int  LastShownMaterial    = -1;

private:
    void updateTmpMaterialGui();   // yield / EnRes  !!!***
    void switchToMaterial(int index);
    void setWasModified(bool flag);
    void updateActionButtons();
    void updateWaveButtons();
    void ShowTreeWithChemicalComposition();
    bool parseDecayOrRaiseTime(bool doParseDecay);
    void updateWarningIcons();
    void updateG4RelatedGui();
    void configureG4Materials();

    bool checkCurrentMaterial();
signals:
    void requestShowGeometry(bool activateWindow, bool same, bool colorUpdateAllowed);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // A3MATWIN_H
