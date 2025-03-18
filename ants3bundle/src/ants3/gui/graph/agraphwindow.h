#ifndef AGRAPHWINDOW_H
#define AGRAPHWINDOW_H

#include "aguiwindow.h"
#include "adrawobject.h"
#include "adrawtemplate.h"

#include <QVector>
#include <QVariantList>
#include <QMouseEvent>
#include <QCloseEvent>

class RasterWindowGraphClass;
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

public:
    explicit AGraphWindow(QWidget * parent);
    ~AGraphWindow();
    friend class ADrawExplorerWidget;

    //Drawing
    void Draw(TObject* obj, const char* options = "", bool DoUpdate = true, bool TransferOwnership = true);  //registration should be skipped only for scripts!
    void DrawWithoutFocus(TObject* obj, const char* options = "", bool DoUpdate = true, bool TransferOwnership = true);  //registration should be skipped only for scripts!
    void RedrawAll();

    //canvas control
    void ShowAndFocus();
    void SetAsActiveRootWindow();
    void ClearRootCanvas();
    void UpdateRootCanvas();
    void SetModifiedFlag();

    //Canvas size in actual coordinates of plotted data
    double getCanvasMinX();
    double getCanvasMaxX();
    double getCanvasMinY();
    double getCanvasMaxY();

    //Values presented to user in Range boxes
    double getMinX(bool *ok);
    double getMaxX(bool *ok);
    double getMinY(bool *ok);
    double getMaxY(bool *ok);
    double getMinZ(bool *ok);
    double getMaxZ(bool *ok);

    //extraction of coordinates from graph
    bool IsExtractionComplete();
    bool IsExtractionCanceled() {return ExtractionCanceled;}

    //commands to start extraction of shapes on canvas
    void ExtractX();  //start extraction of X coordinate from a 1D graph/histogram using mouse
    void Extract2DLine();  //start extraction of ABC line coordinate from a 2D graph/histogram using mouse
    void Extract2DEllipse();  //start extraction of (T)ellipse from a 2D graph/histogram using mouse
    void Extract2DBox();  //start extraction of 2D box (2 opposite corners)
    void Extract2DPolygon();  //start extraction of 2D polygon, extraction ends by right click (or doubleclick?)

    //commands to get results of extraction
    double extractedX();
    double extracted2DLineA();
    double extracted2DLineB();
    double extracted2DLineC();
    double extracted2DLineXstart();
    double extracted2DLineXstop();
    double extracted2DLineYstart();
    double extracted2DLineYstop();
    double extracted2DEllipseX();
    double extracted2DEllipseY();
    double extracted2DEllipseR1();
    double extracted2DEllipseR2();
    double extracted2DEllipseTheta();
    double extractedX1();
    double extractedY1();
    double extractedX2();
    double extractedY2();
    QList<double> extractedPolygon();

    void AddLine(double x1, double y1, double x2, double y2, int color, int width, int style);
    void AddArrow(double x1, double y1, double x2, double y2, int color, int width, int style);

    void OnBusyOn();
    void OnBusyOff();

    bool Extraction();

    TObject * GetMainPlottedObject();
    void SaveGraph(const QString & fileName);
    void EnforceOverlayOff();
    void ClearDrawObjects_OnShutDown(); //precvents crash on shut down
    void RegisterTObject(TObject* obj);

    void SetStatPanelVisible(bool flag); // script interface
    void TriggerGlobalBusy(bool flag); // !!!*** not implemented!

    void MakeCopyOfDrawObjects(); // !!!*** infamous "gcc optimizer fix:"
    void ClearCopyOfDrawObjects();

    void ClearBasketActiveId();
    void MakeCopyOfActiveBasketId();
    void RestoreBasketActiveId();
    void ClearCopyOfActiveBasketId();
    void ShowProjectionTool();
    TLegend * addLegend();
    void HighlightUpdateBasketButton(bool flag);

    QString UseProjectionTool(const QString & option);
    void    ConfigureProjectionTool(double x0, double y0, double dx, double dy, double angle);

    void close3DviewWindow();
    void doRedrawOnUpdateMargins();

protected:
    void mouseMoveEvent(QMouseEvent * event);
    bool event(QEvent * event);
    void closeEvent(QCloseEvent * event);

public slots:
    void UpdateControls(); //updates visualisation of the current master graph parameters   !!!***
    void DoSaveGraph(QString name);
    void addCurrentToBasket(const QString & name);
    void ClearBasket();
    void SetLog(bool X, bool Y);
    void drawLegend(double x1, double y1, double x2, double y2, QString title);
    void ShowTextPanel(const QString Text, bool bShowFrame=true, int AlignLeftCenterRight=0,
                       double x1=0.15, double y1=0.75, double x2=0.5, double y2=0.85, const QString opt = "NDC");
    void SetLegendBorder(int color, int style, int size);
    void on_pbAddLegend_clicked();
    //void ExportTH2AsText(QString fileName); //for temporary script command
    //QVector<double> Get2DArray(); //for temporary script command

    void DrawStrOpt(TObject* obj, QString options = "", bool DoUpdate = true);
    void onDrawRequest(TObject* obj, QString options, bool transferOwnership, bool focusWindow);

    void show3D(QString castorFileName, bool keepSettings);

    void addObjectToBasket(TObject * obj, QString options, QString name);

private slots:
    void onScriptDrawRequest(TObject * obj, QString options, bool fFocus);
    void processScriptDrawRequest(TObject * obj, QString options, bool fFocus);
    // !!!*** TODO: similarly to two above, modify draw tree from script
    bool onScriptDrawTree(TTree * tree, QString what, QString cond, QString how,
                          QVariantList binsAndRanges = QVariantList(), QVariantList markersAndLines = QVariantList(), QString * result = nullptr);

    void Reshape();
    void BasketCustomContextMenuRequested(const QPoint &pos);
    void onBasketItemDoubleClicked(QListWidgetItem *item);
    void BasketReorderRequested(const QVector<int> & indexes, int toRow);
    void deletePressed();
    void onCursorPositionReceived(double x, double y, bool bOn);

    void on_pbToolboxDragMode_clicked();

    void selBoxGeometryChanged();
    void selBoxResetGeometry(double halfW, double halfH);
    void selBoxControlsUpdated();
    void on_pbSelBoxToCenter_clicked();
    void on_pbSelBoxFGColor_clicked();
    void on_pbSelBoxBGColor_clicked();
    void rulerGeometryChanged();
    void rulerControlsP1Updated();
    void rulerControlsP2Updated();
    void rulerControlsLenAngleUpdated();
    void on_ledRulerTicksLength_editingFinished();
    void on_pbRulerFGColor_clicked();
    void on_pbRulerBGColor_clicked();
    void on_pbResetRuler_clicked();
    void on_pbAddToBasket_clicked();
    void on_cbGridX_toggled(bool checked);
    void on_cbGridY_toggled(bool checked);
    void on_cbLogX_toggled(bool checked);
    void on_cbLogY_toggled(bool checked);
    void on_ledXfrom_editingFinished();
    void on_ledXto_editingFinished();
    void on_ledYfrom_editingFinished();
    void on_ledYto_editingFinished();
    void on_ledZfrom_editingFinished();
    void on_ledZto_editingFinished();
    void on_cbShowLegend_toggled(bool checked);
    void on_pbZoom_clicked();
    void on_pbUnzoom_clicked();
    void on_leOptions_editingFinished();
    void on_pbXprojection_clicked();
    void on_pbYprojection_clicked();
    void on_actionBasic_ROOT_triggered();
    void on_actionDeep_sea_triggered();
    void on_actionGrey_scale_triggered();
    void on_actionDark_body_radiator_triggered();
    void on_actionTwo_color_hue_triggered();
    void on_actionRainbow_triggered();
    void on_actionInverted_dark_body_triggered();
    void on_pbToolboxDragMode_2_clicked();
    void on_actionTop_triggered();
    void on_actionSide_triggered();
    void on_actionFront_triggered();
    void on_pbDensityDistribution_clicked();
    void on_actionEqualize_scale_XY_triggered();
    void on_ledRulerDX_editingFinished();
    void on_ledRulerDY_editingFinished();
    void on_cbShowFitParameters_toggled(bool checked);
    void on_pbXaveraged_clicked();
    void on_pbYaveraged_clicked();
    void on_pbAddText_clicked();
    void on_pbRemoveLegend_clicked();
    void on_ledAngle_customContextMenuRequested(const QPoint &pos);
    void on_actionToggle_toolbar_triggered(bool checked);
    void on_pbBackToLast_clicked();
    void on_actionToggle_Explorer_Basket_toggled(bool arg1);
    void on_pbUpdateInBasket_clicked();
    void on_actionShow_ROOT_attribute_panel_triggered();
    void on_pbShowRuler_clicked();
    void on_pbExitToolMode_clicked();
    void on_actionSet_width_triggered();
    void on_actionSet_height_triggered();
    void on_actionMake_square_triggered();
    void on_actionCreate_template_triggered();
    void on_actionApply_template_triggered();
    void on_actionApply_selective_triggered();

    void on_actionShow_first_drawn_object_context_menu_triggered();

    void on_pbManipulate_clicked();

    void on_cbShowCross_toggled(bool checked);

    void on_actionOpen_MultiGraphDesigner_triggered();

    void onExternalBasketChange();

private:
    Ui::AGraphWindow       * ui = nullptr;

    ABasketManager         * Basket       = nullptr;
    ADrawExplorerWidget    * Explorer     = nullptr;
    ABasketListWidget      * lwBasket     = nullptr;
    RasterWindowGraphClass * RasterWindow = nullptr;
    QGraphicsView          * gvOver       = nullptr;
    AToolboxScene          * scene        = nullptr;
    AMultiGraphDesigner    * MGDesigner   = nullptr;
    AViewer3D              * Viewer3D     = nullptr;

    QVector<ADrawObject>     DrawObjects;  //always local objects -> can have a copy from the Basket
    QVector<ADrawObject>     PreviousDrawObjects; //last draw made from outside of the graph window

    QVector<TObject*>       tmpTObjects;

    ADrawTemplate           DrawTemplate;

    int  ActiveBasketItem         = -1; //-1 - Basket is off; 0+ -> basket loaded, can be updated
    int  PreviousActiveBasketItem = -1; //-1 - Basket is off; 0+ -> basket loaded, can be updated
    bool ExtractionCanceled       = false;
    int  LastOptStat              = 1111;
    bool TMPignore                = false; //temporarily forbid updates - need for bulk update to avoid cross-modification
    bool ColdStart                = true;
    bool DrawFinished             = false;

    double xmin, xmax, ymin, ymax, zmin, zmax;

    void doDraw(TObject *obj, const char *opt, bool DoUpdate); //actual drawing, does not have window focussing - done to avoid refocussing issues leading to bugs

    void clearTmpTObjects();   //enable qDebugs inside for diagnostics of cleanup!

    void changeOverlayMode(bool bOn);

    void switchToBasket(int index);
    void UpdateBasketGUI();
    void Basket_DrawOnTop(int row);

    void ShowProjection(QString type);
    void UpdateGuiControlsForMainObject(const QString &ClassName, const QString & options);
    void contextMenuForBasketMultipleSelection(const QPoint &pos);
    void removeAllSelectedBasketItems();
    void requestMultidraw();
    void requestMergeHistograms();
    void applyTemplate(bool bAll);
    void updateSecondaryAxis(TGaxis *gaxis, const char *opt);
    void showHintInStatus();
    void setShowCursorPosition(bool flag);
    void fixGraphFrame();
    void updateLogScaleFlags(QVector<ADrawObject> & drawObjects) const;
    void createMGDesigner();
    void connectScriptUnitDrawRequests(const std::vector<AScriptInterface *> interfaces);
    void updateMargins(ADrawObject * obj = nullptr);

private slots:
    void onRequestMakeCopyViewer3D(AViewer3D * ptr);

    void on_actionSet_default_margins_triggered();

    void on_pbSaveImage_clicked();

    void on_pbSaveImage_customContextMenuRequested(const QPoint &pos);

    void on_actionSave_image_2_triggered();

    void on_actionCopy_image_to_clipboard_triggered();

    void on_actionSet_histogram_stat_box_content_triggered();

    void on_actionSet_palette_triggered();

signals:
    void requestLocalDrawObject(TObject *obj, QString options, bool fFocus);
};

#endif // AGRAPHWINDOW_H
