#include "ageotree.h"
#include "ageometryhub.h"
#include "ageobasetreewidget.h"
#include "ageodelegatewidget.h"
#include "ageobasedelegate.h"
#include "ageoobjectdelegate.h"
#include "amonitordelegate.h"
#include "agridelementdelegate.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
//#include "ashapehelpdialog.h"
#include "arootlineconfigurator.h"
#include "agridelementdialog.h"
#include "amonitordelegateform.h"
#include "guitools.h"
#include "ageoconsts.h"
#include "a3global.h"

#include <QDebug>
#include <QDropEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QApplication>
#include <QPainter>
#include <QClipboard>
#include <QShortcut>

#include <vector>

#include "TMath.h"
#include "TGeoShape.h"

AGeoTree::AGeoTree() :
    QObject(), Geometry(AGeometryHub::getInstance()), World(Geometry.World), Prototypes(Geometry.Prototypes)
{
    loadImages();

    // main geo tree widget
    twGeoTree = new AGeoBaseTreeWidget(World);
    connect(twGeoTree,    &AGeoBaseTreeWidget::customContextMenuRequested, this,       &AGeoTree::customMenuRequested);
    connect(twGeoTree,    &AGeoBaseTreeWidget::itemSelectionChanged,       this,       &AGeoTree::onItemSelectionChanged);
    connect(twGeoTree,    &AGeoBaseTreeWidget::itemExpanded,               this,       &AGeoTree::onItemExpanded);
    connect(twGeoTree,    &AGeoBaseTreeWidget::itemCollapsed,              this,       &AGeoTree::onItemCollapsed);
    connect(twGeoTree,    &AGeoBaseTreeWidget::itemClicked,                this,       &AGeoTree::onItemClicked);
    connect(twGeoTree,    &AGeoBaseTreeWidget::RequestRebuildDetector,     this,       &AGeoTree::RequestRebuildDetector);

    // widget for delegates
    EditWidget = new AGeoDelegateWidget(this);
    connect(EditWidget,   &AGeoDelegateWidget::showMonitor,                this,       &AGeoTree::RequestShowMonitor);
    connect(EditWidget,   &AGeoDelegateWidget::requestDraw,                this,       &AGeoTree::requestDraw);
    connect(this,         &AGeoTree::ObjectSelectionChanged,               EditWidget, &AGeoDelegateWidget::onObjectSelectionChanged);

    // tree for prototypes
    twPrototypes = new AGeoBaseTreeWidget(World);
    connect(twPrototypes, &AGeoBaseTreeWidget::customContextMenuRequested, this,       &AGeoTree::customProtoMenuRequested);
    connect(twPrototypes, &AGeoBaseTreeWidget::itemExpanded,               this,       &AGeoTree::onPrototypeItemExpanded);
    connect(twPrototypes, &AGeoBaseTreeWidget::itemCollapsed,              this,       &AGeoTree::onPrototypeItemCollapsed);
    connect(twPrototypes, &AGeoBaseTreeWidget::itemSelectionChanged,       this,       &AGeoTree::onProtoItemSelectionChanged);
    connect(twPrototypes, &AGeoBaseTreeWidget::itemClicked,                this,       &AGeoTree::onProtoItemClicked);
    connect(twPrototypes, &AGeoBaseTreeWidget::RequestRebuildDetector,     this,       &AGeoTree::RequestRebuildDetector);
    connect(this,         &AGeoTree::ProtoObjectSelectionChanged,          EditWidget, &AGeoDelegateWidget::onObjectSelectionChanged);

    // shortcuts
    QShortcut * Del = new QShortcut(Qt::Key_Backspace, twGeoTree, nullptr,nullptr, Qt::WidgetShortcut);
    connect(Del, &QShortcut::activated, this, &AGeoTree::onRemoveTriggered);
    QShortcut * DelRec = new QShortcut(QKeySequence(QKeySequence::Delete), twGeoTree, nullptr,nullptr, Qt::WidgetShortcut);
    connect(DelRec, &QShortcut::activated, this, &AGeoTree::onRemoveRecursiveTriggered);
}

void AGeoTree::loadImages()
{
    QString dir = ":/images/gui/images/";

    Lock.load(dir+"lock.png");
    //GroupStart.load(dir+"TopGr.png");
    //GroupMid.load(dir+"MidGr.png");
    //GroupEnd.load(dir+"BotGr.png");
    StackStart.load(dir+"TopSt.png");
    StackMid.load(dir+"MidSt.png");
    StackEnd.load(dir+"BotSt.png");
}

void AGeoTree::SelectObjects(QStringList ObjectNames)
{
   twGeoTree->clearSelection();
   //qDebug() << "Request select the following objects:"<<ObjectNames;

   for (int i=0; i<ObjectNames.size(); i++)
     {
       QList<QTreeWidgetItem*> list = twGeoTree->findItems(ObjectNames.at(i), Qt::MatchExactly | Qt::MatchRecursive);
       if (!list.isEmpty())
         {
            //qDebug() << "Attempting to focus:"<<list.first()->text(0);
            list.first()->setSelected(true);
         }
     }
}

void AGeoTree::UpdateGui(QString ObjectName)
{
    if (!World) return;

    //qDebug() << "==> Update tree triggered, ObjectName = "<<ObjectName;

    EditWidget->ClearGui();

    blockSignals(true);
    twGeoTree->clear();
    blockSignals(false);

    twPrototypes->blockSignals(true);
    twPrototypes->clear();
    twPrototypes->blockSignals(false);

    //World
    QTreeWidgetItem * topItem = new QTreeWidgetItem(twGeoTree);
    topItem->setText(0, "World");
    QFont f = topItem->font(0);
    f.setBold(true);
    topItem->setFont(0, f);
    topItem->setSizeHint(0, QSize(50, 20));
    topItem->setFlags(topItem->flags() & ~Qt::ItemIsDragEnabled);// & ~Qt::ItemIsSelectable);
    //w->setBackgroundColor(0, BackgroundColor);

    populateTreeWidget(topItem, World);
    updateExpandState(topItem, false);
    updatePrototypeTreeGui();


    // restoring delegate for the last shown obeject if possible, otherwise showing delegate for the World
    if (ObjectName.isEmpty())
    {
        if (LastShownObjectName.isEmpty())
        {
            ObjectName = "World";
            LastShownObjectName = "World";
        }
        else ObjectName = LastShownObjectName;
    }
    QList<QTreeWidgetItem*> list = twPrototypes->findItems(ObjectName, Qt::MatchExactly | Qt::MatchRecursive);
    if (list.isEmpty())
    {
        bWorldTreeSelected = true;
        list = twGeoTree->findItems(ObjectName, Qt::MatchExactly | Qt::MatchRecursive);
    }
    else bWorldTreeSelected = false;

    if (list.isEmpty())
    {
        if (twGeoTree->topLevelItemCount() > 0) twGeoTree->setCurrentItem(twGeoTree->topLevelItem(0));
    }
    else
    {
        if (bWorldTreeSelected) twGeoTree->setCurrentItem(list.first());
        else twPrototypes->setCurrentItem(list.first());
    }
}

void AGeoTree::updatePrototypeTreeGui()
{
    if (!Prototypes) return;

    topItemPrototypes = new QTreeWidgetItem(twPrototypes);
    topItemPrototypes->setText(0, "Defined prototypes:");
    QFont f = topItemPrototypes->font(0); f.setBold(true); topItemPrototypes->setFont(0, f);
    topItemPrototypes->setSizeHint(0, QSize(50, 20));
    topItemPrototypes->setFlags(topItemPrototypes->flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsSelectable);
    Prototypes->fExpanded = true;  // force-expand top!

    populateTreeWidget(topItemPrototypes, Prototypes);
    updateExpandState(topItemPrototypes, true);
}

#include "agridhub.h"
void AGeoTree::onGridReshapeRequested(QString objName)
{
    AGeoObject * obj = World->findObjectByName(objName);
    if (!obj) return;
    if (!obj->Type->isGrid()) return;

    if (!obj->getGridElement()) return;
    ATypeGridElementObject * GE = static_cast<ATypeGridElementObject*>(obj->getGridElement()->Type);

    AGridElementDialog * d = new AGridElementDialog(EditWidget);
    switch (GE->shape)
    {
    case 0: d->setValues(0, GE->size1, GE->size2, obj->getGridElement()->Shape->getHeight()-0.001); break;
    case 1: d->setValues(1, GE->size1, GE->size2, obj->getGridElement()->Shape->getHeight()-0.001); break;
    case 2:
        {
            AGeoPgon * pg = dynamic_cast<AGeoPgon*>(obj->getGridElement()->Shape);
            if (pg) d->setValues(2, GE->size1, GE->size2, obj->getGridElement()->Shape->getHeight()-0.001);
            break;
        }
    }

    //setting materials
    d->setBulkMaterial(obj->Material);
    if (!obj->HostedObjects.empty())
        if (!obj->HostedObjects.front()->HostedObjects.empty())
        {
            int wireMat = obj->HostedObjects.front()->HostedObjects.front()->Material;
            d->setWireMaterial(wireMat);
        }

    int res = d->exec();

    if (res != 0)
    {
        //qDebug() << "Accepted!";
        AGridHub & GridHub = AGridHub::getInstance();
        switch (d->shape())
        {
        case 0: GridHub.shapeGrid(obj, 0, d->pitch(),  d->length(), d->diameter(), d->wireMaterial()); break;
        case 1: GridHub.shapeGrid(obj, 1, d->pitchX(), d->pitchY(), d->diameter(), d->wireMaterial()); break;
        case 2: GridHub.shapeGrid(obj, 2, d->outer(),  d->inner(),  d->height(),   d->wireMaterial()); break;
        default:
            qWarning() << "Unknown grid type!";
        }

        obj->Material = d->bulkMaterial();

        emit RequestRebuildDetector();
        UpdateGui(objName);
    }
    //else qDebug() << "Rejected!";
    delete d;
}

void AGeoTree::populateTreeWidget(QTreeWidgetItem * parent, AGeoObject * Container, bool fDisabled)
{  
    for (AGeoObject * obj : Container->HostedObjects)
    {
        if (obj->Type->isPrototypeCollection()) continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(parent);

        bool fDisabledLocal = fDisabled || !obj->fActive;
        if (fDisabledLocal) item->setForeground(0, QBrush(Qt::red));

        item->setText(0, obj->Name);
        item->setSizeHint(0, QSize(50, 20));  // ?

        if (obj->Type->isWorld())
        {
            item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);// & ~Qt::ItemIsSelectable);
            QFont f = item->font(0); f.setBold(true); item->setFont(0, f);
        }
        else if (obj->Type->isInstance() && obj->fActive)
        {
            item->setForeground(0, Qt::blue);
            updateIcon(item, obj);
        }
        else if (obj->Type->isHandlingSet() || obj->Type->isHandlingArray() || obj->Type->isGridElement())
        { //group or stack or array or gridElement
            QFont f = item->font(0); f.setItalic(true); item->setFont(0, f);
            updateIcon(item, obj);
            //item->setBackgroundColor(0, BackgroundColor);
        }      
        else
        {
            updateIcon(item, obj);
            if (obj->isStackReference())
            {
                item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
                QFont f = item->font(0); f.setBold(true); item->setFont(0, f);
            }
            //item->setBackgroundColor(0, BackgroundColor);
        }      

        populateTreeWidget(item, obj, fDisabledLocal);
    }
}

void AGeoTree::updateExpandState(QTreeWidgetItem * item, bool bPrototypes)
{
    QTreeWidget * treeWidget = nullptr;
    AGeoObject  * obj        = nullptr;

    if (bPrototypes)
    {
        treeWidget = twPrototypes;
        obj        = ( item == topItemPrototypes ? Prototypes : Prototypes->findObjectByName(item->text(0)) );
    }
    else
    {
        treeWidget = twGeoTree;
        obj        = World->findObjectByName(item->text(0));
    }

    if (obj && obj->fExpanded)
    {
        treeWidget->expandItem(item);
        for (int i = 0; i < item->childCount(); i++)
            updateExpandState(item->child(i), bPrototypes);
    }
}

void AGeoTree::onItemSelectionChanged()
{
  //  qDebug() << "---Widget selection cghanged";
  QList<QTreeWidgetItem*> sel = twGeoTree->selectedItems();

  if      (sel.size() == 0) emit ObjectSelectionChanged("");
  else if (sel.size() == 1)
  {
      const QString name = sel.first()->text(0);
      emit ObjectSelectionChanged(name);
  }
  else emit ObjectSelectionChanged(""); //with multiple selected do not show EditWidget
}

void AGeoTree::onProtoItemSelectionChanged()
{
    QList<QTreeWidgetItem*> sel = twPrototypes->selectedItems();

    if      (sel.size() == 0) emit ProtoObjectSelectionChanged("");
    else if (sel.size() == 1)
    {
        const QString name = sel.first()->text(0);
        emit ProtoObjectSelectionChanged(name);
        return;
    }
    else
    {
        //allow to select only one prototype object
        //allow only selection of objects of the same container
        QTreeWidgetItem * FirstParent = sel.first()->parent();
        for (int i = 1; i < sel.size(); i++)
        {
            if (sel.at(i)->parent() != FirstParent)
            {
                qDebug() << "Cannot select together items from different containers!";
                sel.at(i)->setSelected(false);
                return; // will retrigger anyway
            }
            /*
            if (sel.at(i)->font(0).bold())
            {
                //qDebug() << "Cannot select together different slabs or    world and slab(s)";
                sel.at(i)->setSelected(false);
                return; // will retrigger anyway
            }
            */
        }
        emit ProtoObjectSelectionChanged(""); //with multiple selected do not show EditWidget
    }
}

QAction* Action(QMenu& Menu, QString Text)
{
  QAction* s = Menu.addAction(Text);
  s->setEnabled(false);
  return s;
}

void AGeoTree::customMenuRequested(const QPoint &pos)
{  
  QMenu menu;
  QList<QTreeWidgetItem*> selected = twGeoTree->selectedItems();

  QAction* focusObjA = Action(menu, "Show - focus geometry view");
  QAction* showA     = Action(menu, "Show - highlight in geometry");
  QAction* showAdown = Action(menu, "Show - this object with content");
  QAction* showAonly = Action(menu, "Show - only this object");
  QAction* lineA     = Action(menu, "Change line color/width/style");

  menu.addSeparator();

  QAction* enableDisableA = Action(menu, "Enable/Disable");

  menu.addSeparator();

  QMenu * addObjMenu = menu.addMenu("Add object");
    QAction* newBox  = addObjMenu->addAction("Box");
    QMenu * addTubeMenu = addObjMenu->addMenu("Tube");
        QAction* newTube =        addTubeMenu->addAction("Tube");
        QAction* newTubeSegment = addTubeMenu->addAction("Tube segment");
        QAction* newTubeSegCut =  addTubeMenu->addAction("Tube segment cut");
        QAction* newTubeElli =    addTubeMenu->addAction("Elliptical tube");
    QMenu * addTrapMenu = addObjMenu->addMenu("Trap");
        QAction* newTrapSim =     addTrapMenu->addAction("Trap (trapezoidal prizm)");
        QAction* newTrap    =     addTrapMenu->addAction("Trap2");
    QAction* newPcon = addObjMenu->addAction("Polycone");
    QMenu * addPgonMenu = addObjMenu->addMenu("Polygon");
        QAction* newPgonSim =     addPgonMenu->addAction("Polygon simplified");
        QAction* newPgon    =     addPgonMenu->addAction("Polygon");
    QAction* newPara = addObjMenu->addAction("Parallelepiped");
    QAction* newSphere = addObjMenu->addAction("Sphere");
    QMenu * addConeMenu = addObjMenu->addMenu("Cone");
        QAction* newCone =        addConeMenu->addAction("Cone");
        QAction* newConeSeg =     addConeMenu->addAction("Cone segment");
    QAction* newTor = addObjMenu->addAction("Torus");
    QAction* newParabol = addObjMenu->addAction("Paraboloid");
    QAction* newArb8 = addObjMenu->addAction("Arb8");

  QAction* newArrayA  = Action(menu, "Add array");
  QAction* newCircArrayA  = Action(menu, "Add circular array");
  QAction* newHexArrayA  = Action(menu, "Add hexagonal array");
  QAction* newCompositeA  = Action(menu, "Add composite object");
  QAction* newGridA = Action(menu, "Add optical grid");

  QMenu * addMonitorMenu = menu.addMenu("Add monitor"); addMonitorMenu->setEnabled(false);
    QAction* newPhotonMonitorA   = addMonitorMenu->addAction("Optical photons");
    QAction* newParticleMonitorA = addMonitorMenu->addAction("Particles");

  menu.addSeparator();

  QMenu * addInstanceMenu = menu.addMenu("Add instance of");
    QVector< QPair<QAction*, QString> > addInstanceA;
    if (Prototypes->HostedObjects.empty())
        Action(*addInstanceMenu, "There are no defined prototypes");
    else
        for (AGeoObject * protoObj : Prototypes->HostedObjects)
            addInstanceA << QPair<QAction*, QString>(addInstanceMenu->addAction(protoObj->Name), protoObj->Name);

  menu.addSeparator();

  QAction* cloneA = Action(menu, "Clone this object");

  menu.addSeparator();

  QAction* removeWithContA = Action(menu, "Remove object AND hosted");
  removeWithContA->setShortcut(QKeySequence(QKeySequence::Delete));
  QAction* removeKeepContA = Action(menu, "Remove object, KEEP hosted");
  removeKeepContA->setShortcut(Qt::Key_Backspace);
  QAction* removeHostedA = Action(menu, "Remove all hosted objects");

  menu.addSeparator();

  QAction* stackA = Action(menu, "Form a stack");
  QAction* stackRefA = Action(menu, "Mark as the stack reference volume");

  menu.addSeparator();

  QAction* prototypeA = Action(menu, QString("Make prototype (move object%1)").arg(selected.size()>1 ? "s" : ""));


  // enable actions according to selection
  QString objName;
  AGeoObject * obj = nullptr;
  if      (selected.size() == 0)
  {
      objName = "World";
      obj = World;
  }
  else if (selected.size() == 1)
  {
      objName = selected.first()->text(0);
      obj = World->findObjectByName(objName);
      if (!obj) return;
      const AGeoType & Type = *obj->Type;

      bool fNotGridNotMonitor = !Type.isGrid() && !Type.isMonitor();

      addObjMenu->setEnabled(fNotGridNotMonitor);
      enableDisableA->setEnabled( !obj->isWorld() );
      enableDisableA->setText( (obj->isDisabled() ? "Enable object" : "Disable object" ) );

      newCompositeA->setEnabled(fNotGridNotMonitor);
      newArrayA->setEnabled(fNotGridNotMonitor);
      newCircArrayA->setEnabled(fNotGridNotMonitor);
      newHexArrayA->setEnabled(fNotGridNotMonitor);
      addMonitorMenu->setEnabled(fNotGridNotMonitor);
      newGridA->setEnabled(fNotGridNotMonitor);
      cloneA->setEnabled(true);
      removeHostedA->setEnabled(fNotGridNotMonitor);
      removeWithContA->setEnabled(!Type.isWorld());
      removeKeepContA->setEnabled(!Type.isWorld());
      lineA->setEnabled(true);
      focusObjA->setEnabled(true);
      showA->setEnabled(true);
      showAonly->setEnabled(true);
      showAdown->setEnabled(true);
      stackRefA->setEnabled(obj->isStackMember());
      prototypeA->setEnabled(obj->isPossiblePrototype());
  }
  else
  {
      addObjMenu->setEnabled(false);
      addInstanceMenu->setEnabled(false);
      removeWithContA->setEnabled(true);
      stackA->setEnabled(true);
      prototypeA->setEnabled(true);
  }

  if (!obj || !obj->isGoodContainerForInstance()) addInstanceMenu->setEnabled(false);

  QAction* SelectedAction = menu.exec(twGeoTree->mapToGlobal(pos));
  if (!SelectedAction) return;

  // -- EXECUTE SELECTED ACTION --
  if (SelectedAction == focusObjA)  // FOCUS OBJECT
  {
      emit RequestFocusObject(objName);
      UpdateGui(objName);
  }
  if (SelectedAction == showA)               ShowObject(obj);
  else if (SelectedAction == showAonly)      ShowObjectOnly(obj);
  else if (SelectedAction == showAdown)      ShowObjectRecursive(obj);
  else if (SelectedAction == lineA)          SetLineAttributes(obj);
  else if (SelectedAction == enableDisableA) menuActionEnableDisable(obj);
  // ADD NEW OBJECT
  else if (SelectedAction == newBox)         menuActionAddNewObject(obj, new AGeoBox());
  else if (SelectedAction == newTube)        menuActionAddNewObject(obj, new AGeoTube());
  else if (SelectedAction == newTubeSegment) menuActionAddNewObject(obj, new AGeoTubeSeg());
  else if (SelectedAction == newTubeSegCut)  menuActionAddNewObject(obj, new AGeoCtub());
  else if (SelectedAction == newTubeElli)    menuActionAddNewObject(obj, new AGeoEltu());
  else if (SelectedAction == newTrapSim)     menuActionAddNewObject(obj, new AGeoTrd1());
  else if (SelectedAction == newTrap)        menuActionAddNewObject(obj, new AGeoTrd2());
  else if (SelectedAction == newPcon)        menuActionAddNewObject(obj, new AGeoPcon());
  else if (SelectedAction == newPgonSim)     menuActionAddNewObject(obj, new AGeoPolygon());
  else if (SelectedAction == newPgon)        menuActionAddNewObject(obj, new AGeoPgon());
  else if (SelectedAction == newPara)        menuActionAddNewObject(obj, new AGeoPara());
  else if (SelectedAction == newSphere)      menuActionAddNewObject(obj, new AGeoSphere());
  else if (SelectedAction == newCone)        menuActionAddNewObject(obj, new AGeoCone());
  else if (SelectedAction == newConeSeg)     menuActionAddNewObject(obj, new AGeoConeSeg());
  else if (SelectedAction == newTor)         menuActionAddNewObject(obj, new AGeoTorus());
  else if (SelectedAction == newParabol)     menuActionAddNewObject(obj, new AGeoParaboloid());
  else if (SelectedAction == newArb8)        menuActionAddNewObject(obj, new AGeoArb8());
  else if (SelectedAction == newCompositeA)  menuActionAddNewComposite(obj);
  else if (SelectedAction == newArrayA)      menuActionAddNewArray(obj);
  else if (SelectedAction == newCircArrayA)  menuActionAddNewCircularArray(obj);
  else if (SelectedAction == newHexArrayA)   menuActionAddNewHexagonalArray(obj);
  else if (SelectedAction == newGridA)       menuActionAddNewGrid(obj);
  else if (SelectedAction == newPhotonMonitorA)    menuActionAddNewMonitor(obj, true);
  else if (SelectedAction == newParticleMonitorA)  menuActionAddNewMonitor(obj, false);
  else if (SelectedAction == cloneA)         menuActionCloneObject(obj);
  else if (SelectedAction == stackA)         menuActionFormStack(selected);
  else if (SelectedAction == stackRefA)      markAsStackRefVolume(obj);
  else if (SelectedAction == removeKeepContA)menuActionRemoveKeepContent(twGeoTree);
  else if (SelectedAction == removeWithContA)menuActionRemoveWithContent(twGeoTree);
  else if (SelectedAction == removeHostedA)  menuActionRemoveHostedObjects(obj);

  else if (SelectedAction == prototypeA)     menuActionMakeItPrototype(selected);

  else
  {
      for (auto & pair : addInstanceA)
          if (SelectedAction == pair.first)  menuActionAddInstance(obj, pair.second);
  }
}

void AGeoTree::customProtoMenuRequested(const QPoint & pos)
{
    // top level (Prototypes) can have only single selection (see onProtoSelectionChanged())
    QList<QTreeWidgetItem*> selected = twPrototypes->selectedItems();
    if (selected.isEmpty())
    {
        protoMenuEmptySelection(pos);
        return;
    }

    // non-empty selection is assumed below!
    QMenu menu;

    QAction* showAllA  = Action(menu, "Show all instances");
    QAction* lineA     = Action(menu, "Change line color/width/style");

    menu.addSeparator();

    QAction* enableDisableA = Action(menu, "Enable/Disable");

    menu.addSeparator();

    QMenu * addObjMenu = menu.addMenu("Add object");
      QAction* newBox  = addObjMenu->addAction("Box");
      QMenu * addTubeMenu = addObjMenu->addMenu("Tube");
          QAction* newTube =        addTubeMenu->addAction("Tube");
          QAction* newTubeSegment = addTubeMenu->addAction("Tube segment");
          QAction* newTubeSegCut =  addTubeMenu->addAction("Tube segment cut");
          QAction* newTubeElli =    addTubeMenu->addAction("Elliptical tube");
      QMenu * addTrapMenu = addObjMenu->addMenu("Trap");
          QAction* newTrapSim =     addTrapMenu->addAction("Trap (trapezoidal prizm");
          QAction* newTrap    =     addTrapMenu->addAction("Trap2");
      QAction* newPcon = addObjMenu->addAction("Polycone");
      QMenu * addPgonMenu = addObjMenu->addMenu("Polygon");
          QAction* newPgonSim =     addPgonMenu->addAction("Polygon simplified");
          QAction* newPgon    =     addPgonMenu->addAction("Polygon");
      QAction* newPara = addObjMenu->addAction("Parallelepiped");
      QAction* newSphere = addObjMenu->addAction("Sphere");
      QMenu * addConeMenu = addObjMenu->addMenu("Cone");
          QAction* newCone =        addConeMenu->addAction("Cone");
          QAction* newConeSeg =     addConeMenu->addAction("Cone segment");
      QAction* newTor = addObjMenu->addAction("Torus");
      QAction* newParabol = addObjMenu->addAction("Paraboloid");
      QAction* newArb8 = addObjMenu->addAction("Arb8");

    QAction* newArrayA  = Action(menu, "Add array");
    QAction* newCompositeA  = Action(menu, "Add composite object");
    //QAction* newGridA = Action(menu, "Add optical grid");

    QMenu * addMonitorMenu = menu.addMenu("Add monitor"); addMonitorMenu->setEnabled(false);
      QAction* newPhotonMonitorA   = addMonitorMenu->addAction("Optical photons");
      QAction* newParticleMonitorA = addMonitorMenu->addAction("Particles");

    menu.addSeparator();

    QAction* cloneA = Action(menu, "Clone this object");

    menu.addSeparator();

    QAction* removeWithContA = Action(menu, "Remove object and content");
    QAction* removeKeepContA = Action(menu, "Remove object, keep its content");
    QAction* removeHostedA = Action(menu, "Remove all objects inside");

    menu.addSeparator();

    QAction* stackA = Action(menu, "Form a stack");
    QAction* stackRefA = Action(menu, "Mark as the stack reference volume");

    menu.addSeparator();

    QAction* moveToWorldA = Action(menu, "Move to World");

    // selection is not empty!
    QString objName = selected.first()->text(0);
    AGeoObject * obj = Prototypes->findObjectByName(objName);
    if (!obj) return;
    const AGeoType & Type = *obj->Type;
    const bool bNotGridNotMonitor = !Type.isGrid() && !Type.isMonitor();
    const bool bIsPrototype = Type.isPrototype();

    if (selected.size() == 1)
    {
        showAllA->setEnabled(bIsPrototype);
        lineA->setEnabled(!bIsPrototype);
        enableDisableA->setEnabled(!obj->isWorld() && !bIsPrototype);
        enableDisableA->setText( (obj->isDisabled() ? "Enable object" : "Disable object" ) );
        addObjMenu->setEnabled(bNotGridNotMonitor);
        newCompositeA->setEnabled(bNotGridNotMonitor);
        newArrayA->setEnabled(bNotGridNotMonitor);
        addMonitorMenu->setEnabled(bNotGridNotMonitor);
        cloneA->setEnabled(true);
        removeHostedA->setEnabled(bNotGridNotMonitor);
        removeWithContA->setEnabled(true);
        removeKeepContA->setEnabled(!bIsPrototype);
        stackRefA->setEnabled(obj->isStackMember());
        moveToWorldA->setEnabled(bIsPrototype);
    }
    else
    {
        addObjMenu->setEnabled(false);
        removeWithContA->setEnabled(true);
        stackA->setEnabled(!bIsPrototype);
    }

    QAction * SelectedAction = menu.exec(twPrototypes->mapToGlobal(pos));
    if (!SelectedAction) return; //nothing was selected

    // -- EXECUTE SELECTED ACTION --
    //if (SelectedAction == showA)               ShowObject(obj); else
    if (SelectedAction == showAllA)            ShowAllInstances(obj);
    else if (SelectedAction == lineA)          SetLineAttributes(obj);
    else if (SelectedAction == enableDisableA) menuActionEnableDisable(obj);
    // ADD NEW OBJECT
    else if (SelectedAction == newBox)         menuActionAddNewObject(obj, new AGeoBox());
    else if (SelectedAction == newTube)        menuActionAddNewObject(obj, new AGeoTube());
    else if (SelectedAction == newTubeSegment) menuActionAddNewObject(obj, new AGeoTubeSeg());
    else if (SelectedAction == newTubeSegCut)  menuActionAddNewObject(obj, new AGeoCtub());
    else if (SelectedAction == newTubeElli)    menuActionAddNewObject(obj, new AGeoEltu());
    else if (SelectedAction == newTrapSim)     menuActionAddNewObject(obj, new AGeoTrd1());
    else if (SelectedAction == newTrap)        menuActionAddNewObject(obj, new AGeoTrd2());
    else if (SelectedAction == newPcon)        menuActionAddNewObject(obj, new AGeoPcon());
    else if (SelectedAction == newPgonSim)     menuActionAddNewObject(obj, new AGeoPolygon());
    else if (SelectedAction == newPgon)        menuActionAddNewObject(obj, new AGeoPgon());
    else if (SelectedAction == newPara)        menuActionAddNewObject(obj, new AGeoPara());
    else if (SelectedAction == newSphere)      menuActionAddNewObject(obj, new AGeoSphere());
    else if (SelectedAction == newCone)        menuActionAddNewObject(obj, new AGeoCone());
    else if (SelectedAction == newConeSeg)     menuActionAddNewObject(obj, new AGeoConeSeg());
    else if (SelectedAction == newTor)         menuActionAddNewObject(obj, new AGeoTorus());
    else if (SelectedAction == newParabol)     menuActionAddNewObject(obj, new AGeoParaboloid());
    else if (SelectedAction == newArb8)        menuActionAddNewObject(obj, new AGeoArb8());
    else if (SelectedAction == newCompositeA)  menuActionAddNewComposite(obj);
    else if (SelectedAction == newArrayA)      menuActionAddNewArray(obj);
    //else if (SelectedAction == newGridA)       menuActionAddNewGrid(obj);
    else if (SelectedAction == newPhotonMonitorA)   menuActionAddNewMonitor(obj, true);
    else if (SelectedAction == newParticleMonitorA) menuActionAddNewMonitor(obj, false);

    else if (SelectedAction == cloneA)         menuActionCloneObject(obj);
    else if (SelectedAction == stackA)         menuActionFormStack(selected);
    else if (SelectedAction == stackRefA)      markAsStackRefVolume(obj);

    else if (SelectedAction == removeKeepContA)menuActionRemoveKeepContent(twPrototypes);
    else if (SelectedAction == removeWithContA)menuActionRemoveWithContent(twPrototypes);
    else if (SelectedAction == removeHostedA)  menuActionRemoveHostedObjects(obj);

    else if (SelectedAction == moveToWorldA)   menuActionMoveProtoToWorld(obj);
}

void AGeoTree::protoMenuEmptySelection(const QPoint & pos)
{
    QMenu menu;
    menu.addAction("Create new prototype");

    QAction * SelectedAction = menu.exec(twPrototypes->mapToGlobal(pos));
    if (!SelectedAction) return;

    const QString name = Geometry.generateObjectName("Prototype");
    AGeoObject * proto = new AGeoObject(name, nullptr);

    delete proto->Type; proto->Type = new ATypePrototypeObject();
    proto->migrateTo(Prototypes);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::onItemClicked()
{
    if (fSpecialGeoViewMode)
    {
          fSpecialGeoViewMode = false;
          emit RequestNormalDetectorDraw();
    }
    if (!bWorldTreeSelected) onItemSelectionChanged();
    bWorldTreeSelected = true;
}

void AGeoTree::onProtoItemClicked()
{
    if (fSpecialGeoViewMode)
    {
          fSpecialGeoViewMode = false;
          emit RequestNormalDetectorDraw();
    }
    if (bWorldTreeSelected) onProtoItemSelectionChanged();
    bWorldTreeSelected = false;
}

void AGeoTree::onItemExpanded(QTreeWidgetItem *item)
{
    AGeoObject * obj = World->findObjectByName(item->text(0));
    if (obj) obj->fExpanded = true;
}

void AGeoTree::onItemCollapsed(QTreeWidgetItem *item)
{
    AGeoObject * obj = World->findObjectByName(item->text(0));
    if (obj) obj->fExpanded = false;
}

void AGeoTree::onPrototypeItemExpanded(QTreeWidgetItem * item)
{
    AGeoObject * obj = ( item == twPrototypes->topLevelItem(0) ? Prototypes
                                                               : Prototypes->findObjectByName(item->text(0)) );
    if (obj) obj->fExpanded = true;
}

void AGeoTree::onPrototypeItemCollapsed(QTreeWidgetItem * item)
{
    AGeoObject * obj = ( item == twPrototypes->topLevelItem(0) ? Prototypes
                                                               : Prototypes->findObjectByName(item->text(0)) );
    if (obj) obj->fExpanded = false;
}

void AGeoTree::onRemoveTriggered()
{
    menuActionRemoveKeepContent(twGeoTree);
}

void AGeoTree::onRemoveRecursiveTriggered()
{
    menuActionRemoveWithContent(twGeoTree);
}

void AGeoTree::onRequestShowPrototype(QString ProtoName)
{
    emit RequestShowPrototypeList();

    QList<QTreeWidgetItem*> list = twPrototypes->findItems(ProtoName, Qt::MatchExactly | Qt::MatchRecursive);
    if (!list.isEmpty())
    {
        list.first()->setSelected(true);
        twPrototypes->setCurrentItem(list.first());
    }
}

void AGeoTree::onRequestIsValidPrototypeName(const QString &ProtoName, bool &bResult) const
{
    bResult = Geometry.isValidPrototypeName(ProtoName);
}

void AGeoTree::menuActionRemoveKeepContent(QTreeWidget * treeWidget)
{
  QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
  if (selected.isEmpty()) return;

  QMessageBox msgBox(treeWidget);
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setWindowTitle("Remove but keep content");
  QString str = ( selected.size() == 1 ? "Remove "+selected.first()->text(0)+"?"
                                       : "Remove selected objects?" );
  //str += "                                             ";
  msgBox.setText(str);
  QPushButton *remove = msgBox.addButton(QMessageBox::Yes);
  QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
  msgBox.setDefaultButton(remove);

  msgBox.exec();

  QString failedDeletes;
  if (msgBox.clickedButton() == remove)
  {
      for (int i=0; i<selected.size(); i++)
      {
          QString ObjectName = selected.at(i)->text(0);
          AGeoObject* obj = World->findObjectByName(ObjectName);
          if (obj)
          {
              if (  obj->isInUseByComposite() ||
                   (obj->Type->isPrototype() && World->isPrototypeInUseRecursive(obj->Name, nullptr)) )
              {
                  failedDeletes += "  " + obj->Name;
                  continue;
              }
              obj->suicide();
          }
      }

      if (!failedDeletes.isEmpty())
          guitools::message("The following objects are in use and could not be deleted:\n" + failedDeletes);

      emit RequestRebuildDetector();
  }
}

void AGeoTree::menuActionRemoveWithContent(QTreeWidget * treeWidget)
{
    QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
    if (selected.isEmpty()) return;

    QMessageBox msgBox(treeWidget->parentWidget());
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Remove with content");
    QString str = ( selected.size() == 1 ? "Remove " + selected.first()->text(0) + "?"
                                         : "Remove selected objects?" );
    msgBox.setText(str);
    QPushButton *remove = msgBox.addButton(QMessageBox::Yes);
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(remove);

    msgBox.exec();

    QString failedDeletes;
    if (msgBox.clickedButton() == remove)
    {
        for (QTreeWidgetItem * item : selected)
        {
            AGeoObject * obj = World->findObjectByName(item->text(0));
            if (obj)
            {
                if (Geometry.canBeDeleted(obj)) obj->recursiveSuicide();
                else failedDeletes += obj->Name + "\n";
            }
        }
    }

    emit RequestRebuildDetector();

    if (!failedDeletes.isEmpty())
        guitools::message("The following objects are in use and could not be deleted:\n\n" + failedDeletes, treeWidget);
}

void AGeoTree::menuActionRemoveHostedObjects(AGeoObject * obj)
{
    if (!obj) return;

    QMessageBox msgBox(twGeoTree);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Remove content");
    msgBox.setText("Remove objects hosted inside " + obj->Name + "?");
    QPushButton * yes    = msgBox.addButton(QMessageBox::Yes);
    QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(yes);

    msgBox.exec();
    if (msgBox.clickedButton() == cancel) return;

    for (int i = obj->HostedObjects.size()-1; i > -1; i--)
        obj->HostedObjects[i]->recursiveSuicide();

    const QString name = obj->Name;
    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionCloneObject(AGeoObject * obj)
{
    if (!obj) return;
    if (obj->Type->isWorld()) return;

    AGeoObject * clone = obj->makeClone(World);
    if (!clone)
    {
        guitools::message("Failed to clone object " + obj->Name);
        return;
    }

    if (clone->PositionStr[2].isEmpty()) clone->Position[2] += 10.0;
    else clone->PositionStr[2] += " + 10";

    AGeoObject * container = obj->Container;
    if (!container) container = World;
    container->addObjectFirst(clone);  //inserts to the first position in the list of HostedObjects!
    clone->repositionInHosted(obj, true);

    const QString name = clone->Name;
    emit RequestRebuildDetector();
    emit RequestHighlightObject(name);
}

void AGeoTree::menuActionAddNewObject(AGeoObject * contObj, AGeoShape * shape)
{
    if (!contObj) return;

    const QString objName = Geometry.generateStandaloneObjectName(shape);
    AGeoObject * newObj = new AGeoObject(objName);

    delete newObj->Shape; newObj->Shape = shape;

    newObj->color = 1;

    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    emit RequestRebuildDetector();
    UpdateGui(objName);
}

void AGeoTree::menuActionAddNewArray(AGeoObject * contObj)
{
    if (!contObj) return;

    const QString name = Geometry.generateObjectName("Array");
    AGeoObject * newObj = new AGeoObject(name, nullptr);

    delete newObj->Type; newObj->Type = new ATypeArrayObject();

    newObj->color = 1;

    //contObj->addObjectFirst(newObj);  //inserts to the first position in the list of HostedObjects!
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    //element inside
    AGeoBox * shape = new AGeoBox();
    const QString elName = Geometry.generateStandaloneObjectName(shape);
    AGeoObject * elObj = new AGeoObject(elName, shape);
    elObj->color = 1;
    newObj->addObjectFirst(elObj);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionAddNewCircularArray(AGeoObject * contObj)
{
    if (!contObj) return;

    const QString name = Geometry.generateObjectName("CircArray");
    AGeoObject * newObj = new AGeoObject(name, nullptr);

    delete newObj->Type; newObj->Type = new ATypeCircularArrayObject();

    newObj->color = 1;

    //contObj->addObjectFirst(newObj);  //inserts to the first position in the list of HostedObjects!
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    //element inside
    AGeoBox * shape = new AGeoBox();
    const QString elName = Geometry.generateStandaloneObjectName(shape);
    AGeoObject * elObj = new AGeoObject(elName, shape);
    elObj->color = 1;
    newObj->addObjectFirst(elObj);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionAddNewHexagonalArray(AGeoObject * contObj)
{
    if (!contObj) return;

    const QString name = Geometry.generateObjectName("HexArray");
    AGeoObject * newObj = new AGeoObject(name, nullptr);

    delete newObj->Type; newObj->Type = new ATypeHexagonalArrayObject();

    newObj->color = 1;

    //contObj->addObjectFirst(newObj);  //inserts to the first position in the list of HostedObjects!
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    //element inside
    AGeoBox * shape = new AGeoBox();
    const QString elName = Geometry.generateStandaloneObjectName(shape);
    AGeoObject * elObj = new AGeoObject(elName, shape);
    elObj->color = 1;
    newObj->addObjectFirst(elObj);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

#include "agridhub.h"
void AGeoTree::menuActionAddNewGrid(AGeoObject * contObj)
{
    if (!contObj) return;

    AGeoObject * newObj = new AGeoObject();
    newObj->Name = Geometry.generateObjectName("Grid");
    delete newObj->Shape; newObj->Shape = new AGeoBox(50.0, 50.0, 0.501);
    newObj->Material = contObj->Material;

    newObj->color = 1;

    //contObj->addObjectFirst(newObj);
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    AGridHub::getInstance().convertObjToGrid(newObj);

    const QString name = newObj->Name;
    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionAddNewMonitor(AGeoObject * contObj, bool isPhoton)
{
    if (!contObj) return;

    const QString name = Geometry.generateObjectName("Monitor");
    AGeoObject * newObj = new AGeoObject(name, nullptr);

    newObj->Material = contObj->Material;

    delete newObj->Type; newObj->Type = new ATypeMonitorObject();
    static_cast<ATypeMonitorObject*>(newObj->Type)->config.PhotonOrParticle = (isPhoton ? 0 : 1);

    //newObj->updateMonitorShape();

    newObj->color = 1;

    //contObj->addObjectFirst(newObj);
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionAddInstance(AGeoObject * contObj, const QString & prototypeName)
{
    if (!contObj) return;
    if (!contObj->isGoodContainerForInstance()) return;

    AGeoObject * protoObj = Prototypes->findObjectByName(prototypeName);
    if (!protoObj)
    {
        guitools::message("Something went very wrong: prototype not found", twGeoTree);
        return;
    }

    const QString name = Geometry.generateObjectName(prototypeName + "_Inst");
    AGeoObject * newObj = new AGeoObject(name, nullptr);

    delete newObj->Type; newObj->Type = new ATypeInstanceObject(prototypeName);

    for (int i = 0; i < 3; i++)
    {
        newObj->Position[i]       = protoObj->Position[i];
        newObj->PositionStr[i]    = protoObj->PositionStr[i];
        newObj->Orientation[i]    = protoObj->Orientation[i];
        newObj->OrientationStr[i] = protoObj->OrientationStr[i];
    }

    //contObj->addObjectFirst(newObj);
    if (A3Global::getInstance().NewGeoObjectAddedLast)
        contObj->addObjectLast(newObj);
    else
        contObj->addObjectFirst(newObj);

    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionMakeItPrototype(const QList<QTreeWidgetItem*> & selected)
{
    std::vector<AGeoObject*> vec;
    for (const QTreeWidgetItem * item : selected)
    {
        AGeoObject * obj = World->findObjectByName(item->text(0));
        if (!obj)
        {
            guitools::message("Something went wrong: object not found", twGeoTree);
            return;
        }
        vec.push_back(obj);
    }
    QString err = Geometry.convertToNewPrototype(vec);
    if (!err.isEmpty())
    {
        guitools::message(err, twGeoTree);
        return;
    }

    const QString name = "";//obj->Name;
    emit RequestRebuildDetector();
    UpdateGui(name);
    emit RequestShowPrototypeList();
}

void AGeoTree::menuActionMoveProtoToWorld(AGeoObject * obj)
{
    if (!obj || !obj->Type->isPrototype()) return;

    QStringList users;
    bool bIsUsed = World->isPrototypeInUseRecursive(obj->Name, &users);
    if (bIsUsed)
    {
        guitools::message("The prototype is in used by these instances(s):\n   " + users.join("\n   "), twGeoTree);
        return;
    }

    while (!obj->HostedObjects.empty())
    {
        AGeoObject * hosted = obj->HostedObjects.front();
        hosted->migrateTo(World, true);
    }
    Prototypes->removeHostedObject(obj);
    delete obj;

    emit RequestRebuildDetector();
}

void AGeoTree::menuActionAddNewComposite(AGeoObject * contObj)
{
  if (!contObj) return;

  const QString name = Geometry.generateObjectName("Composite");
  AGeoObject * newObj = new AGeoObject(name, nullptr);

  newObj->color = 1;

  //contObj->addObjectFirst(newObj);  //inserts to the first position in the list of HostedObjects!
  if (A3Global::getInstance().NewGeoObjectAddedLast)
      contObj->addObjectLast(newObj);
  else
      contObj->addObjectFirst(newObj);

  Geometry.convertObjToComposite(newObj);

  emit RequestRebuildDetector();
  UpdateGui(name);
}

void AGeoTree::SetLineAttributes(AGeoObject * obj)
{
    if (!obj) return;

    ARootLineConfigurator* rlc = new ARootLineConfigurator(&obj->color, &obj->width, &obj->style, twGeoTree);
    int res = rlc->exec();
    if (res != 0)
    {
        if (obj->Type->isHandlingArray() || obj->Type->isHandlingSet())
        {
            std::vector<AGeoObject*> vec;
            obj->collectContainingObjects(vec);
            for (AGeoObject * co : vec)
            {
                co->color = obj->color;
                co->width = obj->width;
                co->style = obj->style;
            }
        }
        const QString name = obj->Name;
        emit RequestRebuildDetector();
        UpdateGui(name);
    }
}

void AGeoTree::ShowObject(AGeoObject * obj)
{
    if (obj)
    {
        fSpecialGeoViewMode = true;
        emit RequestHighlightObject(obj->Name);
        UpdateGui(obj->Name);
    }
}

void AGeoTree::ShowObjectRecursive(AGeoObject * obj)
{
    if (obj)
    {
        fSpecialGeoViewMode = true;
        emit RequestShowObjectRecursive(obj->Name);
        UpdateGui(obj->Name);
    }
}

void AGeoTree::ShowObjectOnly(AGeoObject * obj)
{
    if (obj)
    {
        fSpecialGeoViewMode = true;
        TGeoShape * sh = obj->Shape->createGeoShape();  // make window member?   !!!*** register!
        sh->Draw();
    }
}

void AGeoTree::ShowAllInstances(AGeoObject * obj)
{
    if (obj)
    {
        fSpecialGeoViewMode = true;
        emit RequestShowAllInstances(obj->Name);
        UpdateGui(obj->Name);
    }
}

void AGeoTree::menuActionEnableDisable(AGeoObject * obj)
{
    if (!obj) return;

    if (obj->isDisabled())
        obj->enableUp();
    else
    {
        if (obj->isStackReference())
        {
            guitools::message("Cannot disable stack reference object!", EditWidget);
            return;
        }
        obj->fActive = false;
    }

    obj->fExpanded = obj->fActive;

    const QString name = obj->Name;
    emit RequestRebuildDetector();
    UpdateGui(name);
}

void AGeoTree::menuActionFormStack(QList<QTreeWidgetItem*> selected)
{
    if (selected.isEmpty()) return;

    std::vector<AGeoObject*> objs;
    AGeoObject * ContObj = nullptr;
    for (QTreeWidgetItem * item : selected)
    {
        AGeoObject * obj  = World->findObjectByName(item->text(0));
        if (!obj)
        {
            guitools::message("Something went wrong: object with name " + item->text(0) + " not found!", twGeoTree);
            return;
        }
        if (obj->Type->isWorld())
        {
            guitools::message("World cannot be a member of a stack", twGeoTree);
            return;
        }
        if (obj->Type->isHandlingArray())
        {
            guitools::message("Array cannot be a member of a stack", twGeoTree);
            return;
        }
        if (obj->Type->isComposite() || obj->Type->isGrid())
        {
            guitools::message("Composite objects (and optical grids) cannot be a member of a stack", twGeoTree);
            return;
        }
        if (obj->Type->isHandlingSet() || obj->Type->isLogical())
        {
            guitools::message("Stacks/groups cannot be a member of a stack", twGeoTree);
            return;
        }
        if (obj->Type->isPrototype() || obj->Type->isInstance())
        {
            guitools::message("Prototypes and instances cannot be a member of a stack", twGeoTree);
            return;
        }
        if (!ContObj) ContObj = obj->Container;
        if (ContObj != obj->Container)
        {
            guitools::message("To form a stack all objects have to have the same container", twGeoTree);
            return;
        }

        objs.push_back(obj);
    }

    const QString name = Geometry.generateObjectName("Stack");
    AGeoObject * stackObj = new AGeoObject(name, nullptr);

    delete stackObj->Type; stackObj->Type = new ATypeStackContainerObject();

    static_cast<ATypeStackContainerObject*>(stackObj->Type)->ReferenceVolume = objs.front()->Name;

    AGeoObject * contObj = objs.front()->Container; // All selected objects always have the same container!
    stackObj->Container = contObj;

    for (AGeoObject * obj : objs)
    {
        contObj->removeHostedObject(obj);
        obj->Container = stackObj;
        stackObj->HostedObjects.push_back(obj);
    }
    contObj->HostedObjects.insert(contObj->HostedObjects.begin(), stackObj);

    emit RequestRebuildDetector();  // automatically calculates stack positions there
    UpdateGui(name);
}

void AGeoTree::markAsStackRefVolume(AGeoObject * obj)
{
    if (!obj)
    {
        qWarning() << "Attempting to set nullptr as the stack reference!";
        return;
    }
    if (!obj->Container) return;
    if (!obj->Container->Type) return;
    ATypeStackContainerObject * sc = dynamic_cast<ATypeStackContainerObject*>(obj->Container->Type);
    if (!sc) return;
    const QString name = obj->Name;
    sc->ReferenceVolume = name;

    emit RequestRebuildDetector();
    UpdateGui(name);
}

QImage createImageWithOverlay(const QImage& base, const QImage& overlay)
{
    QImage imageWithOverlay = QImage(overlay.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&imageWithOverlay);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(imageWithOverlay.rect(), Qt::transparent);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, base);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, overlay);

    painter.end();

    return imageWithOverlay;
}

void AGeoTree::updateIcon(QTreeWidgetItem* item, AGeoObject *obj)
{  
  if (!obj || !item) return;

  QImage image;

  AGeoObject* cont = obj->Container;
  if (cont && !cont->HostedObjects.empty())
  {
      if (cont->Type->isStack())
      {
          if (obj == cont->HostedObjects.front())
              image = StackStart;
          else if (obj == cont->HostedObjects.back())
              image = StackEnd;
          else
              image = StackMid;
      }
  }

  if (obj->fLocked)
    {
      if (image.isNull())
        image = Lock;
      else
        image = createImageWithOverlay(Lock, image);
    }

  QIcon icon = QIcon(QPixmap::fromImage(image));
  item->setIcon(0, icon); 
}

void AGeoTree::rebuildDetectorAndRestoreCurrentDelegate()
{
    const QString CurrentObjName = EditWidget->getCurrentObjectName();
    emit RequestRebuildDetector();
    UpdateGui(CurrentObjName);
}
