//has to be on top!!
#include "TCanvas.h"

#include "agraphwindow.h"
#include "ui_agraphwindow.h"
#include "agraphrasterwindow.h"
#include "guitools.h"
#include "shapeablerectitem.h"
#include "graphicsruler.h"
#include "arootlineconfigurator.h"
#include "arootmarkerconfigurator.h"
#include "atoolboxscene.h"
#include "abasketmanager.h"
#include "adrawexplorerwidget.h"
#include "abasketlistwidget.h"
#include "amultigraphdesigner.h"
#include "adrawtemplate.h"
#include "ascripthub.h"
#include "agraphwin_si.h"
#include "ajscriptmanager.h"
#include "aviewer3d.h"
#include "asetmarginsdialog.h"
#include "a3global.h"
#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
#endif

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QColorDialog>
#include <QListWidget>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVariant>
#include <QVariantList>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>
#include <QThread>
#include <QCloseEvent>
#include <QMouseEvent>

#include "TMath.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TStyle.h"
#include "TF1.h"
#include "TF2.h"
#include "TView.h"
#include "TMultiGraph.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TStyle.h"
#include "TLine.h"
#include "TAxis.h"
#include "TAttLine.h"
#include "TLegend.h"
#include "TTree.h"
#include "TGaxis.h"
#include "TFrame.h"
#include "TPaveText.h"

AGraphWindow::AGraphWindow(QWidget * parent) :
    AGuiWindow("Graph", parent),
    ui(new Ui::AGraphWindow)
{
    Basket = new ABasketManager();

    ui->setupUi(this);

    setMinimumWidth(200);
    ui->swToolBox->setVisible(false);
    ui->swToolBox->setCurrentIndex(0);
    ui->sProjBins->setEnabled(false);
    showHintInStatus();

    ui->labX->setText(QChar(8596));
    ui->labY->setText(QChar(8597));

    // Raster window
    RasterWindow = new AGraphRasterWindow(this);
    RasterWindow->resize(400, 400);
    RasterWindow->forceResize();
    connect(RasterWindow, &AGraphRasterWindow::LeftMouseButtonReleased, this, &AGraphWindow::updateControls);
    connect(RasterWindow, &AGraphRasterWindow::reportCursorPosition,    this, &AGraphWindow::onCursorPositionReceived);

    // Draw explorer widget
    Explorer = new ADrawExplorerWidget(*this, DrawObjects);
    ui->layExplorer->insertWidget(2, Explorer);
    ui->splitter->setSizes({200,600});
    ui->pbBackToLast->setVisible(false);
    connect(Explorer, &ADrawExplorerWidget::requestShowLegendDialog, this, &AGraphWindow::showAddLegendDialog);

    // Basket list widget
    lwBasket = new ABasketListWidget(this);
    ui->layBasket->addWidget(lwBasket);
    connect(lwBasket, &ABasketListWidget::customContextMenuRequested, this, &AGraphWindow::onBasketCustomContextMenuRequested);
    connect(lwBasket, &ABasketListWidget::itemDoubleClicked, this, &AGraphWindow::onBasketItemDoubleClicked);
    connect(lwBasket, &ABasketListWidget::requestReorder, this, &AGraphWindow::onBasketReorderRequested);

    connectScriptUnitDrawRequests(AScriptHub::getInstance().getJScriptManager().getInterfaces());
#ifdef ANTS3_PYTHON
    connectScriptUnitDrawRequests(AScriptHub::getInstance().getPythonManager().getInterfaces());
#endif
    connect(this, &AGraphWindow::requestLocalDrawObject, this, &AGraphWindow::processScriptDrawRequest, Qt::QueuedConnection);
    // !!!*** TODO: similarly to the above, modify draw tree from script

    //input boxes format validators
    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    updateMargins();
    setShowCursorPosition(false);

    QHBoxLayout* l = dynamic_cast<QHBoxLayout*>(centralWidget()->layout());
    if (l) l->insertWidget(1, RasterWindow);
    else guitools::message("Unexpected layout!", this);

    //overlay to show selection box, later scale tool too
    gvOverlay = new QGraphicsView(this);
    gvOverlay->setFrameStyle(0);
    gvOverlay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gvOverlay->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ToolBoxScene = new AToolboxScene(this);
    gvOverlay->setScene(ToolBoxScene);
    gvOverlay->hide();

    //toolbox graphics scene
    connect(ToolBoxScene->getSelBox(), &ShapeableRectItem::geometryChanged, this, &AGraphWindow::selBoxGeometryChanged);
    connect(ToolBoxScene->getSelBox(), &ShapeableRectItem::requestResetGeometry, this, &AGraphWindow::selBoxResetGeometry);
    connect(ui->cbSelBoxShowBG, &QCheckBox::toggled, ToolBoxScene->getSelBox(), &ShapeableRectItem::setShowContrast);
    connect(ToolBoxScene->getRuler(), &GraphicsRuler::geometryChanged, this, &AGraphWindow::rulerGeometryChanged);
    connect(ui->cbRulerTicksLength, &QCheckBox::toggled, ToolBoxScene->getRuler(), &GraphicsRuler::setShowTicks);
    connect(ui->cbRulerShowBG, &QCheckBox::toggled, ToolBoxScene->getRuler(), &GraphicsRuler::setShowContrast);

    //new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(onBasketDeleteShortcutActivated()));
    new QShortcut(QKeySequence(Qt::Key_Delete), this, this, &AGraphWindow::onBasketDeleteShortcutActivated);

    DrawTemplate.Selection.bExpanded = true;

    AScriptHub::getInstance().addCommonInterface(new AGraphWin_SI(this), "grwin");
}

AGraphWindow::~AGraphWindow()
{
    delete Viewer3D; Viewer3D = nullptr;

    clearBasket();

    delete ui;

    clearRegisteredTObjects();

    delete ToolBoxScene; ToolBoxScene =  nullptr;
    delete gvOverlay; gvOverlay = nullptr;

    delete Basket; Basket = nullptr;
}

#include "agraph_si.h"
#include "ahist_si.h"
#include "atree_si.h"
void AGraphWindow::connectScriptUnitDrawRequests(const std::vector<AScriptInterface *> interfaces)
{
    const AGraph_SI * graphInter = nullptr;
    const AHist_SI  * histInter  = nullptr;
    const ATree_SI  * treeInter  = nullptr;

    for (const AScriptInterface * inter : interfaces)
    {
        if (!graphInter)
        {
            const AGraph_SI * test = dynamic_cast<const AGraph_SI*>(inter);
            if (test)
            {
                graphInter = test;
                continue;
            }
        }
        if (!histInter)
        {
            const AHist_SI * test = dynamic_cast<const AHist_SI*>(inter);
            if (test)
            {
                histInter = test;
                continue;
            }
        }
        if (!treeInter)
        {
            const ATree_SI * test = dynamic_cast<const ATree_SI*>(inter);
            if (test)
            {
                treeInter = test;
                continue;
            }
        }
    }

    if (graphInter) connect(graphInter, &AGraph_SI::requestDraw,    this, &AGraphWindow::onScriptDrawRequest, Qt::DirectConnection);
    if (histInter)  connect(histInter,  &AHist_SI::requestDraw,     this, &AGraphWindow::onScriptDrawRequest, Qt::DirectConnection);
    if (treeInter)  connect(treeInter,  &ATree_SI::requestTreeDraw, this, &AGraphWindow::onScriptDrawTree);
}

void AGraphWindow::addLine(double x1, double y1, double x2, double y2, int color, int width, int style)
{
    TLine* l = new TLine(x1, y1, x2, y2);
    l->SetLineColor(color);
    l->SetLineWidth(width);
    l->SetLineStyle(style);

    draw(l, "SAME");
}

#include "TArrow.h"
void AGraphWindow::addArrow(double x1, double y1, double x2, double y2, int color, int width, int style)
{
    TArrow * l = new TArrow(x1, y1, x2, y2);
    l->SetLineColor(color);
    l->SetLineWidth(width);
    l->SetLineStyle(style);

    draw(l, ">SAME");
}

void AGraphWindow::showAndFocus()
{
    RasterWindow->fCanvas->cd();
    show();
    activateWindow();
    raise();
}

void AGraphWindow::setAsActiveRootWindow()
{
    RasterWindow->fCanvas->cd();
}

void AGraphWindow::clearRootCanvas()
{
    RasterWindow->fCanvas->Clear();
}

void AGraphWindow::updateRootCanvas()
{
    RasterWindow->updateRootCanvas();
}

void AGraphWindow::setModifiedFlag()
{
    RasterWindow->fCanvas->Modified();
}

void AGraphWindow::setLogScale(bool X, bool Y)
{
    ui->cbLogX->setChecked(X);
    ui->cbLogY->setChecked(Y);
}

void AGraphWindow::clearDrawObjects_OnShutDown()
{
    DrawObjects.clear();
    RasterWindow->fCanvas->Clear();
}

double AGraphWindow::getCanvasMinX()
{
    return RasterWindow->getCanvasMinX();
}

double AGraphWindow::getCanvasMaxX()
{
    return RasterWindow->getCanvasMaxX();
}

double AGraphWindow::getCanvasMinY()
{
    return RasterWindow->getCanvasMinY();
}

double AGraphWindow::getCanvasMaxY()
{
    return RasterWindow->getCanvasMaxY();
}

double AGraphWindow::getMinX(bool *ok)
{
    return ui->ledXfrom->text().toDouble(ok);
}

double AGraphWindow::getMaxX(bool *ok)
{
    return ui->ledXto->text().toDouble(ok);
}

double AGraphWindow::getMinY(bool *ok)
{
    return ui->ledYfrom->text().toDouble(ok);
}

double AGraphWindow::getMaxY(bool *ok)
{
    return ui->ledYto->text().toDouble(ok);
}

double AGraphWindow::getMinZ(bool *ok)
{
    return ui->ledZfrom->text().toDouble(ok);
}

double AGraphWindow::getMaxZ(bool *ok)
{
    return ui->ledZto->text().toDouble(ok);
}

void AGraphWindow::draw(TObject * obj, QString options, bool update, bool transferOwnership)
{
    QString optNoSame = (options.simplified()).remove("same", Qt::CaseInsensitive);
    if (obj && optNoSame.isEmpty())
    {
        QString Type = obj->ClassName();
        if (Type.startsWith("TH1") || Type == "TProfile") options += "hist";
        //else if (Type.startsWith("TH2")) opt += "colz";
    }

    if (options.contains("same", Qt::CaseInsensitive))
    {
        makeCopyOfDrawObjects();
    }
    else
    {
        //this is new main object
        clearRegisteredTObjects();             //delete all TObjects previously drawn
        clearCopyOfDrawObjects();       //"go back" is not possible anymore
        clearCopyOfActiveBasketId();    //restore basket current item is not possible anymore
        ActiveBasketItem = -1;
        updateBasketGUI();

        DrawObjects.clear();
    }
    DrawObjects.push_back( ADrawObject(obj, options) );

    if (DrawObjects.size() == 1) updateMargins(&DrawObjects.front());

    drawSingleObject(obj, options.toLatin1().data(), update);

    if (transferOwnership) registerTObject(obj);

    enforceOverlayOff();
    updateControls();

    DrawFinished = true;
}

void AGraphWindow::updateGuiControlsForMainObject(const QString & className, const QString & options)
{
    //3D control
    bool flag3D = false;
    if (className.startsWith("TH3") || className.startsWith("TProfile2D") || className.startsWith("TH2") || className.startsWith("TF2") || className.startsWith("TGraph2D"))
        flag3D = true;
    if ((className.startsWith("TH2") || className.startsWith("TProfile2D")) && ( options.contains("col",Qt::CaseInsensitive) || options.contains("prof", Qt::CaseInsensitive) || (options.isEmpty())) )
        flag3D = false;
    //      qDebug()<<"3D flag:"<<flag3D;

    ui->fZrange->setEnabled(flag3D);
    setShowCursorPosition(!flag3D);

    ui->leOptions->setText(options);

    if ( className.startsWith("TH1") || className == "TF1" )
    {
        ui->fZrange->setEnabled(false);
        ui->cbRulerTicksLength->setChecked(false);
    }
    else if ( className.startsWith("TH2") )
    {
        ui->fZrange->setEnabled(true);
    }
}

void AGraphWindow::registerTObject(TObject * obj)
{
    RegisteredTObjects.push_back(obj);
}

void AGraphWindow::drawSingleObject(TObject * obj, QString options, bool update)
{
    if (!obj)
    {
        qWarning() << "Object does not exist in doDraw";
        return;
    }

    setAsActiveRootWindow();

    TH1* h = dynamic_cast<TH1*>(obj);
    if (h) h->SetStats(ui->cbShowLegend->isChecked());

    TGaxis * gaxis = dynamic_cast<TGaxis*>(obj);
    if (gaxis) updateSecondaryAxis(gaxis, options);

    obj->Draw(options.toLatin1().data());
    if (update) RasterWindow->fCanvas->Update();

    Explorer->updateGui();
    ui->pbBackToLast->setVisible( !PreviousDrawObjects.empty() );

    if (!options.contains("same", Qt::CaseInsensitive))
        updateGuiControlsForMainObject(obj->ClassName(), options);

    fixGraphFrame();
}

void AGraphWindow::fixGraphFrame()
{
    TVirtualPad * pad = RasterWindow->fCanvas->GetPad(0);
    if (pad)
    {
        TFrame * frame = pad->GetFrame();
        if (frame) frame->SetBit(TBox::kCannotMove);
    }
}

void AGraphWindow::updateSecondaryAxis(TGaxis * gaxis, QString options)
{
    updateRootCanvas();   // need to update canvas to request min/max info

    bool bRight = options.contains("Y");
    bool bTop   = options.contains("X");

    if (bRight || bTop)
    {
        //qDebug() << "---->----" << Options;
        double xMin = getCanvasMinX();
        double xMax = getCanvasMaxX();
        double yMin = getCanvasMinY();
        double yMax = getCanvasMaxY();

        gaxis->SetX1(bRight ? xMax : xMin);
        gaxis->SetX2(xMax);
        gaxis->SetY1(bRight ? yMin : yMax);
        gaxis->SetY2(yMax);

        QStringList sl = options.split(';', Qt::SkipEmptyParts);
        if (sl.size() > 3)
        {
            QString sA = sl.at(2);
            QString sB = sl.at(3);

            bool bOKA, bOKB;
            double A = sA.toDouble(&bOKA);
            double B = sB.toDouble(&bOKB);
            if (bOKA && bOKB)
            {
                gaxis->SetWmin( A * (bRight ? yMin : xMin) + B);
                gaxis->SetWmax( A * (bRight ? yMax : xMax) + B);
            }
        }
    }
}

void AGraphWindow::showHintInStatus()
{
    ui->statusBar->showMessage("Use context menus (right mouse button click) to manipulate objects in \"Currently drawn\" and \"Basket\"");
}

void AGraphWindow::setShowCursorPosition(bool flag)
{
    if (RasterWindow) RasterWindow->ShowCursorPosition = flag;
    ui->frCursor->setVisible(flag);

    if (!flag && ui->cbShowCross->isChecked()) ui->cbShowCross->setChecked(false);
}

void AGraphWindow::on_cbShowCross_toggled(bool checked)
{
    RasterWindow->fCanvas->SetCrosshair(checked);

    if (!checked) redrawAll();
}

void AGraphWindow::onBusyOn()
{
    ui->fUIbox->setEnabled(false);
    ui->fBasket->setEnabled(false);
}

void AGraphWindow::onBusyOff()
{
    ui->fUIbox->setEnabled(true);
    ui->fBasket->setEnabled(true);
}

void AGraphWindow::mouseMoveEvent(QMouseEvent *event)
{
    QMainWindow::mouseMoveEvent(event);
}

bool AGraphWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
    {
        RasterWindow->updateRootCanvas();
        showHintInStatus(); // sometimes it is hidden by its own
    }

    if (event->type() == QEvent::Show)
    {
        if (ColdStart)
        {
            //first time this window is shown
            ColdStart = false;
            this->resize(width()+1, height());
            this->resize(width()-1, height());
        }
        else
        {
            //qDebug() << "Graph win show event";
            //RasterWindow->UpdateRootCanvas();
            QTimer::singleShot(100, RasterWindow, [this](){RasterWindow->updateRootCanvas();}); // without delay canvas is not shown in Qt 5.9.5
        }
    }

    //  return AGuiWindow::event(event);  // !!!***
    return QMainWindow::event(event);
}

void AGraphWindow::closeEvent(QCloseEvent * event)
{
    //qDebug() << "Graph win close event";
    RasterWindow->ExtractionCanceled = true;
    RasterWindow->setExtractionComplete(true);
    event->ignore();

    hide();
}

void AGraphWindow::on_cbGridX_toggled(bool checked)
{
    if (TMPignore) return;
    RasterWindow->fCanvas->SetGridx(checked);
    RasterWindow->fCanvas->Update();
}

void AGraphWindow::on_cbGridY_toggled(bool checked)
{
    if (TMPignore) return;
    RasterWindow->fCanvas->SetGridy(checked);
    RasterWindow->fCanvas->Update();
}

void AGraphWindow::on_cbLogX_toggled(bool checked)
{
    if (TMPignore) return;
    RasterWindow->fCanvas->SetLogx(checked);
    RasterWindow->fCanvas->Update();
    updateControls();
}

void AGraphWindow::on_cbLogY_toggled(bool checked)
{
    if (TMPignore) return;
    RasterWindow->fCanvas->SetLogy(checked);
    RasterWindow->fCanvas->Update();
    updateControls();
}

void AGraphWindow::on_ledXfrom_editingFinished()
{
    if (xmin == ui->ledXfrom->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_ledXto_editingFinished()
{
    if (xmax == ui->ledXto->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_ledYfrom_editingFinished()
{
    if (ymin == ui->ledYfrom->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_ledYto_editingFinished()
{
    if (ymax == ui->ledYto->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_ledZfrom_editingFinished()
{
    if (zmin == ui->ledZfrom->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_ledZto_editingFinished()
{
    if (zmax == ui->ledZto->text().toDouble()) return;
    AGraphWindow::reshape();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

TObject * AGraphWindow::getMainPlottedObject()
{
    if (DrawObjects.empty()) return nullptr;
    return DrawObjects.front().Pointer;
}

#include "TView3D.h"
void AGraphWindow::reshape()
{    
    //qDebug() << "Reshape triggered";
    qApp->processEvents();
    //ui->pbHideBar->setFocus(); //remove focus
    //    qDebug()<<"GraphWindow  -> Reshape triggered; objects:"<<DrawObjects.size();

    //if (DrawObjects.isEmpty()) return;
    if (DrawObjects.empty()) return;

    TObject * tobj = DrawObjects.front().Pointer;

    //double xmin, xmax, ymin, ymax, zmin, zmax;
    xmin = ui->ledXfrom->text().toDouble();
    xmax = ui->ledXto->text().toDouble();
    ymin = ui->ledYfrom->text().toDouble();
    ymax = ui->ledYto->text().toDouble();
    bool OKzmin;
    zmin = ui->ledZfrom->text().toDouble(&OKzmin);
    bool OKzmax;
    zmax = ui->ledZto->text().toDouble(&OKzmax);

    //Reshaping the main (first) object
    QString PlotType = tobj->ClassName();
    //    QString PlotOptions = DrawObjects.first().Options;
    //    qDebug()<<"  main object name/options:"<<PlotType<<PlotOptions;

    if (PlotType.startsWith("TH1"))
    {
        TH1* h = (TH1*)tobj;
        h->GetXaxis()->SetRangeUser(xmin, xmax);
        h->SetMinimum(ymin);
        h->SetMaximum(ymax);
    }
    else if (PlotType == "TProfile")
    {
        TProfile* h = (TProfile*)tobj;
        h->GetXaxis()->SetRangeUser(xmin, xmax);
        h->SetMinimum(ymin);
        h->SetMaximum(ymax);
    }
    else if (PlotType.startsWith("TH2"))
    {
        TH2* h = (TH2*)tobj;
        h->GetXaxis()->SetRangeUser(xmin, xmax);
        h->GetYaxis()->SetRangeUser(ymin, ymax);

        if (OKzmin) h->SetMinimum(zmin);
        if (OKzmax) h->SetMaximum(zmax);
    }
    else if (PlotType == "TProfile2D")
    {
        TProfile2D* h = (TProfile2D*)tobj;
        h->GetXaxis()->SetRangeUser(xmin, xmax);
        h->GetYaxis()->SetRangeUser(ymin, ymax);
        //  h->SetMinimum(zmin);
        //  h->SetMaximum(zmax);
    }
    else if (PlotType.startsWith("TF1"))
    {
        TF1* f = (TF1*)tobj;
        f->SetRange(xmin, xmax);
        f->SetMinimum(ymin);
        f->SetMaximum(ymax);
    }
    else if (PlotType.startsWith("TF2"))
    {
        TF2* f = (TF2*)tobj;
        f->SetRange(xmin, ymin, xmax, ymax);
        f->SetMaximum(zmax/1.05);
        f->SetMinimum(zmin);
    }
    else if (PlotType == "TGraph" || PlotType == "TGraphErrors")
    {
        TGraph* gr = (TGraph*)tobj;
        gr->GetXaxis()->SetLimits(xmin, xmax);
        gr->SetMinimum(ymin);
        gr->SetMaximum(ymax);
    }
    else if (PlotType == "TMultiGraph")
    {
        TMultiGraph* gr = (TMultiGraph*)tobj;
        gr->GetXaxis()->SetLimits(xmin, xmax);
        gr->SetMinimum(ymin);
        gr->SetMaximum(ymax);
    }
    else if (PlotType == "TGraph2D")
    {
        TGraph2D * gr = static_cast<TGraph2D*>(tobj);
        //gr->GetXaxis()->SetLimits(xmin, xmax);
        gr->GetXaxis()->SetRangeUser(xmin, xmax);
        //gr->GetYaxis()->SetLimits(ymin, ymax);
        gr->GetYaxis()->SetRangeUser(ymin, ymax);

        //gr->GetZaxis()->SetLimits(zmin, zmax);
        //gr->GetZaxis()->SetRangeUser(zmin, zmax);
        //gr->GetHistogram()->SetRange(xmin, ymin, xmax, ymax);

        // setting min or max; then to basket -> load from basket -> empty screen
        //gr->SetMinimum(zmin);
        //gr->SetMaximum(zmax);

        /*
        TCanvas* c = RasterWindow->fCanvas;
        double min[3], max[3];
        min[0] = xmin; max[0] = xmax;
        min[1] = ymin; max[1] = ymax;
        min[2] = zmin; max[2] = zmax;

        TView3D * v = dynamic_cast<TView3D*>(c->GetView());
        qDebug() << "aaaaaaaaaa" << v;
        if (v) v->SetRange(min, max);
        */
    }

    qApp->processEvents();
    redrawAll();
    //qDebug() << "reshape done";
}

void AGraphWindow::redrawAll()
{  
    enforceOverlayOff();
    updateBasketGUI();

    if (DrawObjects.empty())
    {
        clearRootCanvas();
        updateRootCanvas();
        Explorer->updateGui();
        return;
    }

    for (ADrawObject & obj : DrawObjects)
    {
        QString opt = obj.Options;
        QByteArray ba = opt.toLocal8Bit();
        const char* options = ba.data();

        if (!obj.Options.contains("same", Qt::CaseInsensitive)) updateMargins(&obj);

        if (obj.bEnabled) drawSingleObject(obj.Pointer, options, false);
    }

    qApp->processEvents();
    RasterWindow->fCanvas->Update();
    updateControls();

    fixGraphFrame();
}

void AGraphWindow::clearRegisteredTObjects()
{
    for (TObject * obj : RegisteredTObjects) delete obj;
    RegisteredTObjects.clear();
}

void AGraphWindow::on_cbShowLegend_toggled(bool checked)
{
    if (checked)
        gStyle->SetOptStat(LastOptStat);
    else
    {
        LastOptStat = gStyle->GetOptStat();
        gStyle->SetOptStat("");
    }

    redrawAll();
}

void AGraphWindow::on_pbZoom_clicked()
{
    if (DrawObjects.empty()) return;
    TObject* obj = DrawObjects.front().Pointer;
    QString PlotType = obj->ClassName();
    QString opt = DrawObjects.front().Options;
    //qDebug()<<"  Class name/PlotOptions/opt:"<<PlotType<<opt;
    if (
            PlotType == "TGraph" ||
            PlotType == "TMultiGraph" ||
            PlotType == "TF1" ||
            PlotType.startsWith("TH1") ||
            PlotType == "TProfile" ||
            ( (PlotType.startsWith("TH2") || PlotType == "TProfile2D") && (opt == "" || opt.contains("col", Qt::CaseInsensitive) || opt.contains("prof", Qt::CaseInsensitive)) )
        )
    {
        RasterWindow->Extract2DBox();
        bool ok = RasterWindow->waitForExtractionFinished();
        if (!ok) return;

        ui->ledXfrom->setText(QString::number(RasterWindow->extractedX1, 'g', 4));
        ui->ledXto->  setText(QString::number(RasterWindow->extractedX2, 'g', 4));
        ui->ledYfrom->setText(QString::number(RasterWindow->extractedY1, 'g', 4));
        ui->ledYto->  setText(QString::number(RasterWindow->extractedY2, 'g', 4));

        reshape();
        if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
    }

    updateControls();
}

void AGraphWindow::on_pbUnzoom_clicked()
{
    if (DrawObjects.empty()) return;

    TObject* obj = DrawObjects.front().Pointer;

    TH1 * h = dynamic_cast<TH1*>(obj);
    if (h)
    {
        h->GetXaxis()->UnZoom();
        h->GetYaxis()->UnZoom();
        if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
    }
    else
    {
        TGraph * gr = dynamic_cast<TGraph*>(obj);
        if (gr)
        {
            gr->GetXaxis()->UnZoom(); //does not work!
            gr->GetYaxis()->UnZoom();
            if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
        }
    }

    /*
  else if (PlotType == "TGraph2D")
    {
      if (RasterWindow->fCanvas->GetView())
        {
          RasterWindow->fCanvas->GetView()->UnZoom();
          RasterWindow->fCanvas->GetView()->Modify();
        }
    }
  else if (PlotType == "TGraph")// || PlotType == "TMultiGraph" || PlotType == "TF1" || PlotType == "TF2")
  {
  }
  else
    {
      qDebug() << "Unzoom is not implemented for this object type:"<<PlotType;
      return;
    }
  */

    RasterWindow->fCanvas->Modified();
    RasterWindow->fCanvas->Update();
    updateControls();
}

void AGraphWindow::on_leOptions_editingFinished()
{   
    const QString newOptions = ui->leOptions->text();

    if (DrawObjects.empty()) return;
    if (DrawObjects.front().Options != newOptions)
    {
        DrawObjects.front().Options = newOptions;
        redrawAll();

        if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
    }
}

void AGraphWindow::saveGraph(const QString & fileName)
{
    RasterWindow->saveAs(fileName);
}

void AGraphWindow::updateControls()
{
    if (DrawObjects.empty()) return;

    //qDebug()<<"  GraphWindow: updating indication of ranges";
    TMPignore = true;

    TCanvas* c = RasterWindow->fCanvas;
    ui->cbLogX->setChecked(c->GetLogx());
    ui->cbLogY->setChecked(c->GetLogy());
    ui->cbGridX->setChecked(c->GetGridx());
    ui->cbGridY->setChecked(c->GetGridy());

    TObject* obj = DrawObjects.front().Pointer;
    if (!obj)
    {
        qWarning() << "Cannot update graph window rang controls - object does not exist";
        return;
    }
    QString PlotType = obj->ClassName();
    QString opt = DrawObjects.front().Options;
    //qDebug() << "PlotType:"<< PlotType << "Opt:"<<opt;

    zmin = 0; zmax = 0;
    if (PlotType.startsWith("TH1") || PlotType.startsWith("TH2") || PlotType =="TProfile")
    {
        c->GetRangeAxis(xmin, ymin, xmax, ymax);
        if (c->GetLogx())
        {
            xmin = TMath::Power(10.0, xmin);
            xmax = TMath::Power(10.0, xmax);
        }
        if (c->GetLogy())
        {
            ymin = TMath::Power(10.0, ymin);
            ymax = TMath::Power(10.0, ymax);
        }

        if (PlotType.startsWith("TH2") )
        {
            if (ui->leOptions->text().startsWith("col"))
            {
                //it is color contour - 2D plot
                zmin = ((TH2*) obj)->GetMinimum();
                zmax = ((TH2*) obj)->GetMaximum();
                ui->ledZfrom->setText( QString::number(zmin, 'g', 4) );
                ui->ledZto->setText( QString::number(zmax, 'g', 4) );
            }
            else
            {
                //3D plot
                float min[3], max[3];
                TView* v = c->GetView();
//                if (v && !MW->ShutDown)     !!!***
                if (v)
                {
                    v->GetRange(min, max);
                    ui->ledZfrom->setText( QString::number(min[2], 'g', 4) );
                    ui->ledZto->setText( QString::number(max[2], 'g', 4) );
                }
                else
                {
                    ui->ledZfrom->setText("");
                    ui->ledZto->setText("");
                }
            }
        }
    }
    else if (PlotType.startsWith("TH3"))
    {
        ui->ledZfrom->setText( "" );   //   ui->ledZfrom->setText( QString::number(zmin, 'g', 4) );
        ui->ledZto->setText( "" ); // ui->ledZto->setText( QString::number(zmax, 'g', 4) );
    }
    else if (PlotType.startsWith("TProfile2D"))
    {
        if (opt == "" || opt == "prof" || opt.contains("col") || opt.contains("colz"))
        {
            c->GetRangeAxis(xmin, ymin, xmax, ymax);
            if (c->GetLogx())
            {
                xmin = TMath::Power(10.0, xmin);
                xmax = TMath::Power(10.0, xmax);
            }
            if (c->GetLogy())
            {
                ymin = TMath::Power(10.0, ymin);
                ymax = TMath::Power(10.0, ymax);
            }
        }
        ui->ledZfrom->setText( "" );   //   ui->ledZfrom->setText( QString::number(zmin, 'g', 4) );
        ui->ledZto->setText( "" ); // ui->ledZto->setText( QString::number(zmax, 'g', 4) );
    }
    else if (PlotType.startsWith("TF1") )
    {
        //cannot use GetRange - y is reported 0 always
        //      xmin = ((TF1*) obj)->GetXmin();
        //      xmax = ((TF1*) obj)->GetXmax();
        //      ymin = ((TF1*) obj)->GetMinimum();
        //      ymax = ((TF1*) obj)->GetMaximum();
        c->GetRangeAxis(xmin, ymin, xmax, ymax);
        if (c->GetLogx())
        {
            xmin = TMath::Power(10.0, xmin);
            xmax = TMath::Power(10.0, xmax);
        }
        if (c->GetLogy())
        {
            ymin = TMath::Power(10.0, ymin);
            ymax = TMath::Power(10.0, ymax);
        }
    }
    else if (PlotType.startsWith("TF2"))
    {
        ((TF2*) obj)->GetRange(xmin, ymin, xmax, ymax);
        //  zmin = ((TF2*) obj)->GetMinimum();  -- too slow, it involves minimizer!
        //  zmax = ((TF2*) obj)->GetMaximum();
        float min[3], max[3];
        TView* v = c->GetView();
        if (v)// && !MW->ShutDown)
        {
            v->GetRange(min, max);
            ui->ledZfrom->setText( QString::number(min[2], 'g', 4) );
            ui->ledZto->setText( QString::number(max[2], 'g', 4) );
        }
        else
        {
            ui->ledZfrom->setText("");
            ui->ledZto->setText("");
        }
    }
    else if (PlotType == "TGraph" || PlotType == "TGraphErrors" || PlotType == "TMultiGraph")
    {
        c->GetRangeAxis(xmin, ymin, xmax, ymax);
        if (c->GetLogx())
        {
            xmin = TMath::Power(10.0, xmin);
            xmax = TMath::Power(10.0, xmax);
        }
        if (c->GetLogy())
        {
            ymin = TMath::Power(10.0, ymin);
            ymax = TMath::Power(10.0, ymax);
        }
        //   qDebug()<<"---Ymin:"<<ymin;
    }
    else if (PlotType == "TGraph2D")
    {
        float min[3], max[3];
        TView* v = c->GetView();
        if (v)// && !MW->ShutDown)
        {
            v->GetRange(min, max);
            xmin = min[0]; xmax = max[0];
            ymin = min[1]; ymax = max[1];
            zmin = min[2]; zmax = max[2];
            //qDebug() << "minmax XYZ"<<xmin<<xmax<<ymin<<ymax<<zmin<<zmax;
        }
        ui->ledZfrom->setText( QString::number(zmin, 'g', 4) );
        ui->ledZto->setText( QString::number(zmax, 'g', 4) );
    }

    ui->ledXfrom->setText( QString::number(xmin, 'g', 4) );
    xmin = ui->ledXfrom->text().toDouble();  //to have consistent rounding
    ui->ledXto->setText( QString::number(xmax, 'g', 4) );
    xmax = ui->ledXto->text().toDouble();
    ui->ledYfrom->setText( QString::number(ymin, 'g', 4) );
    ymin = ui->ledYfrom->text().toDouble();
    ui->ledYto->setText( QString::number(ymax, 'g', 4) );
    ymax = ui->ledYto->text().toDouble();

    zmin = ui->ledZfrom->text().toDouble();
    zmax = ui->ledZto->text().toDouble();

    TMPignore = false;
    //qDebug()<<"  GraphWindow: updating toolbar done";
}

void AGraphWindow::onDrawRequest(TObject * obj, QString options, bool transferOwnership, bool focusWindow)
{
    if (focusWindow)
    {
        showAndFocus();
        draw(obj, options, true, transferOwnership);
    }
    else
        draw(obj, options, true, transferOwnership);

    lwBasket->clearFocus();
}

void AGraphWindow::onScriptDrawRequest(TObject * obj, QString options, bool fFocus)
{
    DrawFinished = false;

    emit requestLocalDrawObject(obj, options, fFocus);
    do
    {
        QThread::msleep(100);
        QApplication::processEvents();
    }
    while (!DrawFinished);
}

void AGraphWindow::processScriptDrawRequest(TObject *obj, QString options, bool fFocus)
{
    //always drawing a copy, so always need to register the object
    if (fFocus) showAndFocus();
    draw(obj, options.toLatin1().data(), true, true);
}

void SetMarkerAttributes(TAttMarker* m, const QVariantList& vl)
{
    m->SetMarkerColor(vl.at(0).toInt());
    m->SetMarkerStyle(vl.at(1).toInt());
    m->SetMarkerSize (vl.at(2).toDouble());
}
void SetLineAttributes(TAttLine* l, const QVariantList& vl)
{
    l->SetLineColor(vl.at(0).toInt());
    l->SetLineStyle(vl.at(1).toInt());
    l->SetLineWidth(vl.at(2).toDouble());
}

bool AGraphWindow::onScriptDrawTree(TTree * tree, QString what, QString cond, QString how,
                                        QVariantList binsAndRanges, QVariantList markersAndLines, QString * result)
{
    if (what.isEmpty())
    {
        if (result) *result = "\"What\" string is empty!";
        return false;
    }

    QStringList Vars = what.split(":", Qt::SkipEmptyParts);
    int num = Vars.size();
    if (num > 3)
    {
        if (result) *result = "Invalid \"What\" string - there should be 1, 2 or 3 fields separated with \":\" character!";
        return false;
    }

    QString howProc = how;
    const std::vector<QString> vDisreguard = {"func", "same", "pfc", "plc", "pmc", "lego", "col", "candle", "violin", "cont", "list", "cyl", "pol", "scat"};
    for (const QString & s : vDisreguard) howProc.remove(s, Qt::CaseInsensitive);
    bool bHistToGraph = ( num == 2 && ( howProc.contains("L") || howProc.contains("C") ) );
    qDebug() << "Graph instead of hist?"<< bHistToGraph;

    QVariantList defaultBR;
    defaultBR << (int)100 << (double)0 << (double)0;
    QVariantList defaultMarkerLine;
    defaultMarkerLine << (int)602 << (int)1 << (double)1.0;

    //check ups
    QVariantList vlBR;
    for (int i=0; i<3; i++)
    {
        if (i >= binsAndRanges.size())
            vlBR.push_back(defaultBR);
        else vlBR.push_back(binsAndRanges.at(i));

        QVariantList vl = vlBR.at(i).toList();
        if (vl.size() != 3)
        {
            if (result) *result = "Error in BinsAndRanges argument (bad size)";
            return false;
        }
        bool bOK0, bOK1, bOK2;
        vl.at(0).toInt(&bOK0);
        vl.at(0).toDouble(&bOK1);
        vl.at(0).toDouble(&bOK2);
        if (!bOK0 || !bOK1 || !bOK2)
        {
            if (result) *result = "Error in BinsAndRanges argument (conversion problem)";
            return false;
        }
    }
    //  qDebug() << "binsranges:" << vlBR;

    QVariantList vlML;
    for (int i=0; i<2; i++)
    {
        if (i >= markersAndLines.size())
            vlML.push_back(defaultMarkerLine);
        else vlML.push_back(markersAndLines.at(i));

        QVariantList vl = vlML.at(i).toList();
        if (vl.size() != 3)
        {
            if (result) *result = "Error in MarkersAndLines argument (bad size)";
            return false;
        }
        bool bOK0, bOK1, bOK2;
        vl.at(0).toInt(&bOK0);
        vl.at(0).toInt(&bOK1);
        vl.at(0).toDouble(&bOK2);
        if (!bOK0 || !bOK1 || !bOK2)
        {
            if (result) *result = "Error in MarkersAndLines argument (conversion problem)";
            return false;
        }
    }
    //  qDebug() << "markersLine:"<<vlML;

    QString str = what + ">>htemp(";
    for (int i = 0; i < num; i++)
    {
        if (i == 1 && bHistToGraph) break;

        QVariantList br = vlBR.at(i).toList();
        int    bins = br.at(0).toInt();
        double from = br.at(1).toDouble();
        double to   = br.at(2).toDouble();
        str += QString::number(bins) + "," + QString::number(from) + "," + QString::number(to) + ",";
    }
    str.chop(1);
    str += ")";

    TString What = str.toLocal8Bit().data();
    //TString Cond = ( cond.isEmpty() ? "" : cond.toLocal8Bit().data() );
    TString Cond = cond.toLocal8Bit().data();
    //TString How  = (  how.isEmpty() ? "" :  how.toLocal8Bit().data() );
    TString How  = how.toLocal8Bit().data();

    QString howAdj = how;   //( how.isEmpty() ? "goff" : "goff,"+how );
    if (!bHistToGraph) howAdj = "goff," + how;
    TString HowAdj = howAdj.toLocal8Bit().data();

    // -------------Delete old tmp hist if exists---------------
    TObject* oldObj = gDirectory->FindObject("htemp");
    if (oldObj)
    {
        qDebug() << "Old htemp found: "<<oldObj->GetName() << " -> deleting!";
        gDirectory->RecursiveRemove(oldObj);
    }

    // --------------DRAW--------------
    qDebug() << "TreeDraw -> what:" << What << "cuts:" << Cond << "opt:"<<HowAdj;

    // !!!*** REDO THIS BLOCK!!!
    AGraphWindow * tmpWin = nullptr;
    if (bHistToGraph)
    {
        tmpWin = new AGraphWindow(this);
        tmpWin->setAsActiveRootWindow();
    }

    TH1::AddDirectory(true);
    tree->Draw(What, Cond, HowAdj);
    TH1::AddDirectory(false);

    // --------------Checks------------
    TH1* tmpHist = dynamic_cast<TH1*>(gDirectory->Get("htemp"));
    if (!tmpHist)
    {
        qDebug() << "No histogram was generated: check input!";
        if (result) *result = "No histogram was generated: check input!";
        delete tmpWin;
        return false;
    }

    // -------------Formatting-----------
    if (bHistToGraph)
    {
        TGraph *g = dynamic_cast<TGraph*>(gPad->GetPrimitive("Graph"));
        if (!g)
        {
            qDebug() << "Graph was not generated: check input!";
            if (result) *result = "No graph was generated: check input!";
            delete tmpWin;
            return false;
        }

        TGraph* clone = new TGraph(*g);
        if (clone)
        {
            if (clone->GetN() > 0)
            {
                const QVariantList xx = vlBR.at(0).toList();
                double min = xx.at(1).toDouble();
                double max = xx.at(2).toDouble();
                if (max > min)
                    clone->GetXaxis()->SetLimits(min, max);
                const QVariantList yy = vlBR.at(1).toList();
                min = yy.at(1).toDouble();
                max = yy.at(2).toDouble();
                if (max > min)
                {
                    clone->SetMinimum(min);
                    clone->SetMaximum(max);
                }

                clone->SetTitle(tmpHist->GetTitle());
                SetMarkerAttributes(static_cast<TAttMarker*>(clone), vlML.at(0).toList());
                SetLineAttributes(static_cast<TAttLine*>(clone), vlML.at(1).toList());

                if ( !How.Contains("same", TString::kIgnoreCase) ) How = "A," + How;
                setAsActiveRootWindow();
                showAndFocus();
                draw(clone, How.Data());
            }
            else
            {
                qDebug() << "Empty graph was generated!";
                if (result) *result = "Empty graph was generated!";
                delete tmpWin;
                return false;
            }
        }
    }
    else
    {
        if (tmpHist->GetEntries() == 0)
        {
            qDebug() << "Empty histogram was generated!";
            if (result) *result = "Empty histogram was generated!";
            return false;
        }

        TH1* h = dynamic_cast<TH1*>(tmpHist->Clone(""));

        switch (num)
        {
        case 1:
            h->GetXaxis()->SetTitle(Vars.at(0).toLocal8Bit().data());
            break;
        case 2:
            h->GetYaxis()->SetTitle(Vars.at(0).toLocal8Bit().data());
            h->GetXaxis()->SetTitle(Vars.at(1).toLocal8Bit().data());
            break;
        case 3:
            h->GetZaxis()->SetTitle(Vars.at(0).toLocal8Bit().data());
            h->GetYaxis()->SetTitle(Vars.at(1).toLocal8Bit().data());
            h->GetXaxis()->SetTitle(Vars.at(2).toLocal8Bit().data());
        }

        SetMarkerAttributes(static_cast<TAttMarker*>(h), vlML.at(0).toList());
        SetLineAttributes(static_cast<TAttLine*>(h), vlML.at(1).toList());
        showAndFocus();
        draw(h, How.Data(), true, false);
    }

    if (result) *result = "";
    delete tmpWin;
    return true;
}

void AGraphWindow::changeOverlayMode(bool bOn)
{
    ui->swToolBox->setVisible(bOn);
    ui->swToolBar->setCurrentIndex(bOn ? 1 : 0);
    ui->fBasket->setEnabled(!bOn);
    ui->actionEqualize_scale_XY->setEnabled(!bOn);
    ui->menuPalette->setEnabled(!bOn);
    ui->actionToggle_Explorer_Basket->setEnabled(!bOn);
    ui->actionToggle_toolbar->setEnabled(!bOn);

    if (bOn)
    {
        if (!gvOverlay->isVisible())
        {
            QPixmap map = qApp->screens().first()->grabWindow(RasterWindow->winId());//QApplication::desktop()->winId());
            gvOverlay->resize(RasterWindow->width(), RasterWindow->height());
            gvOverlay->move(RasterWindow->x(), menuBar()->height());
            ToolBoxScene->setSceneRect(0, 0, RasterWindow->width(), RasterWindow->height());
            ToolBoxScene->setBackgroundBrush(map);

            QPointF origin;
            RasterWindow->PixelToXY(0, 0, origin.rx(), origin.ry());
            GraphicsRuler *ruler = ToolBoxScene->getRuler();
            ruler->setOrigin(origin);
            ruler->setScale(RasterWindow->getXperPixel(), RasterWindow->getYperPixel());

            ToolBoxScene->moveToolToVisible();
            setFixedSize(this->size());
            gvOverlay->show();
        }
        ToolBoxScene->moveToolToVisible();
        ToolBoxScene->update(ToolBoxScene->sceneRect());
        gvOverlay->update();
    }
    else
    {
        if (gvOverlay->isVisible())
        {
            gvOverlay->hide();
            setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            RasterWindow->fCanvas->Update();
        }
    }
}

void AGraphWindow::on_pbShowRuler_clicked()
{
    ToolBoxScene->setActiveTool(AToolboxScene::ToolRuler);
    ui->swToolBox->setCurrentIndex(0);
    changeOverlayMode(true);
}

void AGraphWindow::showProjectionTool()
{
    ToolBoxScene->setActiveTool(AToolboxScene::ToolSelBox);
    ui->swToolBox->setCurrentIndex(1);
    changeOverlayMode(true);
}

void AGraphWindow::on_pbExitToolMode_clicked()
{
    changeOverlayMode(false);
}

void AGraphWindow::on_pbToolboxDragMode_clicked()
{
    ui->ledAngle->setText("0");
    ShapeableRectItem *SelBox = ToolBoxScene->getSelBox();
    SelBox->setTrueAngle(0);
    ToolBoxScene->activateItemDrag();
}

void AGraphWindow::on_pbToolboxDragMode_2_clicked()
{
    AGraphWindow::on_pbToolboxDragMode_clicked();
}

void AGraphWindow::selBoxGeometryChanged()
{
    //qDebug() << "selBoxGeometryChanged";
    ShapeableRectItem *SelBox = ToolBoxScene->getSelBox();

    double scaleX = RasterWindow->getXperPixel();
    double scaleY = RasterWindow->getYperPixel();
    SelBox->setScale(scaleX, scaleY);

    ui->ledAngle->setText(QString::number( SelBox->getTrueAngle(), 'f', 2 ));
    ui->ledWidth->setText(QString::number( SelBox->getTrueWidth(), 'f', 2 ));
    ui->ledHeight->setText(QString::number( SelBox->getTrueHeight(), 'f', 2 ));

    double x0, y0;
    RasterWindow->PixelToXY(SelBox->pos().x(), SelBox->pos().y(), x0, y0);
    ui->ledXcenter->setText(QString::number(x0, 'f', 2));
    ui->ledYcenter->setText(QString::number(y0, 'f', 2));

    //SelBox->update(SelBox->boundingRect());
    ToolBoxScene->update(ToolBoxScene->sceneRect());
    gvOverlay->update();
}

void AGraphWindow::selBoxResetGeometry(double halfW, double halfH)
{
    double xc, yc; //center
    RasterWindow->PixelToXY(halfW, halfH, xc, yc);
    double x0, y0; //corner
    RasterWindow->PixelToXY(0, 0, x0, y0);

    double trueW = 0.5 * fabs(x0 - xc);
    double trueH = 0.5 * fabs(y0 - yc);

    ShapeableRectItem *SelBox = ToolBoxScene->getSelBox();
    SelBox->setTrueRectangle(trueW, trueH);
    SelBox->setPos(halfW, halfH);
    SelBox->setTrueAngle(0);

    selBoxGeometryChanged();
    selBoxControlsUpdated();
}

void AGraphWindow::selBoxControlsUpdated()
{
    double x0 = ui->ledXcenter->text().toDouble();
    double y0 = ui->ledYcenter->text().toDouble();
    double dx = ui->ledWidth->text().toDouble();
    double dy = ui->ledHeight->text().toDouble();
    double angle = ui->ledAngle->text().toDouble();

    double scaleX = RasterWindow->getXperPixel();
    double scaleY = RasterWindow->getYperPixel();

    ShapeableRectItem *SelBox = ToolBoxScene->getSelBox();
    SelBox->setScale(scaleX, scaleY);
    SelBox->setTrueAngle(angle);
    SelBox->setTrueRectangle(dx, dy);      //-0.5*DX, -0.5*DY, DX, DY);

    int ix, iy;
    RasterWindow->XYtoPixel(x0, y0, ix, iy);
    SelBox->setPos(ix, iy);

    ToolBoxScene->update(ToolBoxScene->sceneRect());
    gvOverlay->update();
}

void AGraphWindow::on_pbSelBoxToCenter_clicked()
{
    ToolBoxScene->resetTool(AToolboxScene::ToolSelBox);
}

void AGraphWindow::on_pbSelBoxFGColor_clicked()
{
    ShapeableRectItem *selbox = ToolBoxScene->getSelBox();
    QColor fg = QColorDialog::getColor(selbox->getForegroundColor(), this, "Choose the projection box's foreground color", QColorDialog::ShowAlphaChannel);
    if(fg.isValid())
        selbox->setForegroundColor(fg);
}

void AGraphWindow::on_pbSelBoxBGColor_clicked()
{
    ShapeableRectItem *selbox = ToolBoxScene->getSelBox();
    QColor bg = QColorDialog::getColor(selbox->getBackgroundColor(), this, "Choose the projection box's background color", QColorDialog::ShowAlphaChannel);
    if(bg.isValid())
        selbox->setBackgroundColor(bg);
}


void AGraphWindow::rulerGeometryChanged()
{
    const GraphicsRuler *ruler = ToolBoxScene->getRuler();
    QPointF p1 = ruler->getP1();
    QPointF p2 = ruler->getP2();

    ui->ledRulerX->setText(QString::number(p1.x(), 'g', 4));
    ui->ledRulerY->setText(QString::number(p1.y(), 'g', 4));
    ui->ledRulerX2->setText(QString::number(p2.x(), 'g', 4));
    ui->ledRulerY2->setText(QString::number(p2.y(), 'g', 4));
    ui->ledRulerDX->setText(QString::number(p2.x()-p1.x(), 'g', 4));
    ui->ledRulerDY->setText(QString::number(p2.y()-p1.y(), 'g', 4));
    ui->ledRulerLen->setText(QString::number(ruler->getLength(), 'g', 4));
    ui->ledRulerAngle->setText(QString::number(ruler->getAngle()*180.0/M_PI, 'g', 4));
    ui->ledRulerTicksLength->setText(QString::number(ruler->getTickLength(), 'g', 4));
}

void AGraphWindow::rulerControlsP1Updated()
{
    ToolBoxScene->getRuler()->setP1(QPointF(ui->ledRulerX->text().toDouble(), ui->ledRulerY->text().toDouble()));
}

void AGraphWindow::rulerControlsP2Updated()
{
    ToolBoxScene->getRuler()->setP2(QPointF(ui->ledRulerX2->text().toDouble(), ui->ledRulerY2->text().toDouble()));
}

void AGraphWindow::rulerControlsLenAngleUpdated()
{    
    GraphicsRuler * ruler = ToolBoxScene->getRuler();
    ruler->setAngle(ui->ledRulerAngle->text().toDouble() * M_PI/180);
    ruler->setLength(ui->ledRulerLen->text().toDouble());
}

void AGraphWindow::on_ledRulerTicksLength_editingFinished()
{
    ToolBoxScene->getRuler()->setTickLength(ui->ledRulerTicksLength->text().toDouble());
}

void AGraphWindow::on_pbRulerFGColor_clicked()
{
    GraphicsRuler *ruler = ToolBoxScene->getRuler();
    QColor fg = QColorDialog::getColor(ruler->getForegroundColor(), this, "Choose the ruler's foreground color", QColorDialog::ShowAlphaChannel);
    if (fg.isValid()) ruler->setForegroundColor(fg);
}

void AGraphWindow::on_pbRulerBGColor_clicked()
{
    GraphicsRuler *ruler = ToolBoxScene->getRuler();
    QColor bg = QColorDialog::getColor(ruler->getBackgroundColor(), this, "Choose the ruler's background color", QColorDialog::ShowAlphaChannel);
    if (bg.isValid()) ruler->setBackgroundColor(bg);
}

void AGraphWindow::on_pbResetRuler_clicked()
{
    ToolBoxScene->resetTool(AToolboxScene::ToolRuler);
}

void AGraphWindow::on_pbXprojection_clicked()
{
    showProjection("x");
}

void AGraphWindow::on_pbYprojection_clicked()
{
    showProjection("y");
}

void AGraphWindow::on_pbDensityDistribution_clicked()
{
    showProjection("dens");
}

void AGraphWindow::on_pbXaveraged_clicked()
{
    showProjection("xAv");
}

void AGraphWindow::on_pbYaveraged_clicked()
{
    showProjection("yAv");
}

void AGraphWindow::showProjection(QString type)
{
    TH2 * h = Explorer->getObjectForCustomProjection();
    if (!h) return;

    selBoxControlsUpdated();
    triggerGlobalBusy(true);

    const int nBinsX = h->GetXaxis()->GetNbins();
    const int nBinsY = h->GetYaxis()->GetNbins();
    double x0 = ui->ledXcenter->text().toDouble();
    double y0 = ui->ledYcenter->text().toDouble();
    double dx = 0.5*ui->ledWidth->text().toDouble();
    double dy = 0.5*ui->ledHeight->text().toDouble();

    const ShapeableRectItem *SelBox = ToolBoxScene->getSelBox();
    double angle = SelBox->getTrueAngle();
    angle *= 3.1415926535/180.0;
    double cosa = cos(angle);
    double sina = sin(angle);

    TH1D * hProjection = nullptr;
    TH1D * hWeights = nullptr;

    if (type=="x" || type=="xAv")
    {
        int nn;
        if (ui->cbProjBoxAutobin->isChecked())
        {
            int n = h->GetXaxis()->GetNbins();
            double binLength = (h->GetXaxis()->GetBinCenter(n) - h->GetXaxis()->GetBinCenter(1))/(n-1);
            nn = 2.0*dx / binLength;
            ui->sProjBins->setValue(nn);
        }
        else nn = ui->sProjBins->value();
        if (type == "x")
            hProjection = new TH1D("X-Projection","X1 projection", nn, -dx, +dx);
        else
        {
            hProjection = new TH1D("X-Av","X1 averaged", nn, -dx, +dx);
            hWeights    = new TH1D("X-W", "",            nn, -dx, +dx);
        }
    }
    else if (type=="y" || type=="yAv")
    {
        int nn;
        if (ui->cbProjBoxAutobin->isChecked())
        {
            int n = h->GetYaxis()->GetNbins();
            double binLength = (h->GetYaxis()->GetBinCenter(n) - h->GetYaxis()->GetBinCenter(1))/(n-1);
            nn = 2.0*dy / binLength;
            ui->sProjBins->setValue(nn);
        }
        else nn = ui->sProjBins->value();
        if (type == "y")
            hProjection = new TH1D("Y-Projection","Y1 projection", nn, -dy, +dy);
        else
        {
            hProjection = new TH1D("Y-Av","Y1 averaged", nn, -dy, +dy);
            hWeights    = new TH1D("Y-W", "",            nn, -dy, +dy);
        }
    }
    else if (type == "dens")
        hProjection = new TH1D("DensDistr","Density distribution", ui->sProjBins->value(), 0, 0);
    else
    {
        triggerGlobalBusy(false);
        return;
    }

    for (int iy = 1; iy<nBinsY+1; iy++)
        for (int ix = 1; ix<nBinsX+1; ix++)
        {
            double x = h->GetXaxis()->GetBinCenter(ix);
            double y = h->GetYaxis()->GetBinCenter(iy);

            //transforming to the selection box coordinates
            x -= x0;
            y -= y0;

            //oposite direction
            double nx =  x*cosa + y*sina;
            double ny = -x*sina + y*cosa;

            //is it within the borders?
            if (  nx < -dx || nx > dx || ny < -dy || ny > dy  )
            {
                //outside!
                //h->SetBinContent(ix, iy, 0);
            }
            else
            {
                double w = h->GetBinContent(ix, iy);
                if (type == "x") hProjection->Fill(nx, w);
                if (type == "xAv")
                {
                    hProjection->Fill(nx, w);
                    hWeights->Fill(nx, 1);
                }
                else if (type == "y") hProjection->Fill(ny, w);
                if (type == "yAv")
                {
                    hProjection->Fill(ny, w);
                    hWeights->Fill(ny, 1);
                }
                else if (type == "dens") hProjection->Fill(w, 1);
            }
        }

    if (type == "x" || type == "y") hProjection->GetXaxis()->SetTitle("Distance, mm");
    else if (type == "dens") hProjection->GetXaxis()->SetTitle("Density, counts");
    if (type == "xAv" || type == "yAv") *hProjection = *hProjection / *hWeights;

    makeCopyOfDrawObjects();
    makeCopyOfActiveBasketId();

    clearBasketActiveId();

    DrawObjects.clear();
    registerTObject(hProjection);
    DrawObjects.push_back( ADrawObject(hProjection, "hist") );

    redrawAll();

    delete hWeights;
    triggerGlobalBusy(false);
}

void AGraphWindow::enforceOverlayOff()
{
    changeOverlayMode(false);
}

void AGraphWindow::on_pbAddToBasket_clicked()
{   
    if (DrawObjects.empty()) return;

    bool ok;
    int row = Basket->size();
    QString name = "Item"+QString::number(row);
    QString text = QInputDialog::getText(this, "New basket item",
                                         "Enter name:", QLineEdit::Normal,
                                         name, &ok);
    if (!ok || text.isEmpty()) return;

    addCurrentToBasket(text);
}

void AGraphWindow::addCurrentToBasket(const QString & name)
{
    if (DrawObjects.empty()) return;
    updateLogScaleFlags(DrawObjects);
    Basket->add(name.simplified(), DrawObjects);
    ui->actionToggle_Explorer_Basket->setChecked(true);
    updateBasketGUI();
}

void AGraphWindow::updateLogScaleFlags(std::vector<ADrawObject> & drawObjects) const
{
    for (ADrawObject & drObj : drawObjects)
    {
        drObj.bLogScaleX = RasterWindow->isLogX();
        drObj.bLogScaleY = RasterWindow->isLogY();
    }
}

void AGraphWindow::drawLegend(double x1, double y1, double x2, double y2, QString title)
{
    TLegend* leg = RasterWindow->fCanvas->BuildLegend(x1, y1, x2, y2, title.toLatin1());

    registerTObject(leg);
    DrawObjects.push_back(ADrawObject(leg, "same"));

    redrawAll();
}

void AGraphWindow::configureLegendBorder(int color, int style, int size)
{
    for (int i=0; i<DrawObjects.size(); i++)
    {
        QString cn = DrawObjects[i].Pointer->ClassName();
        //qDebug() << cn;
        if (cn == "TLegend")
        {
            TLegend* le = dynamic_cast<TLegend*>(DrawObjects[i].Pointer);
            le->SetLineColor(color);
            le->SetLineStyle(style);
            le->SetLineWidth(size);

            redrawAll();
            return;
        }
    }
    qDebug() << "Legend object was not found!";
}

void AGraphWindow::updateBasketGUI()
{
    lwBasket->clear();
    lwBasket->addItems(Basket->getItemNames());

    if (ActiveBasketItem >= Basket->size()) ActiveBasketItem = -1;

    for (int i=0; i < lwBasket->count(); i++)
    {
        QListWidgetItem * item = lwBasket->item(i);
        if (i == ActiveBasketItem)
        {
            item->setForeground(QBrush(Qt::cyan));
            item->setBackground(QBrush(Qt::lightGray));
        }
        else
        {
            item->setForeground(QBrush(Qt::black));
            item->setBackground(QBrush(Qt::white));
        }
    }
    ui->pbUpdateInBasket->setEnabled(ActiveBasketItem >= 0);

    if (ActiveBasketItem < 0) highlightUpdateBasketButton(false);

    if (MGDesigner) MGDesigner->updateBasketGUI();
}

void AGraphWindow::onBasketItemDoubleClicked(QListWidgetItem *)
{
    //qDebug() << "Row double clicked:"<<ui->lwBasket->currentRow();
    switchToBasket(lwBasket->currentRow());
}

void AGraphWindow::onBasketDeleteShortcutActivated()
{
    if ((lwBasket->rect().contains(lwBasket->mapFromGlobal(QCursor::pos()))))
        removeAllSelectedBasketItems();
}

void AGraphWindow::onCursorPositionReceived(double x, double y, bool bOn)
{
    ui->labCursorX->setText(bOn ? QString::number(x, 'g', 4) : "--");
    ui->labCursorY->setText(bOn ? QString::number(y, 'g', 4) : "--");
}

void AGraphWindow::makeCopyOfDrawObjects()
{
    PreviousDrawObjects = DrawObjects;

    // without this fix cloning of legend objects is broken
    //if (!PreviousDrawObjects.isEmpty())
    //    qDebug() << "gcc optimizer fix:" << PreviousDrawObjects.first().Pointer;
}

void AGraphWindow::clearCopyOfDrawObjects()
{
    PreviousDrawObjects.clear();
}

void AGraphWindow::clearBasketActiveId()
{
    ActiveBasketItem = -1;
}

void AGraphWindow::makeCopyOfActiveBasketId()
{
    PreviousActiveBasketItem = ActiveBasketItem;
}

void AGraphWindow::restoreBasketActiveId()
{
    ActiveBasketItem = PreviousActiveBasketItem;
}

void AGraphWindow::clearCopyOfActiveBasketId()
{
    PreviousActiveBasketItem = -1;
}

void AGraphWindow::onBasketCustomContextMenuRequested(const QPoint &pos)
{
    if (lwBasket->selectedItems().size() > 1)
    {
        contextMenuForBasketMultipleSelection(pos);
        return;
    }

    QMenu BasketMenu;

    int row = -1;
    QListWidgetItem* temp = lwBasket->itemAt(pos);

    QAction * switchToThis = nullptr;
    QAction * onTop = nullptr;
    QAction * del = nullptr;
    QAction * rename = nullptr;

    if (temp)
    {
        //menu triggered at a valid item
        row = lwBasket->row(temp);

        BasketMenu.addSeparator();
        onTop = BasketMenu.addAction("Show on top of the main draw");
        switchToThis = BasketMenu.addAction("Switch to this");
        BasketMenu.addSeparator();
        rename = BasketMenu.addAction("Rename");
        BasketMenu.addSeparator();
        del = BasketMenu.addAction("Delete");
        del->setShortcut(Qt::Key_Delete);
        BasketMenu.addSeparator();
    }
    BasketMenu.addSeparator();
    QAction* append = BasketMenu.addAction("Append basket file");
    BasketMenu.addSeparator();
    QAction* appendTxt = BasketMenu.addAction("Append graph from text file");
    QAction* appendTxtEr = BasketMenu.addAction("Append graph with error bars from text file");
    QAction* appendRootHistsAndGraphs = BasketMenu.addAction("Append graphs / histograms from a ROOT file");
    BasketMenu.addSeparator();
    QAction* save = BasketMenu.addAction("Save basket to file");
    BasketMenu.addSeparator();
    QAction* clear = BasketMenu.addAction("Clear basket");

    //------

    QAction* selectedItem = BasketMenu.exec(lwBasket->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    if (selectedItem == switchToThis)
        switchToBasket(row);
    else if (selectedItem == clear)
    {
        QMessageBox msgBox;
        msgBox.setText("Clear basket cannot be undone!");
        msgBox.setInformativeText("Clear basket?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
            clearBasket();
    }
    else if (selectedItem == save)
    {
        QString fileName = guitools::dialogSaveFile(this, "Save basket to a file", "Root files (*.root)");
        if (!fileName.isEmpty())
        {
            if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".root";
            Basket->saveAll(fileName);
        }
    }
    else if (selectedItem == append)
    {
        bool bDrawEmpty = DrawObjects.empty();
        const QString fileName = guitools::dialogLoadFile(this, "Append all from a basket file", "Root files (*.root)");
        if (!fileName.isEmpty())
        {
            QString err = Basket->appendBasket(fileName);
            if (!err.isEmpty()) guitools::message(err, this);
            updateBasketGUI();
            if (bDrawEmpty) switchToBasket(0);
        }
    }
    else if (selectedItem == appendRootHistsAndGraphs)
    {
        const QString fileName = guitools::dialogLoadFile(this, "Append hist and graph objects from ROOT file", "Root files (*.root)");
        if (!fileName.isEmpty())
        {
            Basket->appendRootHistGraphs(fileName);
            updateBasketGUI();
        }
    }
    else if (selectedItem == appendTxt)
    {
        QString fileName = guitools::dialogLoadFile(this, "Append graph from ascii file to basket", "Data files (*.txt *.dat); All files (*.*)");
        if (fileName.isEmpty()) return;
        const QString res = Basket->appendTxtAsGraph(fileName);
        if (!res.isEmpty()) guitools::message(res, this);
        else
            switchToBasket(Basket->size() - 1);
    }
    else if (selectedItem == appendTxtEr)
    {
        QString fileName = guitools::dialogLoadFile(this, "Append graph with errors from ascii file to basket", "Data files (*.txt *.dat); All files (*.*)");
        if (fileName.isEmpty()) return;
        const QString res = Basket->appendTxtAsGraphErrors(fileName);
        if (!res.isEmpty()) guitools::message(res, this);
        else
            switchToBasket(Basket->size() - 1);
    }
    else if (selectedItem == del)
    {
        Basket->remove(row);
        ActiveBasketItem = -1;
        clearCopyOfActiveBasketId();
        updateBasketGUI();
    }
    else if (selectedItem == rename)
    {
        if (row == -1) return; //protection
        bool ok;
        QString text = QInputDialog::getText(this, "Rename basket item",
                                             "Enter new name:", QLineEdit::Normal,
                                             Basket->getName(row), &ok);
        if (ok && !text.isEmpty())
            Basket->rename(row, text.simplified());
        updateBasketGUI();
    }
    else if (selectedItem == onTop)
        basket_DrawOnTop(row);
}

void AGraphWindow::onBasketReorderRequested(const std::vector<int> & indexes, int toRow)
{
    Basket->reorder(indexes, toRow);
    ActiveBasketItem = -1;
    clearCopyOfActiveBasketId();
    updateBasketGUI();
}

void AGraphWindow::contextMenuForBasketMultipleSelection(const QPoint & pos)
{
    QMenu Menu;
    QAction * multidrawA = Menu.addAction("Make multidraw");
    QAction * mergeA = Menu.addAction("Merge histograms");
    QAction * removeAllSelected = Menu.addAction("Remove all selected");
    removeAllSelected->setShortcut(Qt::Key_Delete);

    QAction* selectedItem = Menu.exec(lwBasket->mapToGlobal(pos));
    if (!selectedItem) return;

    if      (selectedItem == removeAllSelected) removeAllSelectedBasketItems();
    else if (selectedItem == multidrawA)        requestMultidraw();
    else if (selectedItem == mergeA)            requestMergeHistograms();
}

void AGraphWindow::removeAllSelectedBasketItems()
{
    const QList<QListWidgetItem*> selection = lwBasket->selectedItems();
    const int size = selection.size();
    if (size == 0) return;

    bool bConfirm = true;
    if (size > 1)
        bConfirm = guitools::confirm(QString("Remove selected %1 item%2 from the basket?").arg(size).arg(size == 1 ? "" : "s"), this);
    if (!bConfirm) return;

    std::vector<int> indexes;
    for (const QListWidgetItem * item : selection)
        indexes.push_back(lwBasket->row(item));
    std::sort(indexes.begin(), indexes.end());
    for (int i = indexes.size() - 1; i >= 0; i--)
        Basket->remove(indexes[i]);

    ActiveBasketItem = -1;
    clearCopyOfActiveBasketId();
    updateBasketGUI();
}

void AGraphWindow::onExternalBasketChange()
{
    ActiveBasketItem = -1;
    clearCopyOfActiveBasketId();
    updateBasketGUI();
}

void AGraphWindow::createMGDesigner()
{
    if (!MGDesigner)
    {
        MGDesigner = new AMultiGraphDesigner(*Basket, this);
        connect(MGDesigner, &AMultiGraphDesigner::basketChanged, this, &AGraphWindow::onExternalBasketChange);
    }
}

void AGraphWindow::requestMultidraw()
{
    const QList<QListWidgetItem*> selection = lwBasket->selectedItems();

    std::vector<int> indexes;
    for (const QListWidgetItem * const item : selection)
        indexes.push_back( lwBasket->row(item) );

    if (!MGDesigner) createMGDesigner();
    MGDesigner->showNormal();
    MGDesigner->activateWindow();
    MGDesigner->requestAutoconfigureAndDraw(indexes);
}

void AGraphWindow::requestMergeHistograms()
{
    const QList<QListWidgetItem*> selection = lwBasket->selectedItems();

    std::vector<int> indexes;
    for (const QListWidgetItem * const item : selection)
        indexes.push_back(lwBasket->row(item));

    if (indexes.size() < 2) return;

    QString err = Basket->mergeHistograms(indexes);

    if (!err.isEmpty())
        guitools::message(err, this);
    else
        switchToBasket(Basket->size() - 1);
}

void AGraphWindow::clearBasket()
{
    Basket->clear();
    ActiveBasketItem = -1;
    clearCopyOfActiveBasketId();
    updateBasketGUI();
}

void AGraphWindow::on_actionBasic_ROOT_triggered()
{
    gStyle->SetPalette(57);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionDeep_sea_triggered()
{
    gStyle->SetPalette(51);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionGrey_scale_triggered()
{
    gStyle->SetPalette(52);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionDark_body_radiator_triggered()
{
    gStyle->SetPalette(53);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionTwo_color_hue_triggered()
{
    gStyle->SetPalette(54);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionRainbow_triggered()
{
    gStyle->SetPalette(55);
    AGraphWindow::redrawAll();
}

void AGraphWindow::on_actionInverted_dark_body_triggered()
{
    gStyle->SetPalette(56);
    AGraphWindow::redrawAll();
}

void AGraphWindow::basket_DrawOnTop(int row)
{
    if (row == -1) return;
    if (DrawObjects.empty()) return;

    makeCopyOfDrawObjects();
    makeCopyOfActiveBasketId();

    //qDebug() << "Basket item"<<row<<"was requested to be drawn on top of the current draw";
    const std::vector<ADrawObject> DeepCopyBasketDrawObjects = Basket->getCopy(row);

    for (int iObj = 0; iObj < DeepCopyBasketDrawObjects.size(); iObj++)
    {
        TString CName = DeepCopyBasketDrawObjects[iObj].Pointer->ClassName();
        if ( CName== "TLegend" || CName == "TPaveText")
        {
            //qDebug() << CName;
            delete DeepCopyBasketDrawObjects[iObj].Pointer;
            continue;
        }
        QString options = DeepCopyBasketDrawObjects[iObj].Options;
        options.replace("same", "", Qt::CaseInsensitive);
        options.replace("a", "", Qt::CaseInsensitive);
        TString safe = "same";
        safe += options.toLatin1().data();
        //qDebug() << "New options:"<<safe;
        DrawObjects.push_back( ADrawObject(DeepCopyBasketDrawObjects[iObj].Pointer, safe) );
    }

    ActiveBasketItem = -1;
    updateBasketGUI();

    redrawAll();
}

void AGraphWindow::on_actionTop_triggered()
{
    setAsActiveRootWindow();
    TView* v = RasterWindow->fCanvas->GetView();
    if (v) v->TopView();
}

void AGraphWindow::on_actionSide_triggered()
{
    setAsActiveRootWindow();
    TView* v = RasterWindow->fCanvas->GetView();
    if (v) v->SideView();
}

void AGraphWindow::on_actionFront_triggered()
{
    setAsActiveRootWindow();
    TView* v = RasterWindow->fCanvas->GetView();
    if (v) v->FrontView();
}

void AGraphWindow::on_actionToggle_toolbar_triggered(bool checked)
{
    ui->fUIbox->setVisible(checked);
}

void AGraphWindow::on_actionEqualize_scale_XY_triggered()
{
    if (DrawObjects.empty()) return;
    QString ClassName = DrawObjects.front().Pointer->ClassName();
    if (!ClassName.startsWith("TH2") && !ClassName.startsWith("TF2") && !ClassName.startsWith("TGraph2D"))
    {
        guitools::message("Supported only for 2D view", this);
        return;
    }

//    MW->WindowNavigator->BusyOn();  !!!***

    double XperP = fabs(RasterWindow->getXperPixel());
    double YperP = fabs(RasterWindow->getYperPixel());
    double CanvasWidth = RasterWindow->width();
    double NewCanvasWidth = CanvasWidth * XperP/YperP;
    double delta = NewCanvasWidth - CanvasWidth;
    resize(width()+delta, height());

    XperP = fabs(RasterWindow->getXperPixel());
    YperP = fabs(RasterWindow->getYperPixel());
    if (XperP != YperP)
    {
        bool XlargerY = (XperP > YperP);
        do
        {
            if (XperP<YperP) this->resize(this->width()-1, this->height());
            else this->resize(this->width()+1, this->height());
            updateRootCanvas();
            qApp->processEvents();

            XperP = fabs(RasterWindow->getXperPixel());
            YperP = fabs(RasterWindow->getYperPixel());
            if (XperP == YperP) break;
            if ( (XperP > YperP) != XlargerY ) break;
        }
        while ( isVisible() && width()>200 && width()<2000);
    }

//    MW->WindowNavigator->BusyOff();  !!!***
}

void AGraphWindow::on_ledRulerDX_editingFinished()
{
    GraphicsRuler *ruler = ToolBoxScene->getRuler();
    ruler->setDX(ui->ledRulerDX->text().toDouble());
}

void AGraphWindow::on_ledRulerDY_editingFinished()
{
    GraphicsRuler *ruler = ToolBoxScene->getRuler();
    ruler->setDY(ui->ledRulerDY->text().toDouble());
}

void AGraphWindow::on_cbShowFitParameters_toggled(bool checked)
{
    if (checked) gStyle->SetOptFit(1);
    else gStyle->SetOptFit(0);
}

TLegend * AGraphWindow::addLegend()
{
    TLegend * leg = RasterWindow->fCanvas->BuildLegend();
    registerTObject(leg);
    DrawObjects.push_back( ADrawObject(leg, "same") );
    redrawAll();
    return leg;
}

void AGraphWindow::on_pbAddLegend_clicked()
{
    showAddLegendDialog();
}

#include "alegenddialog.h"
void AGraphWindow::showAddLegendDialog()
{
    if (DrawObjects.empty()) return;

    TLegend * leg = nullptr;
    for (size_t i = 0; i < DrawObjects.size(); i++)
    {
        QString cn = DrawObjects[i].Pointer->ClassName();
        if (cn == "TLegend")
        {
            leg = dynamic_cast<TLegend*>(DrawObjects[i].Pointer);
            break;
        }
    }

    if (!leg ) leg = addLegend();

    ALegendDialog Dialog(*leg, DrawObjects, this);
    connect(&Dialog, &ALegendDialog::requestCanvasUpdate, RasterWindow, &ARasterWindow::updateRootCanvas);
    Dialog.exec();
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::on_pbRemoveLegend_clicked()
{
    bool bOK = guitools::confirm("Remove legend?", this);
    if (!bOK) return;

    for (size_t i = 0; i < DrawObjects.size(); i++)
    {
        QString cn = DrawObjects[i].Pointer->ClassName();
        if (cn == "TLegend")
        {
            DrawObjects.erase(DrawObjects.begin()+i);
            redrawAll();
            if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
            break;
        }
    }
}

void AGraphWindow::on_pbAddText_clicked()
{
    addTextPanel("Text", true, 0);
    Explorer->activateCustomGuiForItem(DrawObjects.size() - 1);
    if (ActiveBasketItem != -1) highlightUpdateBasketButton(true);
}

void AGraphWindow::addTextPanel(QString text, bool bShowFrame, int alignLeftCenterRight,
                                double x1, double y1, double x2, double y2, QString options)
{
    TPaveText * la = new TPaveText(x1, y1, x2, y2, options.toLatin1().data());
    la->SetFillColor(0);
    la->SetBorderSize(bShowFrame ? 1 : 0);
    la->SetLineColor(1);
    la->SetTextAlign( (alignLeftCenterRight + 1) * 10 + 2);

    const QStringList sl = text.split("\n");
    for (const QString & s : sl) la->AddText(s.toLatin1());

    draw(la, "same", true, false); //it seems the Paveltext is owned by drawn object - registration causes crash if used with non-registered object (e.g. script)
}

void AGraphWindow::setStatPanelVisible(bool flag)
{
    ui->cbShowLegend->setChecked(flag);
}

void AGraphWindow::triggerGlobalBusy(bool flag)
{
//    if (flag) MW->WindowNavigator->BusyOn();  !!!***
//    else      MW->WindowNavigator->BusyOff(); !!!***
}

void AGraphWindow::on_ledAngle_customContextMenuRequested(const QPoint &pos)
{
    QMenu Menu;

    QAction* alignXWithRuler =Menu.addAction("Align X axis with the Ruler tool");
    QAction* alignYWithRuler =Menu.addAction("Align Y axis with the Ruler tool");

    QAction* selectedItem = Menu.exec(ui->ledAngle->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    double angle = ToolBoxScene->getRuler()->getAngle() *180.0/M_PI;

    if (selectedItem == alignXWithRuler)
    {
        ui->ledAngle->setText( QString::number(angle, 'g', 4) );
        selBoxControlsUpdated();
    }
    else if (selectedItem == alignYWithRuler)
    {
        ui->ledAngle->setText( QString::number(angle - 90.0, 'g', 4) );
        selBoxControlsUpdated();
    }
}

void AGraphWindow::on_pbBackToLast_clicked()
{
    DrawObjects = PreviousDrawObjects;
    PreviousDrawObjects.clear();
    ActiveBasketItem = PreviousActiveBasketItem;
    PreviousActiveBasketItem = -1;

    redrawAll();
    updateBasketGUI();
}

void AGraphWindow::on_actionToggle_Explorer_Basket_toggled(bool arg1)
{
    int w = ui->fBasket->width();
    if (!arg1) w = -w;
    resize(this->width() + w, this->height());

    ui->fBasket->setVisible(arg1);
}

void AGraphWindow::switchToBasket(int index)
{
    if (index < 0 || index >= Basket->size()) return;

    DrawObjects = Basket->getCopy(index);
    redrawAll();

    if (!DrawObjects.empty())
    {
        ui->cbLogX->setChecked(DrawObjects.front().bLogScaleX);
        ui->cbLogY->setChecked(DrawObjects.front().bLogScaleY);
    }

    ActiveBasketItem = index;
    clearCopyOfActiveBasketId();
    clearCopyOfDrawObjects();
    updateBasketGUI();
    highlightUpdateBasketButton(false);
}

void AGraphWindow::on_pbUpdateInBasket_clicked()
{
    highlightUpdateBasketButton(false);

    if (ActiveBasketItem < 0 || ActiveBasketItem >= Basket->size()) return;
    updateLogScaleFlags(DrawObjects);
    Basket->update(ActiveBasketItem, DrawObjects);
}

void AGraphWindow::on_actionShow_ROOT_attribute_panel_triggered()
{
    RasterWindow->fCanvas->SetLineAttributes();
}

void AGraphWindow::on_actionSet_width_triggered()
{
    int w = width();
    guitools::inputInteger("Enter new width:", w, 200, 10000, this);
    this->resize(w, height());
}

void AGraphWindow::on_actionSet_height_triggered()
{
    int h = height();
    guitools::inputInteger("Enter new height:", h, 200, 10000, this);
    this->resize(width(), h);
}

void AGraphWindow::on_actionMake_square_triggered()
{
    double CanvasWidth = RasterWindow->width();
    double CanvasHeight = RasterWindow->height();

    resize(width() + (CanvasHeight - CanvasWidth), height());

    int protectionCounter = 0;
    while (RasterWindow->width() != RasterWindow->height())
    {
        CanvasWidth = RasterWindow->width();
        CanvasHeight = RasterWindow->height();

        if (CanvasWidth > CanvasHeight) resize(width() - 1, height());
        else resize(width()+1, height());
        updateRootCanvas();
        qApp->processEvents();

        if (width() < 200 || width() > 2000) break;
        protectionCounter++;
        if (protectionCounter > 100) break;
    }
}

void AGraphWindow::on_actionCreate_template_triggered()
{
    if (DrawObjects.empty()) return;

    std::vector<std::pair<double,double>> limits = {std::pair<double,double>(xmin, xmax), std::pair<double,double>(ymin, ymax), std::pair<double,double>(zmin, zmax)};
    DrawTemplate.createFrom(DrawObjects, limits); // it seems TH1 does not contain data on the shown range for Y (and Z) axes ... -> using inidcated range!
}

void AGraphWindow::on_actionApply_template_triggered()
{
    applyTemplate(true);
}

void AGraphWindow::applyTemplate(bool bAll)
{
    if (DrawObjects.empty()) return;

    if (DrawTemplate.hasLegend())
    {
        const ATemplateSelectionRecord * legend_rec = DrawTemplate.findRecord("Legend attributes", &DrawTemplate.Selection);
        if (legend_rec && legend_rec->bSelected)
        {
            TLegend * Legend = nullptr;
            for (ADrawObject & obj : DrawObjects)
            {
                Legend = dynamic_cast<TLegend*>(obj.Pointer);
                if (Legend) break;
            }
            if (!Legend) //cannot build legend inside Template due to limitations in ROOT (problems with positioning if TCanvas is not involved)
                Legend = addLegend();
        }
    }

    std::vector<std::pair<double,double>> XYZ_ranges;
    DrawTemplate.applyTo(DrawObjects, XYZ_ranges, bAll);
    redrawAll();

    //everything but ranges is already applied
    const ATemplateSelectionRecord * range_rec = DrawTemplate.findRecord("Ranges", &DrawTemplate.Selection);
    if (range_rec && range_rec->bSelected)
    {
        const ATemplateSelectionRecord * X_rec = DrawTemplate.findRecord("X range", range_rec);
        if (X_rec && X_rec->bSelected)
        {
            ui->ledXfrom->setText( QString::number(XYZ_ranges[0].first,  'g', 4) );
            ui->ledXto->  setText( QString::number(XYZ_ranges[0].second, 'g', 4) );
        }
        const ATemplateSelectionRecord * Y_rec = DrawTemplate.findRecord("Y range", range_rec);
        if (Y_rec && Y_rec->bSelected)
        {
            ui->ledYfrom->setText( QString::number(XYZ_ranges[1].first,  'g', 4) );
            ui->ledYto->  setText( QString::number(XYZ_ranges[1].second, 'g', 4) );
        }
        const ATemplateSelectionRecord * Z_rec = DrawTemplate.findRecord("Z range", range_rec);
        if (Z_rec && Z_rec->bSelected)
        {
            if (ui->ledZfrom->isEnabled() && !ui->ledZfrom->text().isEmpty())
            {
                ui->ledZfrom->setText( QString::number(XYZ_ranges[2].first,  'g', 4) );
                ui->ledZto->  setText( QString::number(XYZ_ranges[2].second, 'g', 4) );
            }
        }
        reshape();
    }

    highlightUpdateBasketButton(true);
}

void AGraphWindow::highlightUpdateBasketButton(bool flag)
{
    QIcon icon;
    if (flag && ui->pbUpdateInBasket->isEnabled())
        icon = guitools::createColorCircleIcon(QSize(15,15), Qt::yellow);
    ui->pbUpdateInBasket->setIcon(icon);
}

QString AGraphWindow::useProjectionTool(const QString & option)
{
    if (DrawObjects.empty()) return "Graph window is empty";
    TH2 * hist = dynamic_cast<TH2*>(DrawObjects[0].Pointer);
    if (!hist) return "Currently drawn object has to be TH2";

    Explorer->customProjection(DrawObjects[0]);
    showProjection(option);
    return "";
}

void AGraphWindow::configureProjectionTool(double x0, double y0, double dx, double dy, double angle)
{
    ui->ledXcenter->setText(QString::number(x0));
    ui->ledYcenter->setText(QString::number(y0));
    ui->ledWidth->  setText(QString::number(dx));
    ui->ledHeight-> setText(QString::number(dy));
    ui->ledAngle->  setText(QString::number(angle));

    selBoxControlsUpdated();
}

void AGraphWindow::close3DviewWindow()
{
    if (Viewer3D)
    {
        Viewer3D->close();
        delete Viewer3D; Viewer3D = nullptr;
    }
}

#include "atemplateselectiondialog.h"
void AGraphWindow::on_actionApply_selective_triggered()
{
    ATemplateSelectionDialog D(DrawTemplate.Selection, this);
    int res = D.exec();
    if (res == QDialog::Accepted)
        applyTemplate(false);
}

void AGraphWindow::on_actionShow_first_drawn_object_context_menu_triggered()
{
    if (DrawObjects.empty())
    {
        guitools::message("Nothing is drawn!", this);
        return;
    }

    const QPoint pos = mapToGlobal(QPoint(0, menuBar()->height()));
    Explorer->showObjectContextMenu(pos, 0);
}

void AGraphWindow::on_pbManipulate_clicked()
{
    Explorer->manipulateTriggered();
}

void AGraphWindow::on_actionOpen_MultiGraphDesigner_triggered()
{
    if (!MGDesigner) createMGDesigner();
    MGDesigner->showNormal();
    MGDesigner->activateWindow();
    //MGDesigner->updateGUI();
}

void AGraphWindow::show3D(QString castorFileName, bool keepSettings)
{
    // Intended for showing Castor images
    bool doRestore = keepSettings && (bool)Viewer3D;
    QJsonObject js1;
    if (Viewer3D)
    {
        Viewer3D->Settings.writeToJson(js1);
        delete Viewer3D;
    }
    Viewer3D = new AViewer3D(this);
    connect(Viewer3D, &AViewer3D::requestMakeCopy,       this, &AGraphWindow::onRequestMakeCopyViewer3D);
    connect(Viewer3D, &AViewer3D::requestExportToBasket, this, &AGraphWindow::addObjectToBasket);

    if (doRestore) Viewer3D->Settings.readFromJson(js1);
    bool ok = Viewer3D->loadCastorImage(castorFileName);
    if (ok) Viewer3D->showNormal();
}

void AGraphWindow::addObjectToBasket(TObject * obj, QString options, QString name)
{
    qDebug() << "Requested to add object" << obj << "with options" << options << "as" << name;

    std::vector<ADrawObject> tmp;
    tmp.push_back( ADrawObject(obj, options) );
    updateLogScaleFlags(tmp);
    Basket->add(name.simplified(), tmp);
    ui->actionToggle_Explorer_Basket->setChecked(true);
    updateBasketGUI();
}

#include "aviewer3dsettings.h"
void AGraphWindow::onRequestMakeCopyViewer3D(AViewer3D * ptr)
{
    AViewer3D * view = new AViewer3D(this);
    view->setWindowTitle("3D viewer (copy)");
    connect(view, &AViewer3D::requestMakeCopy,       this, &AGraphWindow::onRequestMakeCopyViewer3D);
    connect(view, &AViewer3D::requestExportToBasket, this, &AGraphWindow::addObjectToBasket);

    qApp->processEvents();

    // Data
    {
        QJsonObject json;
        ptr->writeDataToJson(json);
        view->readDataFromJson(json);
        view->initViewers();
    }

    // Settings
    {
        QJsonObject json;
        ptr->Settings.writeToJson(json);
        view->Settings.readFromJson(json);
    }

    // Plane viewers
    {
        QJsonObject json;
        ptr->writeViewersToJson(json);
        view->readViewersFromJson(json);
    }

    QString title = ptr->getTitle();
    if (title.isEmpty()) title = "Copy";
    else title = "Copy of " + title;
    view->setTitle(title);

    view->updateGui();
    view->showNormal();
    view->move(ptr->x() + 50, ptr->y() + 50);
    view->activateWindow();

    view->resize(ptr->width(), ptr->height());
}

void AGraphWindow::on_actionSet_default_margins_triggered()
{
    A3Global & GlobSet = A3Global::getInstance();

    ASetMarginsDialog d(GlobSet.DefaultDrawMargins, ADrawMarginsRecord(), this);
    int res = d.exec();
    if (res == QDialog::Accepted)
    {
        GlobSet.DefaultDrawMargins = d.getResult();
        updateMargins();
        doRedrawOnUpdateMargins();
    }
}

void AGraphWindow::doRedrawOnUpdateMargins()
{
    //ClearRootCanvas();
    //UpdateRootCanvas();
    // bug in this toot version: Z axis is not moved after plain redraw!

    if (!DrawObjects.empty())
    {
        QString oldOpt = DrawObjects.front().Options;
        QString opt = oldOpt;
        if (opt.contains("z", Qt::CaseInsensitive))
        {
            opt.remove("z", Qt::CaseInsensitive);
            DrawObjects.front().Options = opt;
            redrawAll();
            DrawObjects.front().Options = oldOpt;
        }
    }
    redrawAll();
}

void AGraphWindow::updateMargins(ADrawObject * obj)
{
    ADrawMarginsRecord rec;

    if (!obj || !obj->CustomMargins.Override)
        rec = A3Global::getConstInstance().DefaultDrawMargins;
    else rec = obj->CustomMargins;

    RasterWindow->fCanvas->SetTopMargin(rec.Top);
    RasterWindow->fCanvas->SetBottomMargin(rec.Bottom);
    RasterWindow->fCanvas->SetLeftMargin(rec.Left);

    bool hitWithZ = false;
    QString opt = "";
    if (obj) opt = obj->Options;
    if (opt.contains("z", Qt::CaseInsensitive)) hitWithZ = true;

    double right = (hitWithZ ? rec.RightForZ : rec.Right);
    RasterWindow->fCanvas->SetRightMargin(right);
}

void AGraphWindow::on_pbSaveImage_clicked()
{
    QString fileName = guitools::dialogSaveFile(this, "Save image as file", "png (*.png);;gif (*.gif);;Jpg (*.jpg)");
    if (fileName.isEmpty()) return;

    QFileInfo file(fileName);
    if (file.suffix().isEmpty()) fileName += ".png";

    AGraphWindow::saveGraph(fileName);
}

#include <QApplication>
#include <QClipboard>
void AGraphWindow::on_pbSaveImage_customContextMenuRequested(const QPoint &)
{
    on_actionCopy_image_to_clipboard_triggered();
}

void AGraphWindow::on_actionSave_image_2_triggered()
{
    on_pbSaveImage_clicked();
}

void AGraphWindow::on_actionCopy_image_to_clipboard_triggered()
{
    RasterWindow->saveAs("tmpImage.png");
    QImage image("tmpImage.png");
    QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

#include "ahistoptstatdialog.h"
void AGraphWindow::on_actionSet_histogram_stat_box_content_triggered()
{
    AHistOptStatDialog dia(this);
    dia.exec();
}

#include "apaletteselectiondialog.h"
void AGraphWindow::on_actionSet_palette_triggered()
{
    APaletteSelectionDialog dia(this);
    connect(&dia, &APaletteSelectionDialog::requestRedraw, this, &AGraphWindow::redrawAll);
    dia.exec();
}
