#ifndef GEOMETRYWINDOWCLASS_H
#define GEOMETRYWINDOWCLASS_H

#include "aguiwindow.h"
#include "ageowriter.h"

#include <QVector>

#include "TMathBase.h"

class AGeometryHub;
class RasterWindowBaseClass;
class QWebEngineView;
class QWebEngineDownloadItem;
class TGeoVolume;
class ACameraControlDialog;
class GeoMarkerClass;
class ANodeRecord;

namespace Ui {
class AGeometryWindow;
}

class AGeometryWindow : public AGuiWindow
{
    Q_OBJECT

friend class AShowNumbersDialog;

public:
    explicit AGeometryWindow(QWidget * parent);
    ~AGeometryWindow();

    bool ModePerspective = true;
    int  ZoomLevel       = 0;
    bool fRecallWindow   = false;
    bool bDisableDraw    = false;

    std::vector<GeoMarkerClass*> GeoMarkers;

    void ShowAndFocus();
    void SetAsActiveRootWindow();
    void ClearRootCanvas();

    void SaveAs(const QString & filename);
    void OpenGLview();

    void ResetView();
    void setHideUpdate(bool flag);
    void PostDraw();
    void Zoom(bool update = false);

    void AddLineToGeometry(QPointF &start, QPointF &end, Color_t color = 1, int width = 1);
    void AddPolygonfToGeometry(QPolygonF &poly, Color_t color, int width);

    void onBusyOn();
    void onBusyOff();

    bool isColorByMaterial() {return ColorByMaterial;}

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    bool IsWorldVisible();

    void ShowPMsignals(const QVector<float> &Event, bool bFullCycle = true);
    void ShowTracksAndMarkers();

    void ClearTracks(bool bRefreshWindow = true);

protected:
    bool event(QEvent *event) override;
    void closeEvent(QCloseEvent * event) override;

public slots:
    void ShowGeometry(bool ActivateWindow = true, bool SAME = true, bool ColorUpdateAllowed = true);
    void showRecursive(QString objectName);
    void UpdateRootCanvas();
    void ShowTracks();
    void ShowPoint(double * r, bool keepTracks = false);
    void addGenerationMarker(const double * Pos);
    void FocusVolume(QString name);
    void CenterView(double * r);
    void showPhotonMonIndexes();  // !!!***
    void showParticleMonIndexes();  // !!!***
    void showSensorIndexes();  // !!!***
    void showCalorimeterIndexes();  // !!!***
    void showSensorModelIndexes(int iModel = -1);  // !!!***

    void showText(const std::vector<QString> & textVec, int color, AGeoWriter::EDraw onWhat, bool bFullCycle = true);

    void on_pbTop_clicked();
    void on_pbFront_clicked();
    void onRasterWindowChange();
    void readRasterWindowProperties();   // !*!

    void on_pbShowTracks_clicked();
    void on_pbClearTracks_clicked();

    void clearGeoMarkers(int All_Rec_True = 0);
    void showGeoMarkers();
    void addPhotonNodeGeoMarker(const ANodeRecord & record);

    void addGeoMarkers(const std::vector<std::array<double, 3>> & XYZs, int color, int style, double size);

private slots:
    void onDownloadPngRequested(QWebEngineDownloadItem *item);

private slots:
    void on_pbShowGeometry_clicked();
    void on_cbShowTop_toggled(bool checked);
    void on_cbColor_toggled(bool checked);
    void on_pbSaveAs_clicked();
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
    void on_cobViewer_currentIndexChanged(int index);
    void on_actionOpen_GL_viewer_triggered();
    void on_actionJSROOT_in_browser_triggered();
    void on_cbWireFrame_toggled(bool checked);
    void on_cbLimitVisibility_clicked();
    void on_sbLimitVisibility_editingFinished();
    void on_pbCameraDialog_clicked();
    void on_pbClearMarkers_clicked();
    void on_pbShowNumbers_clicked();

private:
    AGeometryHub          & Geometry;

    Ui::AGeometryWindow   * ui = nullptr;
    RasterWindowBaseClass * RasterWindow = nullptr;

    ACameraControlDialog  * CameraControl = nullptr;

#ifdef __USE_ANTS_JSROOT__
    QWebEngineView * WebView = nullptr;
#endif

    int GeoMarkerSize  = 2;
    int GeoMarkerStyle = 6;

    bool TMPignore = false;
    bool ShowTop = false;
    bool ColorByMaterial = false;

    AGeoWriter GeoWriter;

private:
    void doChangeLineWidth(int deltaWidth);
    void showWebView();
    void prepareGeoManager(bool ColorUpdateAllowed = true);
    void adjustGeoAttributes(TGeoVolume * vol, int Mode, int transp, bool adjustVis, int visLevel, int currentLevel);

signals:
    void requestUpdateRegisteredGeoManager();
    void requestUpdateMaterialListWidget();
    void requestShowNetSettings();
};

#endif // GEOMETRYWINDOWCLASS_H
