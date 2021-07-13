#ifndef MATERIALINSPECTORWINDOW_H
#define MATERIALINSPECTORWINDOW_H

//#include "aguiwindow.h"
#include <QMainWindow>

class A3Geometry;
class A3MatHub;
class A3Global;
class TGraph;
class QJsonObject;
class AElasticScatterElement;
class QTreeWidgetItem;
class AChemicalElement;
class ANeutronInteractionElement;
class AMaterial;

namespace Ui {
class MaterialInspectorWindow;
}

class MaterialInspectorWindow : public QMainWindow // AGuiWindow
{
    Q_OBJECT

public:
    explicit MaterialInspectorWindow(QWidget* parent);
    ~MaterialInspectorWindow();

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
    void on_cobActiveMaterials_activated(int index);
    void on_leName_editingFinished();
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
    void on_pbRename_clicked();          // !!!*** need to update GeoManager
    void on_pbAddNewMaterial_clicked();
    void on_ledIntEnergyRes_editingFinished();
    void on_cobYieldForParticle_activated(int index);
    void on_lePriT_raise_editingFinished();
    void on_pbCopyPrYieldToAll_clicked();
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
    void on_pbCopyIntrEnResToAll_clicked();
    void on_pbModifyByWeight_clicked();

    //user or code controlled change - safe or only GUI
    void on_ledRayleigh_textChanged(const QString &arg1);
    void on_pteComments_textChanged();
    void on_cbG4Material_toggled(bool checked);

    //menu actions
    void on_actionSave_material_triggered();
    void on_actionLoad_material_triggered();
//    void on_actionLoad_from_material_library_triggered();   !!!***
    void on_actionAdd_default_material_triggered();

    void on_pbUpdateMaterial_clicked();

private:
    A3Geometry & Geometry;
    A3MatHub   & MatHub;
    A3Global   & GlobSet;

    Ui::MaterialInspectorWindow * ui = nullptr;

    bool bMaterialWasModified = false;
    bool flagDisreguardChange = false;
    bool fLockTable           = false;
    bool bLockTmpMaterial     = false;
    bool bMessageLock         = false;
    bool bClearInProgress     = false;

    int  LastShownMaterial    = -1;

private:
    void updateTmpMaterialGui();   // yield / EnRes  !!!***
    void addNewOrUpdateMaterial();

    void showMaterial(int index);
    void setWasModified(bool flag);

    void updateActionButtons();
    void updateWaveButtons();

    void ShowTreeWithChemicalComposition();

    bool parseDecayOrRaiseTime(bool doParseDecay);
    void updateWarningIcons();
    void updateG4RelatedGui();
};

#endif // MATERIALINSPECTORWINDOW_H
