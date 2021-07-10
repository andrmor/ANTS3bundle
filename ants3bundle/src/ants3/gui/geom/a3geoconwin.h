#ifndef A3GEOCONWIN_H
#define A3GEOCONWIN_H

//#include "aguiwindow.h"

#include <QMainWindow>

class MainWindow;
class AGeometry;
class DetectorClass;
class AGeo_SI;
class AGeoTree;
class AGeoObject;

namespace Ui {
  class A3GeoConWin;
}

class A3GeoConWin : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit A3GeoConWin(QWidget * parent);
  ~A3GeoConWin();

  void UpdateGUI(); //update gui controls

  AGeo_SI  * AddObjScriptInterface = nullptr;  // if created -> owned by the script manager
  AGeoTree * twGeo   = nullptr;                  // WorldTree widget

private slots:
  void onReconstructDetectorRequest();
  void onGeoConstEditingFinished(int index, QString newValue);
  void onGeoConstExpressionEditingFinished(int index, QString newValue);
  void onGeoConstEscapePressed(int index);
  void onRequestShowPrototypeList();
  void updateMenuIndication();
  void onSandwichRebuild();

  void on_tabwConstants_customContextMenuRequested(const QPoint &pos);
  void on_pbSaveTGeo_clicked();
  void on_pbLoadTGeo_clicked();
  void on_pbBackToSandwich_clicked();
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


private:
  AGeometry & Geometry;
  Ui::A3GeoConWin * ui;
  MainWindow * MW;
  DetectorClass * Detector;

  QString ObjectScriptTarget;

  bool bGeoConstsWidgetUpdateInProgress = false;

  void    HighlightVolume(const QString & VolName);
  bool    GDMLtoTGeo(const QString &fileName);
  QString loadGDML(const QString &fileName, QString &gdml);  //returns error string - empty if OK
  void    updateGeoConstsIndication();
  QString createScript(QString &script, bool usePython);

protected:
  void resizeEvent(QResizeEvent *event);

public slots:
  void UpdateGeoTree(QString name = "");
  void ShowObject(QString name = "");
  void FocusVolume(QString name);
  void ShowObjectRecursive(QString name);
  void ShowAllInstances(QString name);
  void OnrequestShowMonitor(const AGeoObject* mon);
  void onRequestEnableGeoConstWidget(bool flag);

signals:
  void requestDelayedRebuildAndRestoreDelegate();

};

#include <QLineEdit>
class ALineEditWithEscape : public QLineEdit
{
    Q_OBJECT
public:
    ALineEditWithEscape(const QString & text, QWidget * parent) : QLineEdit(text, parent){}

protected:
    void keyPressEvent(QKeyEvent * event);

signals:
    void escapePressed();
};

#endif // A3GEOCONWIN_H
