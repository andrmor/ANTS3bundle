#ifndef AGRAPHWINDOW_H
#define AGRAPHWINDOW_H

#include "aguiwindow.h"
#include "adrawobject.h"
#include "adrawtemplate.h"
#include "apadproperties.h"

#include <QVariantList>

#include <vector>

class AGraphRasterWindow;
class QGraphicsView;
class AToolboxScene;
class QListWidgetItem;
class TObject;
class TTree;
class ABasketManager;
class ADrawExplorerWidget;
class ABasketListWidget;
class TLegend;
class TGaxis;
class AMultiGraphDesigner;
class AScriptInterface;
class AViewer3D;

namespace Ui {
class AGraphWindow;
}

class AGraphWindow : public AGuiWindow
{
    Q_OBJECT
    friend class ADrawExplorerWidget;

public:
    explicit AGraphWindow(QWidget * parent);
    ~AGraphWindow();

    void draw(TObject * obj, QString options, bool update = true, bool transferOwnership = true);

public slots:
    void onDrawRequest(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
    void redrawAll();
    void addCurrentToBasket(const QString & name);
    void drawLegend(double x1, double y1, double x2, double y2, QString title);
    void configureLegendBorder(int color, int style, int size);
    void clearBasket();
    void saveGraph(const QString & fileName);
    void setStatPanelVisible(bool flag); // script interface
    void setLogScale(bool X, bool Y);
    void addTextPanel(QString text, bool bShowFrame=true, int alignLeftCenterRight=0, double x1=0.15, double y1=0.75, double x2=0.5, double y2=0.85, QString opt = "NDC");
    void addLine(double x1, double y1, double x2, double y2, int color, int width, int style);
    void addArrow(double x1, double y1, double x2, double y2, int color, int width, int style);

    void showAddLegendDialog(); // called from draw explorer widget
    void show3D(QString castorFileName, bool keepSettings); // script interface

    void addObjectToBasket(TObject * obj, QString options, QString name); // called from viewer3D. Maybe drop?

public:
    TObject * getMainPlottedObject();        // direct access in script interface
    void      clearDrawObjects_OnShutDown(); // prevents crash on shutdown

    // Values shown in Range boxes of GUI
    double getMinX(bool *ok);
    double getMaxX(bool *ok);
    double getMinY(bool *ok);
    double getMaxY(bool *ok);
    double getMinZ(bool *ok);
    double getMaxZ(bool *ok);

    void close3DviewWindow();

protected:
    void mouseMoveEvent(QMouseEvent * event);
    bool event(QEvent * event);
    void closeEvent(QCloseEvent * event);

private slots:
    // range control
    void updateControls(); //updates visualisation of the current master graph parameters   !!!*** need refactor and improvement for hist objects
    void reshape();
    // basket-related
    void onBasketCustomContextMenuRequested(const QPoint & pos);
    void onBasketItemDoubleClicked(QListWidgetItem * item);
    void onBasketReorderRequested(const std::vector<int> & indexes, int toRow);
    void onBasketDeleteShortcutActivated();
    // script related
    void onScriptDrawRequest(TObject * obj, QString options, bool fFocus);      // these two work together (QueuedConnection to enable calls from another thread)
    void processScriptDrawRequest(TObject * obj, QString options, bool fFocus); // these two work together (QueuedConnection to enable calls from another thread)
    // !!!*** next needs serious refactor! Old comment: similarly to two above, modify draw tree from script
    bool onScriptDrawTree(TTree * tree, QString what, QString cond, QString how,
                          QVariantList binsAndRanges = QVariantList(), QVariantList markersAndLines = QVariantList(), QString * result = nullptr);
    // selection box and ruler
    void selBoxGeometryChanged();
    void selBoxResetGeometry(double halfW, double halfH);
    void selBoxControlsUpdated();
    void rulerGeometryChanged();
    void rulerControlsP1Updated();
    void rulerControlsP2Updated();
    void rulerControlsLenAngleUpdated();
    // misc
    void onCursorPositionReceived(double x, double y, bool bOn);
    void onRequestMakeCopyViewer3D(AViewer3D * ptr);
    void onExternalBasketChange();

    // on user interaction with GUI elements
    void on_pbAddLegend_clicked();
    void on_pbToolboxDragMode_clicked();
    void on_pbSelBoxToCenter_clicked();
    void on_pbSelBoxFGColor_clicked();
    void on_pbSelBoxBGColor_clicked();
    void on_cbGridX_toggled(bool checked);
    void on_cbGridY_toggled(bool checked);
    void on_cbLogX_toggled(bool checked);
    void on_cbLogY_toggled(bool checked);
    void on_ledRulerTicksLength_editingFinished();
    void on_pbRulerFGColor_clicked();
    void on_pbRulerBGColor_clicked();
    void on_pbResetRuler_clicked();
    void on_pbAddToBasket_clicked();
    void on_ledXfrom_editingFinished();
    void on_ledXto_editingFinished();
    void on_ledYfrom_editingFinished();
    void on_ledYto_editingFinished();
    void on_ledZfrom_editingFinished();
    void on_ledZto_editingFinished();
    void on_cbShowLegend_toggled(bool checked);
    void on_leOptions_editingFinished();
    void on_pbXprojection_clicked();
    void on_pbYprojection_clicked();
    void on_pbDensityDistribution_clicked();
    void on_pbToolboxDragMode_2_clicked();
    void on_pbZoom_clicked();
    void on_pbUnzoom_clicked();
    void on_ledRulerDX_editingFinished();
    void on_ledRulerDY_editingFinished();
    void on_cbShowFitParameters_toggled(bool checked);
    void on_pbXaveraged_clicked();
    void on_pbYaveraged_clicked();
    void on_pbAddText_clicked();
    void on_pbRemoveLegend_clicked();
    void on_ledAngle_customContextMenuRequested(const QPoint &pos);
    void on_pbBackToLast_clicked();
    void on_pbUpdateInBasket_clicked();
    void on_pbShowRuler_clicked();
    void on_pbExitToolMode_clicked();
    void on_pbManipulate_clicked();
    void on_cbShowCross_toggled(bool checked);
    void on_pbSaveImage_clicked();
    void on_pbSaveImage_customContextMenuRequested(const QPoint &pos);

    // actions
    void on_actionBasic_ROOT_triggered();
    void on_actionDeep_sea_triggered();
    void on_actionGrey_scale_triggered();
    void on_actionDark_body_radiator_triggered();
    void on_actionTwo_color_hue_triggered();
    void on_actionRainbow_triggered();
    void on_actionInverted_dark_body_triggered();
    void on_actionTop_triggered();
    void on_actionSide_triggered();
    void on_actionFront_triggered();
    void on_actionEqualize_scale_XY_triggered();
    void on_actionToggle_toolbar_triggered(bool checked);
    void on_actionToggle_Explorer_Basket_toggled(bool arg1);
    void on_actionShow_ROOT_attribute_panel_triggered();
    void on_actionSet_width_triggered();
    void on_actionSet_height_triggered();
    void on_actionMake_square_triggered();
    void on_actionCreate_template_triggered();
    void on_actionApply_template_triggered();
    void on_actionApply_selective_triggered();
    void on_actionShow_first_drawn_object_context_menu_triggered();
    void on_actionOpen_MultiGraphDesigner_triggered();
    void on_actionSet_default_margins_triggered();
    void on_actionSave_image_2_triggered();
    void on_actionCopy_image_to_clipboard_triggered();
    void on_actionSet_histogram_stat_box_content_triggered();
    void on_actionSet_palette_triggered();

    void on_pbRemoveTextBox_clicked();

    void on_sbMultNumX_editingFinished();


    void on_sbMultNumY_editingFinished();

private:
    Ui::AGraphWindow       * ui = nullptr;

    ABasketManager         * Basket       = nullptr;
    ADrawExplorerWidget    * Explorer     = nullptr;
    ABasketListWidget      * lwBasket     = nullptr;
    AGraphRasterWindow     * RasterWindow = nullptr;
    QGraphicsView          * gvOverlay    = nullptr;
    AToolboxScene          * ToolBoxScene = nullptr;
    AMultiGraphDesigner    * MGDesigner   = nullptr;
    AViewer3D              * Viewer3D     = nullptr;

    std::vector<ADrawObject> DrawObjects;         //always local objects -> can have a copy from the Basket
    std::vector<ADrawObject> PreviousDrawObjects; //last draw made from outside of the graph window

    std::vector<TObject*>    RegisteredTObjects;

    ADrawTemplate            DrawTemplate;

    int  ActiveBasketItem         = -1; //-1 - Basket is off; 0+ -> basket loaded, can be updated
    int  PreviousActiveBasketItem = -1; //-1 - Basket is off; 0+ -> basket loaded, can be updated
    int  LastOptStat              = 1111;
    bool TMPignore                = false; //temporarily forbid updates - need for bulk update to avoid cross-modification
    bool ColdStart                = true;
    bool DrawFinished             = false;

    double xmin, xmax, ymin, ymax, zmin, zmax;

    // Multidraw
    std::vector<APadProperties> Pads;
    void clearPads();

private:
    // Canvas control
    void showAndFocus();
    void setAsActiveRootWindow();
    void clearRootCanvas();
    void updateRootCanvas();
    void setModifiedFlag();

    // Canvas size in actual coordinates of plotted data
    double getCanvasMinX();
    double getCanvasMaxX();
    double getCanvasMinY();
    double getCanvasMaxY();

    void drawSingleObject(TObject * obj, QString options, bool update);
    void registerTObject(TObject * obj);
    void clearRegisteredTObjects();
    void requestMultidraw();
    void requestMultidrawNew();  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void doRedrawOnUpdateMargins();
    void updateGuiControlsForMainObject(const QString & className, const QString & options);
    void showProjection(QString type);
    void requestMergeHistograms();
    void applyTemplate(bool bAll);
    void updateSecondaryAxis(TGaxis * gaxis, QString options);
    void showHintInStatus();
    void setShowCursorPosition(bool flag);
    void fixGraphFrame();
    void updateLogScaleFlags(std::vector<ADrawObject> & drawObjects) const;
    void createMGDesigner();
    void connectScriptUnitDrawRequests(const std::vector<AScriptInterface *> interfaces);
    void updateMargins(ADrawObject * obj = nullptr);
    TLegend * addLegend();

    void makeCopyOfDrawObjects(); // old message was here: "gcc optimizer fix:"
    void clearCopyOfDrawObjects();

    void updateBasketGUI();
    void switchToBasket(int index);
    void basket_DrawOnTop(int row);
    void highlightUpdateBasketButton(bool flag);
    void contextMenuForBasketMultipleSelection(const QPoint & pos); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void removeAllSelectedBasketItems();
    void clearBasketActiveId();
    void makeCopyOfActiveBasketId();
    void restoreBasketActiveId();
    void clearCopyOfActiveBasketId();

    QString useProjectionTool(const QString & option);
    void    configureProjectionTool(double x0, double y0, double dx, double dy, double angle);
    void changeOverlayMode(bool bOn);
    void enforceOverlayOff();
    void showProjectionTool();

    void triggerGlobalBusy(bool flag); // currently disabled - do we need it? One option is trigger inside local busy
    void onBusyOn();
    void onBusyOff();

    void redrawAll_Multidraw(ADrawObject & drawObj); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bool isMultidrawModeOn();

signals:
    void requestLocalDrawObject(TObject *obj, QString options, bool fFocus);
};

#endif // AGRAPHWINDOW_H
