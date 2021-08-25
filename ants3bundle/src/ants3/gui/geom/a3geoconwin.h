#ifndef A3GEOCONWIN_H
#define A3GEOCONWIN_H

//#include "aguiwindow.h"

#include <QMainWindow>

class AGeometryHub;
class AMaterialHub;
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

  void updateGui();

private slots:
  void onRebuildDetectorRequest();
  void onGeoConstEditingFinished(int index, QString newValue);
  void onGeoConstExpressionEditingFinished(int index, QString newValue);
  void onGeoConstEscapePressed(int index);
  void onRequestShowPrototypeList();
  void updateMenuIndication();

  void on_tabwConstants_customContextMenuRequested(const QPoint &pos);
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

private:
  AGeometryHub       & Geometry;
  const AMaterialHub & MaterialHub;

  Ui::A3GeoConWin * ui    = nullptr;
  AGeoTree        * twGeo = nullptr;                // WorldTree widget

  //QString ObjectScriptTarget;
  bool bGeoConstsWidgetUpdateInProgress = false;

  void    highlightVolume(const QString & VolName);  // !!!***  slow!

  bool    GDMLtoTGeo(const QString &fileName);
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
  void onRequestShowMonitorActiveDirection(const AGeoObject* mon);
  void onRequestEnableGeoConstWidget(bool flag);

signals:
  void requestRebuildGeometry(); // to the parent, direct connection
  void requestShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed);
  void requestShowTracks();
  void requestFocusVolume(QString name);

  void requestDelayedRebuildAndRestoreDelegate();  //local

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
