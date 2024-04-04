#ifndef AGEODELEGATEWIDGET_H
#define AGEODELEGATEWIDGET_H

#include <QWidget>
#include <QString>

class AGeometryHub;
class AMaterialHub;
class AGeoTree;
class AGeoObject;
class AGeoShape;
class AGeoBaseDelegate;
class QVBoxLayout;
class QFrame;
class QPushButton;
class TObject;

class AGeoDelegateWidget : public QWidget
{
  Q_OBJECT

public:
  AGeoDelegateWidget(AGeoTree * tw);
  //destructor does not delete Widget - it is handled by the layout

  void ClearGui();
  void UpdateGui();

  QString getCurrentObjectName() const;

private:
  AGeometryHub & Geometry;
  AMaterialHub & Materials;
  AGeoTree  * tw = nullptr;

  AGeoObject       * CurrentObject = nullptr;
  AGeoBaseDelegate * GeoDelegate   = nullptr;

  QVBoxLayout * lMain;
  QVBoxLayout * ObjectLayout;
  QFrame      * frBottom;
  QPushButton * pbConfirm;
  QPushButton * pbCancel;

  bool fIgnoreSignals = true;
  bool fEditingMode   = false;

public slots:
  void onObjectSelectionChanged(QString SelectedObject); //starts GUI update
  void onStartEditing();
  void onRequestChangeShape(AGeoShape * NewShape);
  void onMonitorRequestsShowSensitiveDirection();

  void onRequestShowCurrentObject();
  void onRequestScriptLineToClipboard();
  void onRequestScriptRecursiveToClipboard();
  void onRequestSetVisAttributes();

  void onConfirmPressed(); // CONFIRM BUTTON PRESSED: performing copy from delegate to object
  void onCancelPressed();

private:
  void exitEditingMode();
  void updateInstancesOnProtoNameChange(QString oldName, QString newName);
  AGeoBaseDelegate * createAndAddGeoObjectDelegate();
  AGeoBaseDelegate * createAndAddGridElementDelegate();
  AGeoBaseDelegate * createAndAddMonitorDelegate();

signals:
  void showMonitor(const AGeoObject* mon);
  void requestBuildScript(AGeoObject * obj, QString & script, int ident, bool bExpandMaterial, bool bRecursive, bool usePython);
  void requestEnableGeoConstWidget(bool);
  void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // AGEODELEGATEWIDGET_H
