#include "ageotreewin.h"
#include "ui_ageotreewin.h"
#include "ageometryhub.h"
#include "ageoconsts.h"
#include "amaterialhub.h"
#include "mainwindow.h"
#include "ageotree.h"
#include "ageobasetreewidget.h"
#include "ageodelegatewidget.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "a3global.h"
#include "ageometrytester.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "guitools.h"
#include "alineeditwithescape.h"
#include "aerrorhub.h"
#include "aonelinetextedit.h"
#include "ageobasedelegate.h"
#include "amonitorhub.h"
#include "amonitor.h"
#include "acalorimeterhub.h"
#include "acalorimeter.h"

#include <QDebug>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QMenu>
#include <QTabWidget>

#include "TGeoManager.h"
#include "TGeoTrack.h"
#include "TVirtualGeoTrack.h"
#include "TColor.h"
#include "TROOT.h"

AGeoTreeWin::AGeoTreeWin(QWidget * parent) :
  AGuiWindow("GeoTree", parent),
  Geometry(AGeometryHub::getInstance()),
  MaterialHub(AMaterialHub::getConstInstance()),
  ui(new Ui::AGeoTreeWin)
{
  ui->setupUi(this);

  // world tree widget
  twGeo = new AGeoTree();
  ui->saGeo->setWidget(twGeo->twGeoTree);
  connect(twGeo, &AGeoTree::RequestShowMonitor, this, &AGeoTreeWin::onRequestShowMonitorActiveDirection);

  // prototype tree widget
  ui->saPrototypes->setWidget(twGeo->twPrototypes);

  // Object editor
  QVBoxLayout* l = new QVBoxLayout();
  l->setContentsMargins(0,0,0,0);
  ui->frObjectEditor->setLayout(l);
  l->addWidget(twGeo->GetEditWidget());

  connect(twGeo, &AGeoTree::RequestRebuildDetector, this, &AGeoTreeWin::onRebuildDetectorRequest);
  connect(twGeo, &AGeoTree::RequestFocusObject,     this, &AGeoTreeWin::FocusVolume);
  connect(twGeo, &AGeoTree::RequestHighlightObject, this, &AGeoTreeWin::ShowObject);
  connect(twGeo, &AGeoTree::RequestShowObjectRecursive, this, &AGeoTreeWin::ShowObjectRecursive);
  connect(twGeo, &AGeoTree::RequestShowAllInstances, this, &AGeoTreeWin::showAllInstances);
  connect(twGeo->GetEditWidget(), &AGeoDelegateWidget::requestEnableGeoConstWidget, this, &AGeoTreeWin::onRequestEnableGeoConstWidget);
  // !!!***
//  connect(twGeo, &AGeoTree::RequestNormalDetectorDraw, MW, &MainWindow::ShowGeometrySlot);
  connect(twGeo, &AGeoTree::RequestShowPrototypeList, this, &AGeoTreeWin::onRequestShowPrototypeList);
  // !!!***
//  connect(Detector->Sandwich, &ASandwich::RequestGuiUpdate, this, &A3GeoConWin::onSandwichRebuild);
  connect(&MaterialHub, &AMaterialHub::materialsChanged, this, &AGeoTreeWin::onMaterialsChanged);

  QPalette palette = ui->frObjectEditor->palette();
  palette.setColor( backgroundRole(), QColor( 240, 240, 240 ) );
  ui->frObjectEditor->setPalette( palette );
  ui->frObjectEditor->setAutoFillBackground( true );

  connect(this, &AGeoTreeWin::requestDelayedRebuildAndRestoreDelegate, twGeo, &AGeoTree::rebuildDetectorAndRestoreCurrentDelegate, Qt::QueuedConnection);

  QPalette p = ui->pteTP->palette();
  p.setColor(QPalette::Active, QPalette::Base, QColor(220,220,220));
  p.setColor(QPalette::Inactive, QPalette::Base, QColor(220,220,220));
  ui->pteTP->setPalette(p);
  ui->pteTP->setReadOnly(true);

  QDoubleValidator* dv = new QDoubleValidator(this);
  dv->setNotation(QDoubleValidator::ScientificNotation);
  QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
  for (QLineEdit * w : qAsConst(list)) if (w->objectName().startsWith("led"))
      w->setValidator(dv);

  ui->cbAutoCheck->setChecked(A3Global::getConstInstance().AutoCheckGeometry);
  on_cbAutoCheck_stateChanged(ui->cbAutoCheck->isChecked());

  //if (!MW->PythonScriptWindow) ui->actionTo_Python->setEnabled(false);
  ui->saPrototypes->setVisible(false);

  connect(ui->menuUndo_redo, &QMenu::aboutToShow, this, &AGeoTreeWin::updateMenuIndication);
}

AGeoTreeWin::~AGeoTreeWin()
{
    delete ui; ui = nullptr;
}

void AGeoTreeWin::onRebuildDetectorRequest()
{
    qDebug() << "A3GeoConWin->onRebuildDetectorRequest triggered";

    AErrorHub::clear();
    emit requestRebuildGeometry();

    if (AErrorHub::isError())
    {
        guitools::message1(AErrorHub::getQError(), "Detected errors in geometry definition", this);
        return; // no sense to show conflicts if there are problems in the geometry definitions
    }

    if (ui->cbAutoCheck->isChecked())
    {
        int nooverlaps = Geometry.checkGeometryForConflicts();
        if (nooverlaps != 0) reportGeometryConflicts();
    }
}

void AGeoTreeWin::updateGui()
{
    //qDebug() << ">DAwindow: updateGui";
    UpdateGeoTree();

    QString str = "Show prototypes";
    int numProto = Geometry.Prototypes->HostedObjects.size();
    if (numProto > 0) str += QString(" (%1)").arg(numProto);
    ui->cbShowPrototypes->setText(str);
    QFont font = ui->cbShowPrototypes->font(); font.setBold(numProto > 0); ui->cbShowPrototypes->setFont(font);
}

void AGeoTreeWin::UpdateGeoTree(QString name, bool bShow)
{
    twGeo->UpdateGui(name);
    updateGeoConstsIndication();

    if (bShow)
    {
        ui->tabwidAddOns->setCurrentIndex(0);
        showNormal();
        raise();
    }
}

void AGeoTreeWin::ShowObject(QString name)
{
    highlightVolume(name);
    Geometry.GeoManager->ClearTracks();
    emit requestShowGeometry(true, false, false);
}

void AGeoTreeWin::FocusVolume(QString name)
{
    emit requestFocusVolume(name);
}

void AGeoTreeWin::ShowObjectRecursive(QString name)
{
    emit requestShowRecursive(name);
}

void AGeoTreeWin::showAllInstances(QString name)
{
    std::vector<AGeoObject*> InstancesNotDiscriminated;
    Geometry.World->findAllInstancesRecursive(InstancesNotDiscriminated);

    TObjArray * list = Geometry.GeoManager->GetListOfVolumes();
    const int size = list->GetEntries();
    QSet<QString> set;

    //select those active ones which have the same prototype
    for (AGeoObject * inst : InstancesNotDiscriminated)
    {
        const ATypeInstanceObject * insType = static_cast<const ATypeInstanceObject*>(inst->Type);
        if (insType->PrototypeName == name && inst->fActive)
            for (AGeoObject * obj : inst->HostedObjects)
            {
                if (obj->Type->isHandlingArray() || obj->Type->isHandlingSet())
                {
                    std::vector<AGeoObject*> vec;
                    obj->collectContainingObjects(vec);
                    for (AGeoObject * obj1 : vec)
                        set << obj1->Name;
                }
                else    set << obj->Name;
            }

        for (int iVol = 0; iVol < size; iVol++)
        {
            TGeoVolume * vol = (TGeoVolume*)list->At(iVol);
            if (!vol) break;
            const QString name = vol->GetName();
            if (set.contains(name))
            {
                vol->SetLineColor(kRed);
                vol->SetLineWidth(3);
            }
            else vol->SetLineColor(kGray);
        }
    }

    emit requestShowGeometry(true, true, false);
}

void AGeoTreeWin::onRequestShowMonitorActiveDirection(const AGeoObject * mon)
{
    const AMonitorHub & MonitorHub = AMonitorHub::getConstInstance();
    std::vector<const AMonitorData*> Mons = MonitorHub.getMonitors(mon);
    if (Mons.empty())
    {
        qWarning() << "Monitor records were not found for this object!";
        return;
    }

    const AMonitorConfig & c = Mons.front()->Monitor->config;
    double length1 = c.size1;
    double length2 = c.size2;
    if (c.shape == 1) length2 = length1;

    TGeoManager * GeoManager = Geometry.GeoManager;
    GeoManager->ClearTracks();

    for (const AMonitorData * MonData : Mons)
    {
        Int_t track_index = GeoManager->AddTrack(1,22);
        TVirtualGeoTrack * track = GeoManager->GetTrack(track_index);

        const double * worldPos = MonData->Position.data();

        double hl[3] = {-length1, 0, 0}; //local coordinates
        double vl[3] = {0, -length2, 0}; //local coordinates
        double mhl[3]; //master coordinates (world)
        double mvl[3]; //master coordinates (world)

        TGeoNavigator * navigator = GeoManager->GetCurrentNavigator();
        if (!navigator)
        {
            qDebug() << "Show monitor: Current navigator does not exist, creating new";
            navigator = GeoManager->AddNavigator();
        }
        navigator->FindNode(worldPos[0], worldPos[1], worldPos[2]);
        //qDebug() << navigator->GetCurrentVolume()->GetName();
        navigator->LocalToMasterVect(hl, mhl); //qDebug() << mhl[0]<< mhl[1]<< mhl[2];
        navigator->LocalToMasterVect(vl, mvl);

        track->AddPoint(worldPos[0]+mhl[0], worldPos[1]+mhl[1], worldPos[2]+mhl[2], 0);
        track->AddPoint(worldPos[0]-mhl[0], worldPos[1]-mhl[1], worldPos[2]-mhl[2], 0);
        track->AddPoint(worldPos[0], worldPos[1], worldPos[2], 0);
        track->AddPoint(worldPos[0]+mvl[0], worldPos[1]+mvl[1], worldPos[2]+mvl[2], 0);
        track->AddPoint(worldPos[0]-mvl[0], worldPos[1]-mvl[1], worldPos[2]-mvl[2], 0);
        track->SetLineWidth(4);
        track->SetLineColor(kBlack);

        //show orientation
        double l[3] = {0,0, std::max(length1,length2)}; //local coordinates
        double m[3]; //master coordinates (world)
        navigator->LocalToMasterVect(l, m);
        if (c.bUpper)
        {
            track_index = GeoManager->AddTrack(1,22);
            track = GeoManager->GetTrack(track_index);
            track->AddPoint(worldPos[0], worldPos[1], worldPos[2], 0);
            track->AddPoint(worldPos[0]+m[0], worldPos[1]+m[1], worldPos[2]+m[2], 0);
            track->SetLineWidth(4);
            track->SetLineColor(kRed);
        }
        if (c.bLower)
        {
            track_index = GeoManager->AddTrack(1,22);
            track = GeoManager->GetTrack(track_index);
            track->AddPoint(worldPos[0], worldPos[1], worldPos[2], 0);
            track->AddPoint(worldPos[0]-m[0], worldPos[1]-m[1], worldPos[2]-m[2], 0);
            track->SetLineWidth(4);
            track->SetLineColor(kRed);
        }
    }

    emit requestShowGeometry(true, true, true);
    emit requestShowTracks();
}

void AGeoTreeWin::onRequestEnableGeoConstWidget(bool flag)
{
    ui->tabwConstants->setEnabled(flag);
}

void AGeoTreeWin::onMaterialsChanged()
{
    updateGui();
}

void AGeoTreeWin::highlightVolume(const QString & VolName)
{
    AGeoObject * obj = Geometry.World->findObjectByName(VolName);
    if (!obj) return;

    QSet<QString> set;
    if (obj->Type->isHandlingArray() || obj->Type->isInstance() || obj->Type->isHandlingSet())
    {
        std::vector<AGeoObject*> vec;
        obj->collectContainingObjects(vec);
        for (const AGeoObject * obj : vec)
            set << obj->Name;
    }
    else    set << VolName;

    TObjArray* list = Geometry.GeoManager->GetListOfVolumes();
    int size = list->GetEntries();

    for (int iVol = 0; iVol < size; iVol++)
    {
        TGeoVolume* vol = (TGeoVolume*)list->At(iVol);
        if (!vol) break;

        QString name = vol->GetName();
        int ind = name.indexOf(Geometry.IndexSeparator.Data());   //reserved for monitors/calorimeters: after "_-_" comes monitor index
        if (ind != -1) name.truncate(ind);

        if (set.contains(name))
        {
            vol->SetLineColor(kRed);
            vol->SetLineWidth(3);
        }
        else vol->SetLineColor(kGray);
    }

    emit requestClearGeoMarkers(0);
    if (obj->isCalorimeter()) markCalorimeterBinning(obj);
}

void AGeoTreeWin::markCalorimeterBinning(const AGeoObject * obj)
{
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();
    std::vector<const ACalorimeterData *> calVec = CalHub.getCalorimeters(obj);
    if (calVec.empty())
    {
        qWarning() << "Calorimeter records were not found for this object!";
        return;
    }

    const ACalorimeter * cal = calVec.front()->Calorimeter;
    if (!cal)
    {
        qWarning() << "Calorimeter is nullptr for this object!";
        return;
    }

    const ACalorimeterProperties & props = cal->Properties;

    TGeoManager * GeoManager = Geometry.GeoManager;
    GeoManager->ClearTracks();

    for (const ACalorimeterData * cd : calVec)
    {
        const double * worldPos = cd->Position.data();

        /*
        qDebug() << "Center pos: " << worldPos[0] << worldPos[1] << worldPos[2];
        qDebug() << "Unit vectorX:" << cd->UnitXMaster[0] << cd->UnitXMaster[1] << cd->UnitXMaster[2];
        qDebug() << "Unit vectorY:" << cd->UnitYMaster[0] << cd->UnitYMaster[1] << cd->UnitYMaster[2];
        qDebug() << "Unit vectorZ:" << cd->UnitZMaster[0] << cd->UnitZMaster[1] << cd->UnitZMaster[2];
        qDebug() << "---";
        qDebug() << "CalorimOr:" << props.Origin[0] << props.Origin[1] << props.Origin[2];
        qDebug() << "---";
        */

        bool bXon = !props.isAxisOff(0);
        bool bYon = !props.isAxisOff(1);
        bool bZon = !props.isAxisOff(2);

        double origin[3];
        for (int i=0; i<3; i++)
        {
            origin[i] = worldPos[i];
            if (bXon) origin[i] += props.Origin[0] * cd->UnitXMaster[i];
            if (bYon) origin[i] += props.Origin[1] * cd->UnitYMaster[i];
            if (bZon) origin[i] += props.Origin[2] * cd->UnitZMaster[i];
        }

        //qDebug() << "Origin:" << origin[0] << origin[1] << origin[2];
        //qDebug() << "---";

        std::vector<std::array<double, 3>> XYZs;
        std::array<double,3> pos;
        if (bXon)
        {
            for (int ib = 0; ib < props.Bins[0]; ib++)
            {
                for (int i = 0; i < 3; i++)
                    pos[i] = origin[i] + props.Step[0]*cd->UnitXMaster[i] * ib;
                XYZs.push_back(pos);
            }
            emit requestAddGeoMarkers(XYZs, 2, 2, 1);
        }

        if (bYon)
        {
            XYZs.clear();
            for (int ib = 0; ib < props.Bins[1]; ib++)
            {
                for (int i = 0; i < 3; i++)
                    pos[i] = origin[i] + props.Step[1]*cd->UnitYMaster[i] * ib;
                XYZs.push_back(pos);
            }
            emit requestAddGeoMarkers(XYZs, 3, 2, 1);
        }

        if (bZon)
        {
            XYZs.clear();
            for (int ib = 0; ib < props.Bins[2]; ib++)
            {
                for (int i = 0; i < 3; i++)
                    pos[i] = origin[i] + props.Step[2]*cd->UnitZMaster[i] * ib;
                XYZs.push_back(pos);
            }
            emit requestAddGeoMarkers(XYZs, 4, 2, 1);
        }
    }
}

void ShowNodes(const TGeoNode* node, int level)
{
    int numNodes = node->GetNdaughters();
    qDebug() << QString("+").repeated(level)
             << node->GetName()<<node->GetVolume()->GetName()<<node->GetVolume()->GetShape()->ClassName()<<numNodes;

    for (int i=0; i<numNodes; i++)
        ShowNodes(node->GetDaughter(i), level+1);
}

void AGeoTreeWin::resizeEvent(QResizeEvent *event)
{
    if (!isVisible()) return;

    int AllW = ui->tabwConstants->width() - 3;

    int SecW = AllW * 0.33333;
    if (SecW > 50) SecW = 50;

    int FirstPlusThird = AllW - SecW;

    int FirstW = 0.4 * FirstPlusThird;
    if (FirstW > 150) FirstW = 150;

    ui->tabwConstants->setColumnWidth(0, FirstW);
    ui->tabwConstants->setColumnWidth(1, SecW);
    ui->tabwConstants->setColumnWidth(2, FirstPlusThird - FirstW);

    QMainWindow::resizeEvent(event);
}

void AGeoTreeWin::on_pbRootWeb_clicked()
{
  QDesktopServices::openUrl( QUrl("http://gdml.web.cern.ch/GDML/doc/GDMLmanual.pdf") );
}

void AGeoTreeWin::on_pbCheckGeometry_clicked()
{
    int overlapCount = Geometry.checkGeometryForConflicts();
    if (overlapCount == 0)
    {
        guitools::message("No conflicts were detected", this);
        return;
    }
    reportGeometryConflicts();
}

void AGeoTreeWin::reportGeometryConflicts()
{
    TObjArray * overlaps = Geometry.GeoManager->GetListOfOverlaps();
    const int overlapCount = overlaps->GetEntries();
    QString text;
    for (int i = 0; i < overlapCount; i++)
        text += QString::fromLocal8Bit(((TNamed*)overlaps->At(i))->GetTitle()) + '\n';
    guitools::message1(text, "Detected conflicts:", this);
}

void AGeoTreeWin::on_cbAutoCheck_clicked(bool checked)
{
    A3Global::getInstance().AutoCheckGeometry = checked;
}

void AGeoTreeWin::on_pbRunTestParticle_clicked()
{
   ui->pteTP->clear();
   AGeometryTester Tester(Geometry.GeoManager);

   double Start[3];
   double Dir[3];

   Start[0] = ui->ledTPx->text().toDouble();
   Start[1] = ui->ledTPy->text().toDouble();
   Start[2] = ui->ledTPz->text().toDouble();
   Dir[0]   = ui->ledTPi->text().toDouble();
   Dir[1]   = ui->ledTPj->text().toDouble();
   Dir[2]   = ui->ledTPk->text().toDouble();

   Tester.Test(Start, Dir);

   ui->pteTP->appendHtml("Objects on the path:");
   for (int i=0; i<Tester.Record.size(); i++)
   {
       const AGeometryTesterReportRecord& r = Tester.Record.at(i);

       QString s;
       s += "<pre>";
       s += " " + r.volName;
       s += " (#" + QString::number(r.nodeIndex) + ")";
       s += " of ";
       TColor* rc = gROOT->GetColor(r.matIndex+1);
       int red = 255*rc->GetRed();
       int green = 255*rc->GetGreen();
       int blue = 255*rc->GetBlue();
       s += "<span style = \"color:rgb("+QString::number(red)+", "+QString::number(green)+", "+QString::number(blue)+")\">";
       s += MaterialHub.getMaterialName(r.matIndex);
       s += "</span>";
       s += ",  from ";
       s += "(" + QString::number(r.startX)+", "+ QString::number(r.startY)+", "+ QString::number(r.startZ)+")";

       if (i != Tester.Record.size()-1)
       {
           s += "  to  ";
           const AGeometryTesterReportRecord& r1 = Tester.Record.at(i+1);
           s += "("+QString::number(r1.startX)+", "+ QString::number(r1.startY)+", "+ QString::number(r1.startZ)+")";
           double dx2 = r.startX - r1.startX; dx2 *= dx2;
           double dy2 = r.startY - r1.startY; dy2 *= dy2;
           double dz2 = r.startZ - r1.startZ; dz2 *= dz2;
           double distance = sqrt( dx2 + dy2 + dz2 );
           s += "  Distance: "+QString::number(distance) + " mm.";
       }
       else
       {
           s += " and until escaped.";
       }
       s += "</pre>";

       ui->pteTP->appendHtml(s);
   }

   Geometry.GeoManager->ClearTracks();

   for (int i=0; i<Tester.Record.size(); i++)
   {
       const AGeometryTesterReportRecord& r = Tester.Record.at(i);
       TGeoTrack* track = new TGeoTrack(1, 10);
       track->SetLineColor(r.matIndex+1);
       track->SetLineWidth(4);
       track->AddPoint(r.startX, r.startY, r.startZ, 0);
       if (i != Tester.Record.size()-1)
       {
           const AGeometryTesterReportRecord& r1 = Tester.Record.at(i+1);
           track->AddPoint(r1.startX, r1.startY, r1.startZ, 0);
       }
       else
           track->AddPoint(Tester.escapeX, Tester.escapeY, Tester.escapeZ, 0);

       Geometry.GeoManager->AddTrack(track);
   }

   emit requestShowGeometry(false, true, true);
   emit requestShowTracks();
}

void AGeoTreeWin::on_cbAutoCheck_stateChanged(int)
{
    bool checked = ui->cbAutoCheck->isChecked();
    QColor col = (checked ? Qt::black : Qt::red);
    QPalette p = ui->cbAutoCheck->palette();
    p.setColor(QPalette::Active, QPalette::WindowText, col );
    p.setColor(QPalette::Inactive, QPalette::WindowText, col );
    ui->cbAutoCheck->setPalette(p);
}

void AGeoTreeWin::updateGeoConstsIndication()
{
    ui->tabwConstants->clearContents();

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    const int numConsts = GC.countConstants();

    bGeoConstsWidgetUpdateInProgress = true; // -->
        ui->tabwConstants->setRowCount(numConsts + 1);
        ui->tabwConstants->setColumnWidth(1, 50);
        for (int i = 0; i <= numConsts; i++)
        {
            const QString Name  =      ( i == numConsts ? ""  : GC.getName(i));
            const QString Value =      ( i == numConsts ? "0" : QString::number(GC.getValue(i)) );
            const QString Expression = ( i == numConsts ? ""  : GC.getExpression(i) );

            QTableWidgetItem * newItem = new QTableWidgetItem(Name);
            QString Comment = GC.getComment(i);
            newItem->setToolTip(Comment);
            ui->tabwConstants->setItem(i, 0, newItem);

            ALineEditWithEscape * edit = new ALineEditWithEscape(Value, ui->tabwConstants);
            edit->setValidator(new QDoubleValidator(edit));
            edit->setFrame(false);
            connect(edit, &ALineEditWithEscape::editingFinished, [this, i, edit](){this->onGeoConstEditingFinished(i, edit->text()); });
            connect(edit, &ALineEditWithEscape::escapePressed,   [this, i](){this->onGeoConstEscapePressed(i); });
            ui->tabwConstants->setCellWidget(i, 1, edit);

            /*
            AOneLineTextEdit * ed = new AOneLineTextEdit("", ui->tabwConstants);
            AGeoBaseDelegate::configureHighligherAndCompleter(ed, i);
            ed->setText(Expression);
            ed->setFrame(false);
            connect(ed, &AOneLineTextEdit::editingFinished, [this, i, ed](){this->onGeoConstExpressionEditingFinished(i, ed->text()); });
            connect(ed, &AOneLineTextEdit::escapePressed,   [this, i](){this->onGeoConstEscapePressed(i); });
            */

            newItem = new QTableWidgetItem(Expression);
            newItem->setToolTip(Comment);
            if (i == numConsts) newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable);
            ui->tabwConstants->setItem(i, 2, newItem);

            if (!Expression.isEmpty()) edit->setEnabled(false);
        }
        ui->tabwConstants->resizeRowsToContents();
    bGeoConstsWidgetUpdateInProgress = false; // <--
}

#include "ageoconstexpressiondialog.h"
void AGeoTreeWin::on_tabwConstants_cellClicked(int row, int column)
{
    if (column != 2) return;

    AGeoConsts & GC = AGeoConsts::getInstance();
    const int numConsts = GC.countConstants();
    if (row == -1 || row >= numConsts) return;

    AGeoConstExpressionDialog D(this, row);
    D.exec();

    GC.updateFromExpressions();
    updateGeoConstsIndication();
    emit requestDelayedRebuildAndRestoreDelegate();
}

void AGeoTreeWin::onGeoConstEditingFinished(int index, QString strNewValue)
{
    //qDebug() << "GeoConst value changed! index/text are:" << index << strNewValue;
    AGeoConsts & GC = AGeoConsts::getInstance();

    if (index == GC.countConstants()) return; // nothing to do yet - this constant is not yet defined

    bool ok;
    double val = strNewValue.toDouble(&ok);
    if (!ok)
    {
        guitools::message("Bad format of the edited value of geometry constant!", this);
        updateGeoConstsIndication();
        return;
    }

    if (val == GC.getValue(index)) return;

    GC.setNewValue(index, val);
    emit requestDelayedRebuildAndRestoreDelegate();
}

void AGeoTreeWin::onGeoConstEscapePressed(int /*index*/)
{
    updateGeoConstsIndication();
}

void AGeoTreeWin::onRequestShowPrototypeList()
{
    ui->cbShowPrototypes->setChecked(true);
}

void AGeoTreeWin::updateMenuIndication()
{
//    ui->actionUndo->setEnabled(MW->Config->isUndoAvailable());
      ui->actionUndo->setEnabled(false);
//    ui->actionRedo->setEnabled(MW->Config->isRedoAvailable());
      ui->actionRedo->setEnabled(false);
}

void AGeoTreeWin::on_tabwConstants_cellChanged(int row, int column)
{
    if (column != 0) return; // only name change or new
    if (bGeoConstsWidgetUpdateInProgress) return;
    //qDebug() << "Geo const name changed";

    AGeoConsts & GC = AGeoConsts::getInstance();
    const int numConsts = GC.countConstants();

    if (numConsts == row)
    {
        //qDebug() << "Attempting to add new geometry constant";
        QString Name = ui->tabwConstants->item(row, 0)->text().simplified();

        QLineEdit * le = dynamic_cast<QLineEdit*>(ui->tabwConstants->cellWidget(row, 1));
        if (!le)
        {
            guitools::message("Something went wrong!", this);
            return;
        }
        bool ok;
        double Value = le->text().toDouble(&ok);
        if (!ok) Value = 0;

        QString errorStr = GC.addNewConstant(Name, Value, -1);
        if (!errorStr.isEmpty())
        {
            guitools::message(errorStr, this);
            updateGeoConstsIndication();
            return;
        }
        //MW->writeDetectorToJson(MW->Config->JSON);
        emit requestDelayedRebuildAndRestoreDelegate();
    }
    else
    {
        //qDebug() << "Attempting to change name of a geometry constant";
        QString newName = ui->tabwConstants->item(row, 0)->text().simplified();
        QString errorStr;
        bool ok = GC.rename(row, newName, Geometry.World, errorStr);
        if (!ok)
        {
            if (!errorStr.isEmpty()) guitools::message(errorStr, this);
            updateGeoConstsIndication();
            return;
        }
        else
        {
            emit requestDelayedRebuildAndRestoreDelegate();
        }
    }

    updateGeoConstsIndication();
}

void AGeoTreeWin::on_tabwConstants_customContextMenuRequested(const QPoint &pos)
{
    AGeoConsts & GC = AGeoConsts::getInstance();
    int index = ui->tabwConstants->currentRow();

    QMenu menu;
    QAction * removeA = menu.addAction("Remove selected constant"); removeA->setEnabled(index != -1 && index != GC.countConstants());
    QAction * addAboveA = menu.addAction("Add new constant above"); addAboveA->setEnabled(index != -1 && index != GC.countConstants());

    menu.addSeparator();

    QAction * setCommentA = menu.addAction("Add comment"); setCommentA->setEnabled(index != -1 && index != GC.countConstants());


    QAction * selected = menu.exec(ui->tabwConstants->mapToGlobal(pos));
    if (selected == removeA)
    {
        if (!GC.isIndexValid(index)) return;

        QString name = GC.getName(index);
        if (!name.isEmpty())
        {
            QString constUsingIt = GC.isGeoConstInUse(QRegularExpression("\\b"+name+"\\b"), index);
            if (!constUsingIt.isEmpty())
            {
                guitools::message(QString("\"%1\" cannot be removed.\nThe first geometric constant using it:\n\n%2").arg(name, constUsingIt), this);
                return;
            }
            const AGeoObject * obj = Geometry.World->isGeoConstInUseRecursive(QRegularExpression("\\b"+name+"\\b"));
            if (obj)
            {
                guitools::message(QString("\"%1\" cannot be removed.\nThe first object using it:\n\n%2").arg(name, obj->Name), this);
                return;
            }
        }

        GC.removeConstant(index);
 //       MW->writeDetectorToJson(MW->Config->JSON);
        updateGeoConstsIndication();
        emit requestDelayedRebuildAndRestoreDelegate();
    }
    else if (selected == addAboveA)
    {
        GC.addNoNameConstant(index);
//        MW->writeDetectorToJson(MW->Config->JSON);
        updateGeoConstsIndication();
    }
    else if (selected == setCommentA)
    {
        QString txt;
        guitools::inputString("New comment (empty to remove)", txt, this);
        GC.setNewComment(index, txt);
//        MW->writeDetectorToJson(MW->Config->JSON);
        updateGeoConstsIndication();
    }
}

void AGeoTreeWin::on_actionUndo_triggered()
{
    /*
    bool ok = MW->Config->isUndoAvailable();
    if (!ok)
        guitools::message("Undo is not available!", this);
    else
    {
        QString err = MW->Config->doUndo();
        if (!err.isEmpty()) guitools::message(err, this);
    }
    */
}

void AGeoTreeWin::on_actionRedo_triggered()
{
    /*
    bool ok = MW->Config->isRedoAvailable();
    if (!ok)
        guitools::message("Redo is not available!", this);
    else
    {
        QString err = MW->Config->doRedo();
        if (!err.isEmpty()) guitools::message(err, this);
    }
    */
}

void AGeoTreeWin::on_actionHow_to_use_drag_and_drop_triggered()
{
    QString s = "Drag & drop can be used to move items\n"
                "  from one container to another\n"
                "\n"
                "Drop when Alt or Shift or Control is pressed\n"
                "  changes the item order:\n"
                "  the dragged item is inserted between two object\n"
                "  according to the drop indicator";
    guitools::message(s, this);
}

#include "ageoscriptmaker.h"
void AGeoTreeWin::on_actionTo_JavaScript_triggered()
{
    QString script;
    AGeoScriptMaker sm(AGeoScriptMaker::JavaScript);
    sm.createScript(script);
    emit requestAddScript(script);
}

void AGeoTreeWin::on_cbShowPrototypes_toggled(bool checked)
{
    ui->saPrototypes->setVisible(checked);
}

void AGeoTreeWin::on_pbSaveTGeo_clicked()
{
    QString fileName = guitools::dialogSaveFile(this, "Export geometry to file (use .gdml or .root suffix)", "GDML files (*.gdml);;Root files(*.root)");
    if (fileName.isEmpty()) return;

    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".gdml";

    QString err;
    /*
    // old system
    if (QFileInfo(fileName).suffix() == "root") err = Geometry.exportToROOT(fileName);
    else                                        err = Geometry.exportToGDML(fileName);
    */
    // new system
    err = Geometry.exportGeometry(fileName);

    if (!err.isEmpty()) guitools::message(err, this);
}

#include "aconfig.h"
void AGeoTreeWin::on_pmParseInGeometryFromGDML_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load GDML file", "GDML files (*.gdml)");
    if (fileName.isEmpty()) return;
    QFileInfo fi(fileName);
    if (fi.suffix() != "gdml")
    {
        guitools::message("Only GDML files are accepted!", this);
        return;
    }

    AGeometryHub::getInstance().clearWorld();

    //QString err = Geometry.importGDML(fileName);   // old system based on replace in file
    QString err = Geometry.importGeometry(fileName); // new system based on scaling

    Geometry.writeToJson(AConfig::getInstance().JSON);
    MaterialHub.writeToJson(AConfig::getInstance().JSON);
    emit requestRebuildGeometry();
    if (!err.isEmpty()) guitools::message(err, this);
}

void AGeoTreeWin::on_actionFind_object_triggered()
{
    QDialog D(this);
    D.setWindowTitle("Find object by name");
    QHBoxLayout * l = new QHBoxLayout(&D);
    l->addWidget(new QLabel("Object name:"));
    QLineEdit * le = new QLineEdit();
    l->addWidget(le);
    connect(le, &QLineEdit::editingFinished, &D, &QDialog::accept);

    D.move(x()+40, y()+35);
    int res = D.exec();
    if (res == QDialog::Rejected) return;

    QString name = le->text();
    if (name.isEmpty()) return;

    UpdateGeoTree(name, true);
}
