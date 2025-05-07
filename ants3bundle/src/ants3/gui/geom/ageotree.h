#ifndef AGEOTREE_H
#define AGEOTREE_H

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QImage>
#include <QColor>
#include <QList>
#include <QStringList>

class AGeoBaseTreeWidget;
class AGeoObject;
class AGeoShape;
class AGeoDelegateWidget;
class AGeometryHub;
class QPoint;
class QTreeWidgetItem;
class TObject;

class AGeoTree : public QObject
{
  Q_OBJECT

public:
  AGeoTree();

  AGeoDelegateWidget * GetEditWidget() {return EditWidget;}
  void         SetLineAttributes(AGeoObject * obj);
  void         SelectObjects(QStringList ObjectNames);

  AGeoBaseTreeWidget * twGeoTree    = nullptr;
  AGeoBaseTreeWidget * twPrototypes = nullptr;
  QString LastShownObjectName;

public slots:
  void UpdateGui(QString ObjectName = "");
  void onGridReshapeRequested(QString objName);
  void rebuildDetectorAndRestoreCurrentDelegate();  // used by geoConst widget
  void onRequestShowPrototype(QString ProtoName);
  void onRequestIsValidPrototypeName(const QString & ProtoName, bool & bResult) const;
  void ShowAllInstances(AGeoObject * obj);

private slots:
  void onItemSelectionChanged();
  void onProtoItemSelectionChanged();
  void customMenuRequested(const QPoint &pos);      // ==== World tree CONTEXT MENU ====
  void customProtoMenuRequested(const QPoint &pos); // ---- Proto tree CONTEXT MENU ----
  void onItemClicked();
  void onProtoItemClicked();

  void onItemExpanded(QTreeWidgetItem * item);
  void onItemCollapsed(QTreeWidgetItem * item);
  void onPrototypeItemExpanded(QTreeWidgetItem * item);
  void onPrototypeItemCollapsed(QTreeWidgetItem * item);

  void onRemoveTriggered();
  void onRemoveRecursiveTriggered();

private:
  AGeometryHub & Geometry;
  AGeoObject   * World      = nullptr;
  AGeoObject   * Prototypes = nullptr;

  AGeoDelegateWidget * EditWidget = nullptr;

  QTreeWidgetItem * topItemPrototypes = nullptr;
  bool bWorldTreeSelected = true;

  //base images for icons
  QImage Lock;
  QImage StackStart, StackMid, StackEnd;

  //QColor BackgroundColor = QColor(240,240,240);
  bool   fSpecialGeoViewMode = false;

  void loadImages();
  void populateTreeWidget(QTreeWidgetItem *parent, AGeoObject *Container, bool fDisabled = false);
  void updateExpandState(QTreeWidgetItem * item, bool bPrototypes); //recursive!
  void updateIcon(QTreeWidgetItem *item, AGeoObject *obj);
  void menuActionFormStack(QList<QTreeWidgetItem *> selected);
  void markAsStackRefVolume(AGeoObject * obj);
  void clearStackRefVolume(AGeoObject * obj);
  void updatePrototypeTreeGui();

  void menuActionMoveObject(AGeoObject * obj, int option); //0 1 2 3 = top up down bottom
  void menuActionAddNewObject(AGeoObject * contObj, AGeoShape * shape);
  void menuActionCloneObject(AGeoObject * obj);
  void focusObject(const QString & objName);
  void ShowObject(AGeoObject * obj);
  void ShowObjectRecursive(AGeoObject * obj);
  void ShowObjectOnly(AGeoObject * obj);  // !!!***
  void menuActionEnableDisable(AGeoObject * obj);
  void menuActionRemoveKeepContent(QTreeWidget * treeWidget);
  void menuActionRemoveHostedObjects(AGeoObject * obj);
  void menuActionRemoveWithContent(QTreeWidget * treeWidget);
  void menuActionAddNewComposite(AGeoObject * contObj);
  void menuActionAddNewArray(AGeoObject * contObj);
  void menuActionAddNewCircularArray(AGeoObject * contObj);
  void menuActionAddNewHexagonalArray(AGeoObject * contObj);
  void menuActionAddNewGrid(AGeoObject * contObj);
  void menuActionAddNewMonitor(AGeoObject * contObj, bool isPhoton);
  void menuActionAddInstance(AGeoObject * contObj, const QString & prototypeName);
  void menuActionMakeItPrototype(const QList<QTreeWidgetItem *> & selected);
  void menuActionMoveProtoToWorld(AGeoObject * obj);
  void protoMenuEmptySelection(const QPoint & pos);

signals:
  void ObjectSelectionChanged(QString);
  void ProtoObjectSelectionChanged(QString);
  void RequestRebuildDetector();
  void RequestHighlightObject(QString name);
  void RequestFocusObject(QString name);
  void RequestShowObjectRecursive(QString name);
  void RequestShowAllInstances(QString name);
  void RequestNormalDetectorDraw();
  void RequestShowMonitor(const AGeoObject * mon);
  void RequestShowPrototypeList();
  void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // AGEOTREE_H
