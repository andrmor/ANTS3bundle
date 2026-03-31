#ifndef GEOMETRYWINDOWCLASS_H
#define GEOMETRYWINDOWCLASS_H

#include "aguiwindow.h"
#include "ageowriter.h"

#include <vector>

class AGeometryHub;
class ARasterWindow;
class QWebEngineView;
class QWebEngineDownloadItem;
class TGeoVolume;
class ACameraControlDialog;
class AGeoMarkerClass;
class ANodeRecord;
class TVirtualGeoTrack;

namespace Ui {
class AGeometryWindow;
}

class AGeometryWindow : public AGuiWindow
{
    Q_OBJECT

friend class AShowNumbersDialog;

public:
    explicit AGeometryWindow(bool jsrootViewer, QWidget * parent);
    ~AGeometryWindow();

    bool ModePerspective = true;
    int  ZoomLevel       = 0;
    bool fRecallWindow   = false;
    bool bDisableDraw    = false;

    std::vector<AGeoMarkerClass*> GeoMarkers;

    void SaveAs(const QString & filename);

    void ResetView();
    void setHideUpdate(bool flag);
    void PostDraw();
    void Zoom(bool update = false);

    void AddLineToGeometry(QPointF & start, QPointF & end, short color = 1, int width = 1); // not used
    void AddPolygonfToGeometry(QPolygonF &poly, short color, int width); // not used

    void onBusyOn();
    void onBusyOff();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void ShowPMsignals(const std::vector<float> & event, bool bFullCycle = true); // not used
    void ShowTracksAndMarkers();

    void ClearTracks(bool bRefreshWindow = true);

protected:
    bool event(QEvent *event) override; // !!!***
    void closeEvent(QCloseEvent * event) override;

public slots:
    void ClearRootCanvas();
    void onNewConfigLoaded();
    void ShowGeometry(bool ActivateWindow = true, bool SAME = true, bool ColorUpdateAllowed = true);
    void onRequestRedrawFromScript();
    void showRecursive(QString objectName);
    void UpdateRootCanvas();
    void ShowTracks(bool activateWindow = false);
    void onRequestShowTracksFromScript();
    void onRequestClearTracksFromScript();
    void onRequestClearMarkersFromScript();
    void onRequestSaveImageFromScript(QString fileName);
    void onRequestAddMarkersFromScript(AGeoMarkerClass * markers);
    void onRequestAddTrackFromScript(TVirtualGeoTrack * track);
    void ShowPoint(double * r, bool keepTracks = false);
    void addGenerationMarker(const double * Pos);
    void FocusVolume(QString name);
    void CenterView(double * r);
    void showPhotonMonIndexes();  // !!!***
    void showParticleMonIndexes();  // !!!***
    void showSensorIndexes();  // !!!***
    void showCalorimeterIndexes();  // !!!***
    void showSensorModelIndexes(int iModel = -1);  // !!!***
    void showSensorGains();  // !!!***
    void showPhotonFunctionalIndexes();  // !!!***
    void showAnalyzerIndexes();  // !!!***
    void showScintillatorIndexes();  // !!!***

    void showText(const std::vector<QString> & textVec, int color, AGeoWriter::EDraw onWhat, bool bFullCycle = true);

    void onRasterWindowChange();
    void readRasterWindowProperties();

    void on_pbShowTracks_clicked();
    void on_pbClearTracks_clicked();

    void clearGeoMarkers(int All_Rec_True = 0);
    void showGeoMarkers();
    void addPhotonNodeGeoMarker(const ANodeRecord & record);

    void addGeoMarkers(const std::vector<std::array<double, 3>> & XYZs, int color, int style, double size);

    void onRequestShowConnection(int from, int to);
    void onRequestShowAllConnections();

private slots:
    void on_cobViewer_currentIndexChanged(int index);
    void on_pbShowGeometry_clicked();
    void on_cbColor_toggled(bool checked);
    void on_pbSaveAs_clicked();
    void on_pbTop_clicked();
    void on_pbFront_clicked();
    void on_pbSide_clicked();
    void on_cobViewType_currentIndexChanged(int index);
    void on_cbShowAxes_toggled(bool checked);
    void on_actionSmall_dot_toggled(bool arg1);
    void on_actionLarge_dot_triggered(bool arg1);
    void on_actionSmall_cross_toggled(bool arg1);
    void on_actionLarge_cross_toggled(bool arg1);
    void on_actionSize_1_triggered();
    void on_actionSize_2_triggered();
    void on_actionDefault_zoom_1_triggered();
    void on_actionDefault_zoom_2_triggered();
    void on_actionDefault_zoom_to_0_triggered();
    void on_actionSet_line_width_for_objects_triggered();
    void on_actionDecrease_line_width_triggered();
    void on_actionOpen_GL_viewer_triggered();
    void on_actionJSROOT_in_browser_triggered(); // !!!*** refactor + avoid hard coded port of the root server!
    void on_cbLimitVisibility_clicked();
    void on_sbLimitVisibility_editingFinished();
    void on_pbCameraDialog_clicked();
    void on_pbClearMarkers_clicked();
    void on_pbShowNumbers_clicked();

    void on_cbWireFrame_clicked(bool checked);

    void on_sbTransparency_editingFinished();

    void on_cbShowTop_clicked(bool checked);

    void on_cbColor_customContextMenuRequested(const QPoint &pos);

    void on_cbLimitVisibility_toggled(bool checked);

    void on_pbSaveAs_customContextMenuRequested(const QPoint &pos);

    void on_actionSet_number_of_segments_triggered();

private:
    bool                    UseJSRoot = false;
    AGeometryHub          & Geometry;

    Ui::AGeometryWindow   * ui = nullptr;

    ARasterWindow * RasterWindow = nullptr;
#ifdef __USE_ANTS_JSROOT__
    QWebEngineView * WebView = nullptr;
#endif

    ACameraControlDialog  * CameraControl = nullptr;

    int GeoMarkerSize  = 2;
    int GeoMarkerStyle = 6;

    bool TMPignore = false;
    bool ShowTop = false;
    bool ColorByMaterial = false;

    AGeoWriter GeoWriter;
    int LastShowObjectType = 0;

private:
    void redrawWebView(QString extraArguments = "");
    void prepareGeoManager(bool ColorUpdateAllowed = true);
    void showGeometryRasterWindow(bool SAME);
    void showGeometryJSRootWindow();

    void ShowAndFocus();
    void SetAsActiveRootWindow();

    void doChangeLineWidth(int deltaWidth);
    void adjustGeoAttributes(TGeoVolume * vol, int Mode, int transp, bool adjustVis, int visLevel, int currentLevel);
    void copyGeoMarksToGeoManager();

    void onWebPageReplyViewPort(const QVariant & reply);

    void showPhotonTunnel(int from, int to);

signals:
    void requestChangeGeoViewer(bool useJSRoot);
    void requestUpdateRegisteredGeoManager(); // Geometry.notifyRootServerGeometryChanged();
    void requestShowNetSettings();
    void taskRequestedFromScriptCompleted();
};

#endif // GEOMETRYWINDOWCLASS_H
