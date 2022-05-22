#ifndef A3GEOCONWIN_H
#define A3GEOCONWIN_H

#include "aguiwindow.h"

#include <vector>
#include <array>

#include <QString>

class AGeometryHub;
class AMaterialHub;
class AGeoTree;
class AGeoObject;

namespace Ui {
  class A3GeoConWin;
}

class A3GeoConWin : public AGuiWindow
{
  Q_OBJECT
  
public:
  explicit A3GeoConWin(QWidget * parent);
  ~A3GeoConWin();

  void updateGui();

private slots:
  void onRebuildDetectorRequest();
  void onGeoConstEditingFinished(int index, QString newValue);
  void onGeoConstEscapePressed(int index);
  void onRequestShowPrototypeList();
  void updateMenuIndication();

  void on_tabwConstants_customContextMenuRequested(const QPoint & pos);
  void on_pbSaveTGeo_clicked();
  void on_pbRootWeb_clicked();
  void on_pbCheckGeometry_clicked();
  void on_cbAutoCheck_clicked(bool checked);
  void on_pbRunTestParticle_clicked();
  void on_cbAutoCheck_stateChanged(int arg1);
  void on_pmParseInGeometryFromGDML_clicked();
  void on_tabwConstants_cellChanged(int row, int column);
  void on_actionUndo_triggered();
  void on_actionRedo_triggered();
  void on_actionHow_to_use_drag_and_drop_triggered();
  void on_actionTo_JavaScript_triggered();
  void on_cbShowPrototypes_toggled(bool checked);

  void on_tabwConstants_cellClicked(int row, int column);

  void on_actionFind_object_triggered();

private:
  AGeometryHub       & Geometry;
  const AMaterialHub & MaterialHub;

  Ui::A3GeoConWin * ui    = nullptr;
  AGeoTree        * twGeo = nullptr;                // WorldTree widget

  bool bGeoConstsWidgetUpdateInProgress = false;

  void    highlightVolume(const QString & VolName);  // !!!***  slow!

  bool    GDMLtoTGeo(const QString &fileName);
  void    updateGeoConstsIndication();
  void    reportGeometryConflicts();

  void    markCalorimeterBinning(const AGeoObject * obj);

protected:
  void resizeEvent(QResizeEvent * event);

public slots:
  void UpdateGeoTree(QString name = "", bool bShow = false);
  void ShowObject(QString name = "");
  void FocusVolume(QString name);
  void ShowObjectRecursive(QString name); // !!!***
  void showAllInstances(QString name);
  void onRequestShowMonitorActiveDirection(const AGeoObject* mon);
  void onRequestEnableGeoConstWidget(bool flag);
  void onMaterialsChanged();

signals:
  void requestRebuildGeometry(); // to the parent, direct connection
  void requestShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed);
  void requestShowTracks();
  void requestFocusVolume(QString name);
  void requestAddGeoMarkers(const std::vector<std::array<double, 3>> & XYZs, int color, int style, double size);
  void requestClearGeoMarkers(int All_Rec_True);
  void requestAddScript(const QString & script);

  void requestDelayedRebuildAndRestoreDelegate();  //local

};

#endif // A3GEOCONWIN_H
