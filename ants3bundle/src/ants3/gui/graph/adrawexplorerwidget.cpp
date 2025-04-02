#include "adrawexplorerwidget.h"
#include "arootmarkerconfigurator.h"
#include "arootlineconfigurator.h"
#include "guitools.h"
#include "agraphwindow.h"
#include "afiletools.h"
#include "alinemarkerfilldialog.h"
#include "agraphrasterwindow.h"
#include "atextpavedialog.h"
#include "amultidrawrecord.h"

#ifdef USE_EIGEN
    #include "curvefit.h"
#endif

#include <QDebug>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QColor>

#include "TObject.h"
#include "TPaveText.h"
#include "TNamed.h"
#include "TAttMarker.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "TH1.h"
#include "TH2.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TPaveText.h"
#include "TGaxis.h"
#include "TColor.h"
#include "TROOT.h"

ADrawExplorerWidget::ADrawExplorerWidget(AGraphWindow & GraphWindow, std::vector<ADrawObject> &DrawObjects) :
    GraphWindow(GraphWindow), Raster(*GraphWindow.RasterWindow), DrawObjects(DrawObjects)
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &ADrawExplorerWidget::customContextMenuRequested, this, &ADrawExplorerWidget::onContextMenuRequested);
    connect(this, &ADrawExplorerWidget::itemDoubleClicked, this, &ADrawExplorerWidget::onItemDoubleClicked);

    setIndentation(0);

    setSelectionMode(QAbstractItemView::SingleSelection);
    //setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
}

void ADrawExplorerWidget::updateGui()
{
    clear();
    objForCustomProjection = nullptr;

    if (!DrawObjects.empty() && DrawObjects.front().Multidraw)
    {
        updateGui_multidrawMode();
        return;
    }

    for (int i = 0; i < DrawObjects.size(); i++)
    {
        const ADrawObject & drObj = DrawObjects.at(i);
        TObject * tObj = drObj.Pointer;

        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString name = drObj.Name;

        QString nameShort = name;
        if (nameShort.size() > 15)
        {
            nameShort.truncate(15);
            nameShort += "..";
        }

        QString className = tObj->ClassName();
        QString opt = drObj.Options;

        if (className == "TPaveText") className = "TextBox";
        else if (className == "TLegend") className = "Legend";

        item->setText(0, QString("%1 %2").arg(nameShort).arg(className));
        item->setToolTip(0, QString("Name: %1\nClassName: %2\nDraw options: %3").arg(name).arg(className).arg(opt));
        item->setText(1, QString::number(i));

        if (!drObj.bEnabled) item->setForeground(0, QBrush(Qt::red));

        QIcon icon;
        constructIconForObject(icon, drObj);
        item->setIcon(0, icon);

        addTopLevelItem(item);
    }
}

#include "abasketmanager.h"
void ADrawExplorerWidget::updateGui_multidrawMode()
{
    const AMultidrawRecord & rec = DrawObjects.front().MultidrawSettings;

    int ix = 0;
    int iy = 0;
    for (size_t i = 0; i < rec.BasketItems.size(); i++)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString name = GraphWindow.Basket->getName( rec.BasketItems[i] );

        if (name.size() > 15)
        {
            name.truncate(15);
            name += "..";
        }
        name = QString("%0-%1 %2").arg(ix).arg(iy).arg(name);

        if (rec.XY)
        {
            ix++;
            if (ix == rec.NumX)
            {
                ix = 0;
                iy++;
            }
        }
        else
        {
            iy++;
            if (iy == rec.NumY)
            {
                iy = 0;
                ix++;
            }
        }

        item->setText(0, name);
        item->setText(1, QString::number(i));

        //QIcon icon;
        //constructIconForObject(icon, drObj);
        //item->setIcon(0, icon);

        addTopLevelItem(item);
    }
}

void ADrawExplorerWidget::onContextMenuRequested(const QPoint &pos)
{
    if (DrawObjects.empty()) return;

    QTreeWidgetItem * item = currentItem();
    if (!item) return;

    int index = item->text(1).toInt();

    if (DrawObjects.front().Multidraw)
    {
        if (index < 0 || index >= DrawObjects.front().MultidrawSettings.BasketItems.size())
        {
            qWarning() << "Corrupted model: invalid index extracted";
            return;
        }
    }
    else
    {
        if (index < 0 || index >= DrawObjects.size())
        {
            qWarning() << "Corrupted model: invalid index extracted";
            return;
        }
    }

    showObjectContextMenu(mapToGlobal(pos), index);
}

void ADrawExplorerWidget::showObjectContextMenu(const QPoint &pos, int index)
{
    if (DrawObjects.empty()) return;

    QMenu Menu;
    Menu.setToolTipsVisible(true);

    bool multidraw = DrawObjects.front().Multidraw;
    if (multidraw)
    {
        if (index < 0 || index >= DrawObjects.front().MultidrawSettings.BasketItems.size()) return;

        QAction * moveUpA   = Menu.addAction("Move up");
        QAction * moveDownA = Menu.addAction("Move down");
        Menu.addSeparator();
        QAction * removeA = Menu.addAction("Remove");

        QAction * si = Menu.exec(pos);
        if (!si) return;

        if      (si == moveUpA)   onMoveUpAction(index);
        else if (si == moveDownA) onMoveDownAction(index);
        else if (si == removeA)
        {
            if (DrawObjects.front().MultidrawSettings.BasketItems.size() == 1)
            {
                guitools::message("Cannot remove the last item", this);
                return;
            }
            DrawObjects.front().MultidrawSettings.BasketItems.erase(DrawObjects.front().MultidrawSettings.BasketItems.begin() + index);
            GraphWindow.redrawAll();
            GraphWindow.highlightUpdateBasketButton(true);
        }
        return;
    }

    if (index < 0 || index >= DrawObjects.size()) return;

    ADrawObject & obj = DrawObjects[index];
    const QString Type = obj.Pointer->ClassName();


    QAction * editTextPave = ( Type == "TPaveText" ? Menu.addAction("Edit") : nullptr );
    if (editTextPave) Menu.addSeparator();

    QAction * renameA     = Menu.addAction("Rename");

    Menu.addSeparator();
    QAction * enableA     = Menu.addAction("Toggle enabled/disabled"); enableA->setChecked(obj.bEnabled); enableA->setEnabled(index != 0);

    Menu.addSeparator();
    QAction * delA        = Menu.addAction("Remove"); delA->setEnabled(index != 0);

    Menu.addSeparator();
    QAction * setAttrib   = Menu.addAction("Draw attributes");

    Menu.addSeparator();
    QAction* axesX        = Menu.addAction("X axis");
    QAction* axesY        = Menu.addAction("Y axis");
    QAction* axesZ        = Menu.addAction("Z axis");

    Menu.addSeparator();
    QMenu * moveMenu     = Menu.addMenu("Move");
        QAction * moveUpA   = moveMenu->addAction("Move up");   //moveUpA->setEnabled(index > 1);
        QAction * moveDownA = moveMenu->addAction("Move down"); //moveDownA->setEnabled(index != 0 && index != DrawObjects.size()-1);

    Menu.addSeparator();
    QAction* marginsA     = Menu.addAction("Change margins");

    Menu.addSeparator();
    QAction* addAxisRight = Menu.addAction("Add axis on RHS");
    QAction* addAxisTop   = Menu.addAction("Add axis on Top");

    Menu.addSeparator();
    QMenu * scaleMenu     = Menu.addMenu("Scale"); scaleMenu->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));
        QAction * scaleA      = scaleMenu->addAction("Scale");
        QAction * scaleTo1    = scaleMenu->addAction("Scale max to unity");
        QAction * scaleIntTo1 = scaleMenu->addAction("Scale integral to unity");
        QAction * scaleCDRA   = scaleMenu->addAction("Scale: click-drag-release");
    QAction * shiftA      = Menu.addAction("Shift X scale");  shiftA->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));

    Menu.addSeparator();
    QMenu * manipMenu     = Menu.addMenu("Manipulate histogram"); manipMenu->setEnabled(Type.startsWith("TH1"));
        QAction * interpolateA = manipMenu->addAction("Interpolate");
        manipMenu->addSeparator();
        QAction * medianA      = manipMenu->addAction("Apply median filter");

    Menu.addSeparator();
    QMenu * projectMenu   = Menu.addMenu("Projection"); projectMenu->setEnabled(Type.startsWith("TH2") || Type.startsWith("TH3"));
        QAction * projX      = projectMenu->addAction("X projection");
        QAction * projY      = projectMenu->addAction("Y projection");
        QAction * projZ      = projectMenu->addAction("Z projection"); projZ->setVisible(Type.startsWith("TH3"));
        projectMenu->addSeparator();
        QAction * projCustom = projectMenu->addAction("Show projection tool");

    Menu.addSeparator();
    //QMenu * calcMenu      = Menu.addMenu("Calculate");
    //    QAction * integralA = calcMenu->addAction("Draw integral");                   integralA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type == "TGraph" || Type == "TGraphError");
    //    QAction * fractionA = calcMenu->addAction("Calculate fraction before/after"); fractionA->setEnabled(Type.startsWith("TH1") || Type == "TProfile");
    QAction * integralA = Menu.addAction("Draw integral");                  integralA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type == "TGraph" || Type == "TGraphError");
    QAction * fractionA = Menu.addAction("Compute fractions before/after"); fractionA->setEnabled(Type.startsWith("TH1") || Type == "TProfile");

    Menu.addSeparator();
    QMenu * fitMenu       = Menu.addMenu("Fit");
        fitMenu->setToolTipsVisible(true);
        QAction * linFitA    = fitMenu->addAction("Linear (use click-drag along the line to define range)");
            linFitA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
        QAction * fwhmA      = fitMenu->addAction("Gauss with base line (use click-drag to define baseline and range)");
            fwhmA->  setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
        QAction * expA       = fitMenu->addAction("Exp. decay (use click-drag along the line to define range)");
            expA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
        //QAction * splineFitA = fitMenu->addAction("B-spline"); splineFitA->setEnabled(Type == "TGraph" || Type == "TGraphErrors");   //*** implement for TH1 too!
        fitMenu->addSeparator();
        QAction * gauss2FitA = fitMenu->addAction("Symmetric Gauss (use click-drag)"); gauss2FitA->setEnabled(Type.startsWith("TH2"));
                  gauss2FitA->setToolTip("Select a rectangular area indicating the fitting range.\nIf fit fails, try to center the area at the expected mean position");
        fitMenu->addSeparator();
        QAction * showFitPanel = fitMenu->addAction("Show fit panel");

    Menu.addSeparator();
    const QString options = obj.Options;
    bool flag3D = false;
    if (Type.startsWith("TH3") || Type.startsWith("TProfile2D") || Type.startsWith("TH2") || Type.startsWith("TF2") || Type.startsWith("TGraph2D"))
        flag3D = true;
    if ((Type.startsWith("TH2") || Type.startsWith("TProfile2D")) &&
        ( options.contains("col",Qt::CaseInsensitive) || options.contains("prof", Qt::CaseInsensitive) || (options.isEmpty())) )
        flag3D = false;
    //qDebug() << "flag 3D" << flag3D << Type;//options.contains("col",Qt::CaseInsensitive);
    QMenu * drawMenu = Menu.addMenu("Add to draw");        drawMenu->setEnabled(!flag3D);
        QAction* linDrawA     = drawMenu->addAction("Line (use click-drag)");    // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
        QAction* boxDrawA     = drawMenu->addAction("Box (use click-drag)");     // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
        QAction* ellipseDrawA = drawMenu->addAction("Elypse (use click-drag)");  // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));

    Menu.addSeparator();
    QMenu * saveMenu          = Menu.addMenu("Save");
        QAction * saveRootA = saveMenu->addAction("Save ROOT object");
        saveMenu->addSeparator();
        QAction * saveTxtA  = saveMenu->addAction("Save as text");
        QAction* saveEdgeA  = saveMenu->addAction("Save hist as text using bin edges");

    Menu.addSeparator();
    QAction * extractA        = Menu.addAction("Extract object"); extractA->setEnabled(DrawObjects.size() > 1);

    // ------ exec ------

    QAction * si = Menu.exec(pos);
    if (!si) return; //nothing was selected

    if      (si == renameA)      rename(obj);
    else if (si == enableA)      toggleEnable(obj);
    else if (si == delA)         remove(index);
    else if (si == setAttrib)    setAttributes(index);
    else if (si == scaleA)       scale(obj);
    else if (si == scaleTo1)     scaleToUnity(obj);
    else if (si == scaleIntTo1)  scaleIntegralToUnity(obj);
    else if (si == scaleCDRA)    scaleCDR(obj);
    else if (si == integralA)    drawIntegral(obj);
    else if (si == fractionA)    fraction(obj);
    else if (si == linFitA)      linFit(index);
    else if (si == fwhmA)        fwhm(index);
    else if (si == expA)         expFit(index);
    else if (si == gauss2FitA)   gauss2Fit(index);
    else if (si == interpolateA) interpolate(obj);
    else if (si == axesX)        editAxis(obj, 0);
    else if (si == axesY)        editAxis(obj, 1);
    else if (si == axesZ)        editAxis(obj, 2);
    else if (si == marginsA)     setCustomMargins(obj);
    else if (si == addAxisTop)   addAxis(0);
    else if (si == addAxisRight) addAxis(1);
    else if (si == shiftA)       shift(obj);
    else if (si == medianA)      median(obj);
    //else if (si == splineFitA)   splineFit(index);
    else if (si == projX)        projection(obj, 0);
    else if (si == projY)        projection(obj, 1);
    else if (si == projZ)        projection(obj, 2);
    else if (si == saveRootA)    saveRoot(obj);
    else if (si == saveTxtA)     saveAsTxt(obj, true);
    else if (si == saveEdgeA)    saveAsTxt(obj, false);
    else if (si == projCustom)   customProjection(obj);
    else if (si == showFitPanel) fitPanel(obj);
    else if (si == extractA)     extract(obj);
    else if (si == editTextPave) editPave(obj);
    else if (si == linDrawA)     linDraw(index);
    else if (si == boxDrawA)     boxDraw(index);
    else if (si == ellipseDrawA) ellipseDraw(index);
    else if (si == moveUpA)      onMoveUpAction(index);
    else if (si == moveDownA)    onMoveDownAction(index);
}

void ADrawExplorerWidget::manipulateTriggered()
{
    const int size = DrawObjects.size();
    const QPoint pos = mapToGlobal(QPoint(0, 0));

    if      (size == 0) {}
    else if (size == 1) showObjectContextMenu(pos, 0); //showing context menu of the first drawn object
    else
    {
        ADrawObject & obj = DrawObjects[0];
        const QString Type = obj.Pointer->ClassName();

        QMenu Menu;

        QAction * axesX        = Menu.addAction("X axis");
        QAction * axesY        = Menu.addAction("Y axis");
        QAction * axesZ        = Menu.addAction("Z axis");

        Menu.addSeparator();
        QAction * marginsA     = Menu.addAction("Set custom margins");

        Menu.addSeparator();
        QAction * addAxisRight = Menu.addAction("Add axis on RHS");
        QAction * addAxisTop   = Menu.addAction("Add axis on Top");

        Menu.addSeparator();
        QAction * scaleA       = Menu.addAction("Scale all graphs/histograms to max of unity");
            scaleA->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));
        QAction * scaleInte1   = Menu.addAction("Scale all graphs/histograms to integral of unity");
            scaleInte1->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));
        QAction * scaleDialog  = Menu.addAction("Scale all graphs/histograms (dialog)");
        scaleDialog->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));

        Menu.addSeparator();
        QAction * shiftA       = Menu.addAction("Shift X scale for all graphs/histograms");
        shiftA->setEnabled(Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"));

        Menu.addSeparator();
        const QString options = obj.Options;
        bool flag3D = false;
        if (Type.startsWith("TH3") || Type.startsWith("TProfile2D") || Type.startsWith("TH2") || Type.startsWith("TF2") || Type.startsWith("TGraph2D"))
            flag3D = true;
        if ((Type.startsWith("TH2") || Type.startsWith("TProfile2D")) && ( options.contains("col",Qt::CaseInsensitive) || options.contains("prof", Qt::CaseInsensitive) || (options.isEmpty())) )
            flag3D = false;
        //qDebug() << "flag 3D" << flag3D << Type;//options.contains("col",Qt::CaseInsensitive);

        QMenu * drawMenu = Menu.addMenu("Add to draw"); drawMenu->setEnabled(!flag3D);
            QAction * linDrawA     = drawMenu->addAction("Line (use click-drag)");    // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
            QAction * boxDrawA     = drawMenu->addAction("Box (use click-drag)");    // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));
            QAction * ellipseDrawA = drawMenu->addAction("Elypse (use click-drag)");    // linDrawA->setEnabled(Type.startsWith("TH1") || Type == "TProfile" || Type.startsWith("TGraph"));

        // ------ exec ------

        QAction * si = Menu.exec(pos);
        if (!si) return;

        if      (si == axesX)        editAxis(obj, 0);
        else if (si == axesY)        editAxis(obj, 1);
        else if (si == axesZ)        editAxis(obj, 2);
        else if (si == marginsA)     setCustomMargins(obj);
        else if (si == addAxisTop)   addAxis(0);
        else if (si == addAxisRight) addAxis(1);
        else if (si == scaleA)       scaleAllSameMax();
        else if (si == scaleInte1)   scaleAllIntegralsToUnity();
        else if (si == scaleDialog)  scale(DrawObjects);
        else if (si == shiftA)       shift(DrawObjects);
        else if (si == linDrawA)     linDraw(size-1);
        else if (si == boxDrawA)     boxDraw(size-1);
        else if (si == ellipseDrawA) ellipseDraw(size-1);
    }
}

void ADrawExplorerWidget::onItemDoubleClicked(QTreeWidgetItem * item, int)
{
    if (!item) return;
    if (DrawObjects.empty()) return;

    if (DrawObjects.front().Multidraw) return;

    activateCustomGuiForItem(item->text(1).toInt());
}

#include <QDropEvent>
#include <QListWidgetItem>
void ADrawExplorerWidget::dropEvent(QDropEvent * event)
{
    const QList<QTreeWidgetItem*> selected = selectedItems();
    int iMoved = -1;
    if (!selected.empty()) iMoved = selected.front()->text(1).toInt();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTreeWidgetItem * itemTo = this->itemAt(event->pos());
#else
    QTreeWidgetItem * itemTo = this->itemAt(event->position().toPoint());
#endif
    int iTo = -1;
    if (itemTo)
    {
        iTo = itemTo->text(1).toInt();
        if (dropIndicatorPosition() == QAbstractItemView::BelowItem) iTo++;
    }

    event->ignore();
    updateGui();

    qDebug() << "   Indexes:" << iMoved << iTo;
    if (iMoved == iTo) return;
    if (DrawObjects.empty()) return;
    if (DrawObjects.front().Multidraw)
    {
        std::vector<int> & items = DrawObjects.front().MultidrawSettings.BasketItems;
        if (iTo == items.size()) iTo--;
        if (iMoved < 0 || iMoved >= items.size()) return;
        if (iTo    < 0 || iTo    >= items.size()) return;
        std::swap(items[iMoved], items[iTo]);
    }
    else
    {
        std::vector<ADrawObject> & items = DrawObjects;
        if (iTo == items.size()) iTo--;
        if (iMoved == 0 || iTo == 0)
        {
            guitools::message("Cannot change position of the first item", this);
            return;
        }
        if (iMoved < 0 || iMoved >= items.size()) return;
        if (iTo    < 0 || iTo    >= items.size()) return;
        std::swap(items[iMoved], items[iTo]);
    }

    GraphWindow.redrawAll();
}

void ADrawExplorerWidget::activateCustomGuiForItem(int index)
{
    if (index < 0 || index >= DrawObjects.size())
    {
        qWarning() << "Invalid model index!";
        return;
    }

    ADrawObject & obj = DrawObjects[index];
    const QString Type = obj.Pointer->ClassName();

    if      (Type == "TLegend")   emit requestShowLegendDialog();
    else if (Type == "TPaveText") editPave(obj);
    else if (Type == "TGaxis")    editTGaxis(obj);
    else                          setAttributes(index);
}

void ADrawExplorerWidget::addToDrawObjectsAndRegister(TObject * pointer, const QString & options)
{
    DrawObjects.push_back( ADrawObject(pointer, options) );
    GraphWindow.registerTObject(pointer);
}

void ADrawExplorerWidget::rename(ADrawObject & obj)
{
    bool ok;
    QString text = QInputDialog::getText(&GraphWindow, "Rename item", "Enter new name:", QLineEdit::Normal, obj.Name, &ok);
    if (!ok || text.isEmpty()) return;

    obj.Name = text;
    TNamed * tobj = dynamic_cast<TNamed*>(obj.Pointer);
    if (tobj) tobj->SetTitle(text.toLatin1().data());

    GraphWindow.clearCopyOfDrawObjects();
    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
    updateGui();
}

void ADrawExplorerWidget::toggleEnable(ADrawObject & obj)
{
    obj.bEnabled = !obj.bEnabled;

    GraphWindow.clearCopyOfDrawObjects();
    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::remove(int index)
{
    GraphWindow.makeCopyOfDrawObjects();

    DrawObjects.erase(DrawObjects.begin() + index); // do not delete - GraphWindow handles garbage collection!

    if (index == 0 && !DrawObjects.empty())
    {
        //remove "same" from the options line for the new leading object
        DrawObjects.front().Options.remove("same", Qt::CaseInsensitive);
    }

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::setAttributes(int index)
{
    if (index < 0 || index >= DrawObjects.size()) return;

    ADrawObject & obj = DrawObjects[index];
    QString Type = obj.Pointer->ClassName();
    if (Type == "TLegend") return;

    ALineMarkerFillDialog D(obj, (index == 0), this);
    connect(&D, &ALineMarkerFillDialog::requestRedraw, &GraphWindow, &AGraphWindow::redrawAll);
    D.exec();

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::showPanel(ADrawObject &obj)
{
    TH1 * h = dynamic_cast<TH1*>(obj.Pointer);
    if (h)
    {
        h->DrawPanel();
        return;
    }
    TGraph * g = dynamic_cast<TGraph*>(obj.Pointer);
    if (g)
    {
        g->DrawPanel();
        return;
    }
}

void ADrawExplorerWidget::fitPanel(ADrawObject &obj)
{
    TH1 * h = dynamic_cast<TH1*>(obj.Pointer);
    if (h) h->FitPanel();
    else
    {
        TGraph* g = dynamic_cast<TGraph*>(obj.Pointer);
        if (g) g->FitPanel();
    }
}

double runScaleDialog(QWidget * parent)
{
    QDialog * D = new QDialog(parent);
    D->setMinimumWidth(500);

    QDoubleValidator * vali = new QDoubleValidator(D);
    QVBoxLayout * l = new QVBoxLayout(D);
        QHBoxLayout * l1 = new QHBoxLayout();
            QLabel * lab1 = new QLabel("Multiply by ");
        l1->addWidget(lab1);
            QLineEdit * leM = new QLineEdit("1.0");
            leM->setValidator(vali);
        l1->addWidget(leM);
            QLabel * lab2 = new QLabel(" and divide by ");
        l1->addWidget(lab2);
            QLineEdit * leD = new QLineEdit("1.0");
            leD->setValidator(vali);
        l1->addWidget(leD);
    l->addLayout(l1);
        QPushButton* pb = new QPushButton("Scale");
        QObject::connect(pb, &QPushButton::clicked, D, &QDialog::accept);
    l->addWidget(pb);

    int ret = D->exec();
    double res = 1.0;
    if (ret == QDialog::Accepted)
    {
        double Mult = leM->text().toDouble();
        double Div = leD->text().toDouble();
        if (Div == 0) guitools::message("Cannot divide by 0!", D);
        else          res = Mult / Div;
    }

    delete D;
    return res;
}

bool ADrawExplorerWidget::canScale(ADrawObject & obj)
{
    const QString name = obj.Pointer->ClassName();
    QStringList impl;
    impl << "TGraph" << "TGraphErrors" << "TH1I" << "TH1D" << "TH1F" << "TH2I"<< "TH2D"<< "TH2D";
    if (!impl.contains(name))
    {
        guitools::message("Not implemented for this object", &GraphWindow);
        return false;
    }
    else return true;
}

void ADrawExplorerWidget::doScale(ADrawObject &obj, double sf)
{
    GraphWindow.makeCopyOfDrawObjects();
    TObject * tobj = obj.Pointer->Clone();
    GraphWindow.registerTObject(tobj);

    const QString name = obj.Pointer->ClassName();
    if (name == "TGraph")
    {
        TGraph* gr = static_cast<TGraph*>(tobj);
        TF2 f("aaa", "[0]*y",0,1);
        f.SetParameter(0, sf);
        gr->Apply(&f);
    }
    else if (name == "TGraphErrors")
    {
        TGraphErrors* gr = static_cast<TGraphErrors*>(tobj);
        TF2 f("aaa", "[0]*y",0,1);
        f.SetParameter(0, sf);
        gr->Apply(&f);
    }
    else if (name.startsWith("TH1"))
    {
        TH1* h = static_cast<TH1*>(tobj);
        //h->Sumw2();
        h->Scale(sf);
        if (obj.Options.isEmpty()) obj.Options = "hist";
    }
    else if (name.startsWith("TH2"))
    {
        TH2* h = static_cast<TH2*>(tobj);
        //h->Sumw2();
        h->Scale(sf);
    }
    obj.Pointer = tobj;

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::scale(ADrawObject &obj)
{
    if (!canScale(obj)) return;

    double sf = runScaleDialog(&GraphWindow);
    if (sf == 1.0) return;

    doScale(obj, sf);
}

void ADrawExplorerWidget::scale(std::vector<ADrawObject> & drawObjects)
{
    for (ADrawObject & obj : drawObjects)
        if (!canScale(obj)) return;

    double sf = runScaleDialog(&GraphWindow);
    if (sf == 1.0) return;

    for (ADrawObject & obj : drawObjects)
        doScale(obj, sf);
}

void ADrawExplorerWidget::scaleIntegralToUnity(ADrawObject & obj)
{
    if (!canScale(obj)) return;
    double factor = 0;

    const QString name = obj.Pointer->ClassName();
    if (name.startsWith("TGraph"))
    {
        TGraph * gr = static_cast<TGraph*>(obj.Pointer);
        //factor = gr->Integral(); // cannot use: calculates the polygon area!
        const int size = gr->GetN();
        double * ar = gr->GetY();
        for (int i = 0; i < size; i++)
            factor += ar[i];
    }
    else if (name.startsWith("TH1"))
    {
        TH1* h = static_cast<TH1*>(obj.Pointer);
        factor = h->Integral();
    }
    else if (name.startsWith("TH2"))
    {
        TH2* h = static_cast<TH2*>(obj.Pointer);
        factor = h->Integral();
    }

    if (factor == 0 || factor == 1.0) return;
    factor = 1.0 / factor;
    doScale(obj, factor);
}

void ADrawExplorerWidget::scaleToUnity(ADrawObject & obj)
{
    if (!canScale(obj)) return;

    double thisMax = 0;
    getDrawMax(obj, thisMax);
    if (thisMax != 0) doScale(obj, 1.0/thisMax);
}

void ADrawExplorerWidget::scaleCDR(ADrawObject &obj)
{
    if (!canScale(obj)) return;

    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DLine();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double startY = Raster.Line2DstartY; // extracted2DLineYstart;
    double stopY  = Raster.Line2DstopY;  // extracted2DLineYstop;

    if (startY == 0)  // can be very small on log canvas
    {
        guitools::message("Cannot scale: division by zero", this);
        return;
    }

    double sf = stopY / startY;
    if (sf == 1.0) return;

    doScale(obj, sf);
}

bool ADrawExplorerWidget::getDrawMax(ADrawObject & obj, double & max)
{
    max = 0;

    TGraph * g = dynamic_cast<TGraph*>(obj.Pointer);
    if (g)
    {
        max = TMath::MaxElement(g->GetN(), g->GetY());
        return true;
    }

    TH1 * h = dynamic_cast<TH1*>(obj.Pointer);
    if (!h) return false;

    int numBins = h->GetNbinsX();
    if (numBins == 0) return false;
    max = h->GetBinContent(1); //underflow/overflow bins are #0 and #(numBins+1)
    for (int i = 2; i < numBins+1; i++)
    {
        double val = h->GetBinContent(i);
        if (val > max) max = val;
    }
    return true;
}

void ADrawExplorerWidget::scaleAllSameMax()
{
    const int size = DrawObjects.size();
    if (size == 0) return;

    ADrawObject & mainObj = DrawObjects[0];
    if (!canScale(mainObj)) return;

    for (ADrawObject & obj : DrawObjects)
    {
        double thisMax = 0;
        getDrawMax(obj, thisMax);
        if (thisMax == 0) continue;
        doScale(obj, 1.0/thisMax);
    }
}

void ADrawExplorerWidget::scaleAllIntegralsToUnity()
{
    const int size = DrawObjects.size();
    if (size == 0) return;

    ADrawObject & mainObj = DrawObjects[0];
    if (!canScale(mainObj)) return;

    for (ADrawObject & obj : DrawObjects)
    {
        double factor = 0;

        const QString name = obj.Pointer->ClassName();
        if (name.startsWith("TGraph"))
        {
            TGraph * gr = static_cast<TGraph*>(obj.Pointer);
            //factor = gr->Integral(); // cannot use: calculates the polygon area!
            const int size = gr->GetN();
            double * ar = gr->GetY();
            for (int i = 0; i < size; i++)
                factor += ar[i];
        }
        else if (name.startsWith("TH1"))
        {
            TH1* h = static_cast<TH1*>(obj.Pointer);
            factor = h->Integral();
        }
        else if (name.startsWith("TH2"))
        {
            TH2* h = static_cast<TH2*>(obj.Pointer);
            factor = h->Integral();
        }

        if (factor == 0 || factor == 1.0) return;
        factor = 1.0 / factor;
        doScale(obj, factor);
    }
}

const std::pair<double, double> runShiftDialog(QWidget * parent)
{
    QDialog * D = new QDialog(parent);

    QDoubleValidator * vali = new QDoubleValidator(D);
    QVBoxLayout * l = new QVBoxLayout(D);
        QHBoxLayout * l1 = new QHBoxLayout();
            QLabel * lab1 = new QLabel("Multiply by: ");
        l1->addWidget(lab1);
            QLineEdit * leM = new QLineEdit("1.0");
            leM->setValidator(vali);
        l1->addWidget(leM);
            QLabel* lab2 = new QLabel(" Add: ");
        l1->addWidget(lab2);
            QLineEdit* leA = new QLineEdit("0");
            leA->setValidator(vali);
        l1->addWidget(leA);
    l->addLayout(l1);
        QPushButton* pb = new QPushButton("Shift");
        QObject::connect(pb, &QPushButton::clicked, D, &QDialog::accept);
    l->addWidget(pb);

    int ret = D->exec();
    std::pair<double, double> res(1.0, 0);
    if (ret == QDialog::Accepted)
    {
        res.first =  leM->text().toDouble();
        res.second = leA->text().toDouble();
    }

    delete D;
    return res;
}

void ADrawExplorerWidget::shift(ADrawObject &obj)
{
    QString name = obj.Pointer->ClassName();
    QStringList impl;
    impl << "TGraph" << "TGraphErrors"  << "TH1I" << "TH1D" << "TH1F" << "TProfile";
    if (!impl.contains(name))
    {
        guitools::message("Not implemented for this object type", &GraphWindow);
        return;
    }

    const std::pair<double, double> val = runShiftDialog(&GraphWindow);
    if (val.first == 1.0 && val.second == 0) return;

    GraphWindow.makeCopyOfDrawObjects();
    TObject * tobj = obj.Pointer->Clone();
    GraphWindow.registerTObject(tobj);

    if (name.startsWith("TGraph"))
    {
        TGraph* g = dynamic_cast<TGraph*>(tobj);
        if (g)
        {
            const int num = g->GetN();
            for (int i=0; i<num; i++)
            {
                double x, y;
                g->GetPoint(i, x, y);
                x = x * val.first + val.second;
                g->SetPoint(i, x, y);
            }
        }
    }
    else
    {
        TH1* h = dynamic_cast<TH1*>(tobj);
        if (h)
        {
            const int nbins = h->GetXaxis()->GetNbins();
            double* new_bins = new double[nbins+1];
            for (int i=0; i <= nbins; i++)
                new_bins[i] = h-> GetBinLowEdge(i+1) * val.first + val.second;
            h->SetBins(nbins, new_bins);
            delete [] new_bins;
        }
    }
    obj.Pointer = tobj;

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::shift(std::vector<ADrawObject> & drawObjects)
{
    if (drawObjects.empty()) return;
    QString name = drawObjects.front().Pointer->ClassName();
    QStringList impl;
    impl << "TGraph" << "TGraphErrors"  << "TH1I" << "TH1D" << "TH1F" << "TProfile";
    if (!impl.contains(name))
    {
        guitools::message("Not implemented for this object type", &GraphWindow);
        return;
    }

    const std::pair<double, double> val = runShiftDialog(&GraphWindow);
    if (val.first == 1.0 && val.second == 0) return;

    GraphWindow.makeCopyOfDrawObjects();

    for (ADrawObject & obj : drawObjects)
    {
        TObject * tobj = obj.Pointer->Clone();
        GraphWindow.registerTObject(tobj);

        name = obj.Pointer->ClassName();
        if (name.startsWith("TGraph"))
        {
            TGraph * g = dynamic_cast<TGraph*>(tobj);
            if (g)
            {
                const int num = g->GetN();
                for (int i=0; i<num; i++)
                {
                    double x, y;
                    g->GetPoint(i, x, y);
                    x = x * val.first + val.second;
                    g->SetPoint(i, x, y);
                }
            }
        }
        else
        {
            TH1 * h = dynamic_cast<TH1*>(tobj);
            if (h)
            {
                const int nbins = h->GetXaxis()->GetNbins();
                double * new_bins = new double[nbins+1];
                for (int i=0; i <= nbins; i++)
                    new_bins[i] = h->GetBinLowEdge(i+1) * val.first + val.second;
                h->SetBins(nbins, new_bins);
                delete [] new_bins;
            }
        }
        obj.Pointer = tobj;
    }

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::drawIntegral(ADrawObject &obj)
{
    TH1D   * hi = nullptr;
    TGraph * gi = nullptr;

    TH1    * h = dynamic_cast<TH1*>(obj.Pointer);
    TGraph * g = dynamic_cast<TGraph*>(obj.Pointer);
    if (h)
    {
        int bins = h->GetNbinsX();

        double* edges = new double[bins+1];
        for (int i=0; i<bins+1; i++)
            edges[i] = h->GetBinLowEdge(i+1);

        QString title = "Integral of " + obj.Name;
        hi = new TH1D("integral", title.toLocal8Bit().data(), bins, edges);
        delete [] edges;

        QString Xtitle = h->GetXaxis()->GetTitle();
        if (!Xtitle.isEmpty()) hi->GetXaxis()->SetTitle(Xtitle.toLocal8Bit().data());

        double prev = 0;
        for (int i=1; i<bins+1; i++)
        {
            prev += h->GetBinContent(i);
            hi->SetBinContent(i, prev);
        }
    }
    else if (g)
    {
        const int points = g->GetN();
        double sum = 0;
        double x, y;
        std::vector<double> X; X.reserve(points);
        std::vector<double> Y; Y.reserve(points);
        for (int i=0; i<points; i++)
        {
            g->GetPoint(i, x, y);
            X.push_back(x);
            sum += y;
            Y.push_back(sum);
        }

        gi = new TGraph(points, X.data(), Y.data());

        TString XTitle = g->GetXaxis()->GetTitle();
        gi->GetXaxis()->SetTitle(XTitle);

        TString YTitle = g->GetYaxis()->GetTitle();
        gi->GetYaxis()->SetTitle("Integral of " + YTitle);
    }
    else
    {
        guitools::message("Not implemented for this object type", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();
    GraphWindow.clearBasketActiveId();

    DrawObjects.clear();
    if (hi) addToDrawObjectsAndRegister(hi, "hist");
    else    addToDrawObjectsAndRegister(gi, "APL");

    GraphWindow.redrawAll();
}

void ADrawExplorerWidget::fraction(ADrawObject &obj)
{
    TH1* h = dynamic_cast<TH1*>(obj.Pointer);
    if (!h)
    {
        guitools::message("This operation requires TH1 ROOT object", &GraphWindow);
        return;
    }
    TH1* cum = h->GetCumulative(true);

    double integral = h->Integral();

    QDialog D(&GraphWindow);
    D.setWindowTitle("Integral / fraction calculator");

    QVBoxLayout *l = new QVBoxLayout();
        QPushButton * pbD = new QPushButton("Dummy");
    l->addWidget(pbD);
        QHBoxLayout * hh = new QHBoxLayout();
            QLabel * lab = new QLabel("Enter threshold:");
        hh->addWidget(lab);
            QLineEdit* tT = new QLineEdit();
        hh->addWidget(tT);
    l->addLayout(hh);
        QFrame * f = new QFrame();
            f->setFrameShape(QFrame::HLine);
    l->addWidget(f);
        hh = new QHBoxLayout();
            lab = new QLabel("Interpolated sum before:");
        hh->addWidget(lab);
            QLineEdit* tIb = new QLineEdit();
        hh->addWidget(tIb);
            lab = new QLabel("Fraction:");
        hh->addWidget(lab);
            QLineEdit* tIbf = new QLineEdit();
        hh->addWidget(tIbf);
    l->addLayout(hh);
        hh = new QHBoxLayout();
            lab = new QLabel("Interpolated sum after:");
        hh->addWidget(lab);
            QLineEdit* tIa = new QLineEdit();
        hh->addWidget(tIa);
            lab = new QLabel("Fraction:");
        hh->addWidget(lab);
            QLineEdit* tIaf = new QLineEdit();
        hh->addWidget(tIaf);
    l->addLayout(hh);
        hh = new QHBoxLayout();
            lab = new QLabel("Total sum of bins:");
        hh->addWidget(lab);
            QLineEdit* tI = new QLineEdit(QString::number(integral));
            tI->setReadOnly(true);
        hh->addWidget(tI);
    l->addLayout(hh);
        QPushButton * pb = new QPushButton("Close");
    l->addWidget(pb);
    D.setLayout(l);

    pbD->setVisible(false);

    QObject::connect(pb, &QPushButton::clicked, &D, &QDialog::reject);
    QObject::connect(tT, &QLineEdit::editingFinished, [h, cum, integral, tT, tIb, tIbf, tIa, tIaf]()
    {
        bool bOK;
        double val = tT->text().toDouble(&bOK);
        if (bOK)
        {
            int bins = cum->GetNbinsX();
            TAxis * ax = cum->GetXaxis();
            int bin = ax->FindBin(val);

            double result = 0;
            if (bin > bins)
                result = cum->GetBinContent(bins);
            else
            {
                double width = h->GetBinWidth(bin);
                double delta = val - h->GetBinLowEdge(bin);

                double thisBinAdds = h->GetBinContent(bin) * delta / width;

                double prev = (bin == 1 ? 0 : cum->GetBinContent(bin-1));
                result = prev + thisBinAdds;
            }
            tIb->setText(QString::number(result));
            tIbf->setText(QString::number(result/integral));
            if (integral != 0) tIa->setText(QString::number(integral - result));
            else tIa->setText("");
            if (integral != 0) tIaf->setText(QString::number( (integral - result)/integral ));
            else tIa->setText("");
        }
        else
        {
            tIa->clear();
            tIb->clear();
        }
    });

    D.exec();

    delete cum;
}

double GauseWithBase(double * x, double * par)
{
   return par[0] * exp( par[1] * (x[0]+par[2])*(x[0]+par[2]) ) + par[3]*x[0] + par[4];   // [0]*exp([1]*(x+[2])^2) + [3]*x + [4]
}

void ADrawExplorerWidget::fwhm(int index)
{
    ADrawObject & obj = DrawObjects[index];

    TGraph * g = nullptr;
    TH1    * h = nullptr;

    const QString cn = obj.Pointer->ClassName();
    if (cn.startsWith("TGraph"))
    {
        g = dynamic_cast<TGraph*>(obj.Pointer);
        if (!g) return;
    }
    else if (cn.startsWith("TH1") || cn == "TProfile")
    {
        h = dynamic_cast<TH1*>(obj.Pointer);
        if (!h) return;
    }
    else
    {
        guitools::message("Can be used only with TGraph and 1D histogram!", &GraphWindow);
        return;
    }

    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DLine();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double startX = Raster.Line2DstartX; // extracted2DLineXstart();
    double stopX  = Raster.Line2DstopX;  // extracted2DLineXstop();
    if (startX > stopX) std::swap(startX, stopX);
    //qDebug() << startX << stopX;

    double a = Raster.extracted2DLineA;
    double b = Raster.extracted2DLineB;
    double c = Raster.extracted2DLineC;
    if (fabs(b) < 1.0e-10)
    {
        guitools::message("Bad base line, cannot fit", &GraphWindow);
        return;
    }
                                                                   //  S  * exp( -0.5/s2 * (x   -m )^2) +  A *x +  B
    TF1 * f = new TF1("myfunc", GauseWithBase, startX, stopX, 5);  //  [0] * exp(    [1]  * (x + [2])^2) + [3]*x + [4]
    f->SetTitle("Gauss fit");
    GraphWindow.registerTObject(f);

    double initMid = startX + 0.5*(stopX - startX);
    //qDebug() << "Initial mid:"<<initMid;
    double initSigma = (stopX - startX)/2.3548; //sigma
    double startPar1 = -0.5 / (initSigma * initSigma );
    //qDebug() << "Initial par1"<<startPar1;
    double midBinNum, valOnMid;
    if (h)
    {
        midBinNum = h->GetXaxis()->FindBin(initMid);
        valOnMid = h->GetBinContent(midBinNum);
    }
    else
    {
        double x = startX;
        double y = 0;
        for (int i=0; i<g->GetN(); i++)
        {
            g->GetPoint(i, x, y);
            if (x >= initMid) break;
        }
        valOnMid = y;
    }
    double baseAtMid = (c - a * initMid) / b;
    double gaussAtMid = valOnMid - baseAtMid;
    //qDebug() << "bin, valMid, baseMid, gaussmid"<<midBinNum<< valOnMid << baseAtMid << gaussAtMid;

    f->SetParameter(0, gaussAtMid);
    f->SetParameter(1, startPar1);
    f->SetParameter(2, -initMid);
    f->FixParameter(3, -a/b);  // fixed!
    f->FixParameter(4, c/b);   // fixed!

    int status = (h ? h->Fit(f, "R0") : g->Fit(f, "R0"));
    if (status != 0)
    {
        guitools::message("Fit failed!", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    double mid = -f->GetParameter(2);
    double sigma = TMath::Sqrt(-0.5/f->GetParameter(1));
    //qDebug() << "sigma:"<<sigma;
    double FWHM = sigma * 2.0*TMath::Sqrt(2.0*TMath::Log(2.0));
    double rel = FWHM/mid;

    //draw fit line
    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(f, "same"));
    //draw base line
    TF1 *fl = new TF1("line", "pol1", startX, stopX);
    fl->SetTitle("Baseline");
    GraphWindow.registerTObject(fl);
    fl->SetLineStyle(2);
    fl->SetParameters(c/b, -a/b);
    DrawObjects.insert(DrawObjects.begin()+index+2, ADrawObject(fl, "same"));

    //box with results
    TPaveText * la = new TPaveText(0.15, 0.75, 0.5, 0.85, "NDC");
    la->SetFillColor(0);
    la->SetBorderSize(1);
    la->SetLineColor(1);
    la->SetTextAlign( (0 + 1) * 10 + 2);
    //QString text = QString("FWHM = %1\nmean = %2\nfwhm/mean = %3").arg(FWHM).arg(mid).arg(rel);
    QString text = QString("Mean: %0  Sigma: %1\nfwhm: %2  fwhm/mean: %3").arg(mid).arg(sigma).arg(FWHM).arg(rel);
    const QStringList sl = text.split("\n");
    for (const QString & s : sl) la->AddText(s.toLatin1());
    GraphWindow.registerTObject(la);
    DrawObjects.insert(DrawObjects.begin()+index+3, ADrawObject(la, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::linFit(int index)
{
    ADrawObject & obj = DrawObjects[index];

    TGraph * g = nullptr;
    TH1    * h = nullptr;

    const QString cn = obj.Pointer->ClassName();
    if (cn.startsWith("TGraph"))
    {
        g = dynamic_cast<TGraph*>(obj.Pointer);
        if (!g) return;
    }
    else if (cn.startsWith("TH1") || cn == "TProfile")
    {
        h = dynamic_cast<TH1*>(obj.Pointer);
        if (!h) return;
    }
    else
    {
        guitools::message("Can be used only with TGraphs and 1D histogram!", &GraphWindow);
        return;
    }

    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DLine();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double startX = Raster.Line2DstartX; // extracted2DLineXstart();
    double stopX  = Raster.Line2DstopX;  // extracted2DLineXstop();
    if (startX > stopX) std::swap(startX, stopX);

    double a = Raster.extracted2DLineA;
    double b = Raster.extracted2DLineB;
    double c = Raster.extracted2DLineC;
    if (fabs(b) < 1.0e-10)
    {
        guitools::message("Bad line, cannot fit", &GraphWindow);
        return;
    }

    TF1 * f = new TF1("line", "pol1", startX, stopX);
    f->SetTitle("Linear fit");
    GraphWindow.registerTObject(f);

    f->SetParameter(0, c/b);
    f->SetParameter(1, -a/b);

    int status = (h ? h->Fit(f, "R0") : g->Fit(f, "R0"));
    if (status != 0)
    {
        guitools::message("Fit failed!", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    double B = f->GetParameter(0);
    double A = f->GetParameter(1);

    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(f, "same"));

    QString text = QString("y = Ax + B\nA = %1, B = %2\nx range: %3 -> %4").arg(A).arg(B).arg(startX).arg(stopX);
    if (A != 0) text += QString("\ny=0 at %1").arg(-B/A);
    TPaveText* la = new TPaveText(0.15, 0.75, 0.5, 0.85, "NDC");
    la->SetFillColor(0);
    la->SetBorderSize(1);
    la->SetLineColor(1);
    la->SetTextAlign( (0 + 1) * 10 + 2);
    QStringList sl = text.split("\n");
    for (QString s : sl) la->AddText(s.toLatin1());
    GraphWindow.registerTObject(la);
    DrawObjects.insert(DrawObjects.begin()+index+2, ADrawObject(la, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::expFit(int index)
{
    ADrawObject & obj = DrawObjects[index];

    TGraph * g = nullptr;
    TH1    * h = nullptr;

    const QString cn = obj.Pointer->ClassName();
    if (cn.startsWith("TGraph"))
    {
        g = dynamic_cast<TGraph*>(obj.Pointer);
        if (!g) return;
    }
    else if (cn.startsWith("TH1") || cn == "TProfile")
    {
        h = dynamic_cast<TH1*>(obj.Pointer);
        if (!h) return;
    }
    else
    {
        guitools::message("Can be used only with TGraphs and 1D histogram!", &GraphWindow);
        return;
    }

    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DLine();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double startX = Raster.Line2DstartX; // extracted2DLineXstart();
    double startY = Raster.Line2DstartY; // extracted2DLineYstart();
    double stopX  = Raster.Line2DstopX;  // extracted2DLineXstop();
    double stopY  = Raster.Line2DstopY;  // extracted2DLineYstop();
    if (startX > stopX)
    {
        std::swap(startX, stopX);
        std::swap(startY, stopY);
    }

    TF1 * f = new TF1("expDecay", "[0]*exp(-(x-[2])/[1])", startX, stopX);
    f->SetTitle("Exp decay fit");
    GraphWindow.registerTObject(f);

    f->SetParameter(0, startY);
    f->SetParameter(1, stopX - startX);
    f->SetParameter(2, startX);
    f->SetParLimits(2, startX, startX);

    int status = (h ? h->Fit(f, "R0") : g->Fit(f, "R0"));
    if (status != 0)
    {
        guitools::message("Fit failed!", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    double A = f->GetParameter(0);
    double T = f->GetParameter(1);

    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(f, "same"));

    QString text = QString("y = A*exp{-(t-t0)/T)}\nT = %1, A = %2, t0 = %3\nx range: %3 -> %4").arg(T).arg(A).arg(startX).arg(stopX);
    TPaveText* la = new TPaveText(0.15, 0.75, 0.5, 0.85, "NDC");
    la->SetFillColor(0);
    la->SetBorderSize(1);
    la->SetLineColor(1);
    la->SetTextAlign( (0 + 1) * 10 + 2);
    QStringList sl = text.split("\n");
    for (QString s : sl) la->AddText(s.toLatin1());
    GraphWindow.registerTObject(la);
    DrawObjects.insert(DrawObjects.begin()+index+2, ADrawObject(la, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

double gauss2D(double * x, double * par)
{
   return par[0] * exp( -0.5*pow( (x[0]-par[1])/par[3], 2) -0.5*pow( (x[1]-par[2])/par[4], 2)  );
}

double gauss2Dsymmetric(double * x, double * par)
{
   return par[0] * exp( -0.5*pow( (x[0]-par[1])/par[3], 2) -0.5*pow( (x[1]-par[2])/par[3], 2)  );
}

void ADrawExplorerWidget::gauss2Fit(int index)
{
    ADrawObject & obj = DrawObjects[index];

    const QString cn = obj.Pointer->ClassName();
    if (!cn.startsWith("TH2"))
    {
        guitools::message("Can be used only with 2D histograms!", &GraphWindow);
        return;
    }

    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DBox();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double X1 = Raster.extractedX1;
    double X2 = Raster.extractedX2;
    if (X1 > X2) std::swap(X1, X2);
    double Y1 = Raster.extractedY1;
    double Y2 = Raster.extractedY2;
    if (Y1 > Y2) std::swap(Y1, Y2);

    TH2 * h = static_cast<TH2*>(obj.Pointer);
    //double xmin = h->GetXaxis()->GetXmin();
    //double xmax = h->GetXaxis()->GetXmax();
    //double ymin = h->GetYaxis()->GetXmin();
    //double ymax = h->GetYaxis()->GetXmax();
    double xmean = 0.5 * (X1 + X2);
    double ymean = 0.5 * (Y1 + Y2);
    double fwhmX = 0.25 * (X2 - X1);
    double fwhmY = 0.25 * (Y2 - Y1);
    double sig = 0.5 * (fwhmX + fwhmY) / 2.355;
    double A0 = h->Interpolate(xmean, ymean);

    //TF2 * f = new TF2("myfunc", gauss2Dsymmetric, xmin, xmax, ymin, ymax, 4);
    TF2 * f = new TF2("myfunc", gauss2Dsymmetric, X1, X2, Y1, Y2, 4);
    f->SetTitle("2D Gauss fit");
    GraphWindow.registerTObject(f);

    f->SetParameter(0, A0);
    f->SetParameter(1, xmean);
    f->SetParameter(2, ymean);
    f->SetParameter(3, sig);

    //qDebug() << A0 << xmean << ymean << sig;

    int status = h->Fit(f, "R");
    if (status != 0)
    {
        guitools::message("Fit failed!", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    double A     = f->GetParameter(0); double dA     = f->GetParError(0);
    double x0    = f->GetParameter(1); double dx0    = f->GetParError(1);
    double y0    = f->GetParameter(2); double dy0    = f->GetParError(2);
    double Sigma = f->GetParameter(3); double dSigma = f->GetParError(3);

    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(f, "same"));

    QString text = QString("MeanX: %0 #pm %1\nMeanY: %2 #pm %3\nSigma: %4 #pm %5\nScaling: %6 #pm %7")
                          .arg(x0).arg(dx0).arg(y0).arg(dy0).arg(Sigma).arg(dSigma).arg(A).arg(dA);
    TPaveText* la = new TPaveText(0.15, 0.75, 0.5, 0.85, "NDC");
    la->SetFillColor(0);
    la->SetBorderSize(1);
    la->SetLineColor(1);
    la->SetTextAlign( (0 + 1) * 10 + 2);
    QStringList sl = text.split("\n");
    for (const QString & s : sl) la->AddText(s.toLatin1());
    GraphWindow.registerTObject(la);
    DrawObjects.insert(DrawObjects.begin()+index+2, ADrawObject(la, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::interpolate(ADrawObject &obj)
{
    TH1* hist = dynamic_cast<TH1*>(obj.Pointer);
    if (!hist)
    {
        guitools::message("This operation requires TH1 ROOT object", &GraphWindow);
        return;
    }

    QDialog d;
    QVBoxLayout * lMain = new QVBoxLayout(&d);

    QHBoxLayout* l = new QHBoxLayout();
      QVBoxLayout * v = new QVBoxLayout();
          QLabel* lab = new QLabel("From:");
          v->addWidget(lab);
          lab = new QLabel("Step:");
          v->addWidget(lab);
          lab = new QLabel("Steps:");
          v->addWidget(lab);
     l->addLayout(v);
     v = new QVBoxLayout();
          QLineEdit * leFrom = new QLineEdit("0");
          v->addWidget(leFrom);
          QLineEdit * leStep = new QLineEdit("10");
          v->addWidget(leStep);
          QLineEdit* leSteps = new QLineEdit("100");
          v->addWidget(leSteps);
     l->addLayout(v);
    lMain->addLayout(l);

    QPushButton * pb = new QPushButton("Interpolate");
    lMain->addWidget(pb);

    QObject::connect(pb, &QPushButton::clicked,
                     [&d, hist, leFrom, leStep, leSteps, this]()
    {
        int steps = leSteps->text().toDouble();
        int step  = leStep->text().toDouble();
        int from  = leFrom->text().toDouble();

        TH1D* hi = new TH1D("", "", steps, from, from + step*steps);
        for (int i=0; i<steps; i++)
        {
            double x = from + step * i;
            double val = hist->Interpolate(x);
            hi->SetBinContent(i+1, val);
        }
        QString Xtitle = hist->GetXaxis()->GetTitle();
        if (!Xtitle.isEmpty()) hi->GetXaxis()->SetTitle(Xtitle.toLocal8Bit().data());

        GraphWindow.makeCopyOfDrawObjects();
        GraphWindow.makeCopyOfActiveBasketId();
        GraphWindow.clearBasketActiveId();

        DrawObjects.clear();
        addToDrawObjectsAndRegister(hi, "hist");

        GraphWindow.redrawAll();
        GraphWindow.highlightUpdateBasketButton(true);

        d.accept();
    }
                     );
    d.exec();
}

void ADrawExplorerWidget::median(ADrawObject &obj)
{
    TH1* hist = dynamic_cast<TH1*>(obj.Pointer);
    if (!hist)
    {
        guitools::message("This operation requires TH1 ROOT object", &GraphWindow);
        return;
    }

    QDialog d;
    QVBoxLayout * lMain = new QVBoxLayout(&d);

    QHBoxLayout* l = new QHBoxLayout();
          QLabel* lab = new QLabel("Span in bins:");
          l->addWidget(lab);
          QSpinBox * sbSpan = new QSpinBox();
          sbSpan->setMinimum(2);
          sbSpan->setValue(6);
          l->addWidget(sbSpan);
    lMain->addLayout(l);

    QPushButton * pb = new QPushButton("Apply median filter");
    lMain->addWidget(pb);

    QObject::connect(pb, &QPushButton::clicked,
                     [&d, hist, sbSpan, this]()
    {
        int span = sbSpan->value();

        int deltaLeft  = ( span % 2 == 0 ? span / 2 - 1 : span / 2 );
        int deltaRight = span / 2;

        std::vector<double> Filtered;
        int num = hist->GetNbinsX();
        for (int iThisBin = 1; iThisBin <= num; iThisBin++)  // 0-> underflow; num+1 -> overflow
        {
            std::vector<double> content;
            for (int i = iThisBin - deltaLeft; i <= iThisBin + deltaRight; i++)
            {
                if (i < 1 || i > num) continue;
                content.push_back( hist->GetBinContent(i) );
            }

            std::sort(content.begin(), content.end());
            int size = content.size();
            double val;
            if (size == 0) val = 0;
            else val = ( size % 2 == 0 ? (content[size / 2 - 1] + content[size / 2]) / 2 : content[size / 2] );

            Filtered.push_back(val);
        }

        TH1 * hc = dynamic_cast<TH1*>(hist->Clone());
        if (!hc)
        {
            qWarning() << "Failed to clone the histogram!";
            return;
        }
        for (int iThisBin = 1; iThisBin <= num; iThisBin++)
            hc->SetBinContent(iThisBin, Filtered.at(iThisBin-1));

        GraphWindow.makeCopyOfDrawObjects();
        GraphWindow.makeCopyOfActiveBasketId();
        GraphWindow.clearBasketActiveId();

        DrawObjects.clear();
        addToDrawObjectsAndRegister(hc, "hist");

        GraphWindow.redrawAll();

        d.accept();
    }
                     );

    d.exec();
}

#include "TH3D.h"
void ADrawExplorerWidget::projection(ADrawObject &obj, int axis)
{
    TH3 * h3 = dynamic_cast<TH3*>(obj.Pointer);
    if (h3)
    {
        TProfile2D * proj;

        if (axis == 0)
            proj = h3->Project3DProfile("yzo");
        else if (axis == 1)
            proj = h3->Project3DProfile("xzo");
        else
            proj = h3->Project3DProfile("xyo");

        qDebug() << "Projection:" << proj;

        if (proj)
        {
            GraphWindow.makeCopyOfDrawObjects();
            GraphWindow.makeCopyOfActiveBasketId();
            GraphWindow.clearBasketActiveId();

            DrawObjects.clear();
            addToDrawObjectsAndRegister((TH2D*)proj, "colz");

            GraphWindow.redrawAll();
        }
        return;
    }

    TH2* h = dynamic_cast<TH2*>(obj.Pointer);
    if (!h)
    {
        guitools::message("This operation requires TH2 or TH2 ROOT object", &GraphWindow);
        return;
    }

    TH1D* proj;
    if (axis == 0)
        proj = h->ProjectionX();
    else
        proj = h->ProjectionY();

    if (proj)
    {
        GraphWindow.makeCopyOfDrawObjects();
        GraphWindow.makeCopyOfActiveBasketId();
        GraphWindow.clearBasketActiveId();

        DrawObjects.clear();
        addToDrawObjectsAndRegister(proj, "hist");

        GraphWindow.redrawAll();
    }
}

void ADrawExplorerWidget::customProjection(ADrawObject & obj)
{
    TH2* hist = dynamic_cast<TH2*>(obj.Pointer);
    if (!hist)
    {
        guitools::message("This operation requires TH2 ROOT object selected", &GraphWindow);
        return;
    }

    objForCustomProjection = hist;  // tried safer approach  with clone, but got random error in ROOT on clone attempt
    GraphWindow.showProjectionTool();
}

#include "aaxesdialog.h"
#include "TGraph2D.h"
void ADrawExplorerWidget::editAxis(ADrawObject & obj, int axisIndex)
{
    std::vector<TAxis*> axes;

    if (dynamic_cast<TGraph*>(obj.Pointer))
    {
        TGraph * g = dynamic_cast<TGraph*>(obj.Pointer);
        axes = { g->GetXaxis(), g->GetYaxis(), nullptr };
    }
    else if (dynamic_cast<TH1*>(obj.Pointer))
    {
        TH1* h = dynamic_cast<TH1*>(obj.Pointer);
        axes = { h->GetXaxis(), h->GetYaxis(), h->GetZaxis() };
    }
    else if (dynamic_cast<TGraph2D*>(obj.Pointer))
    {
        TGraph2D * g = dynamic_cast<TGraph2D*>(obj.Pointer);
        axes = { g->GetXaxis(), g->GetYaxis(), g->GetZaxis() };
    }
    else
    {
        guitools::message("Not supported for this object type", &GraphWindow);
        return;
    }

    AAxesDialog D(axes, axisIndex, this);
    connect(&D, &AAxesDialog::requestRedraw, &GraphWindow, &AGraphWindow::redrawAll);
    D.exec();

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

#include "asetmarginsdialog.h"
#include "a3global.h"
void ADrawExplorerWidget::setCustomMargins(ADrawObject & obj)
{
    const A3Global & GlobSet = A3Global::getConstInstance();

    ADrawMarginsRecord rec = ( obj.CustomMargins.Override ? obj.CustomMargins : GlobSet.DefaultDrawMargins );

    ASetMarginsDialog d(rec, GlobSet.DefaultDrawMargins, this);
    int res = d.exec();

    if (res == QDialog::Rejected) return;

    obj.CustomMargins = d.getResult();

    if (d.isDefaultSelected()) obj.CustomMargins.Override = false;
    else obj.CustomMargins.Override = true;

    GraphWindow.updateMargins(&obj);
    GraphWindow.doRedrawOnUpdateMargins();
    GraphWindow.highlightUpdateBasketButton(true);
}

QString ADrawExplorerWidget::generateOptionForSecondaryAxis(int axisIndex, double u1, double u2)
{
    double cx1 = GraphWindow.getCanvasMinX();
    double cx2 = GraphWindow.getCanvasMaxX();
    double cy1 = GraphWindow.getCanvasMinY();
    double cy2 = GraphWindow.getCanvasMaxY();

    double A, B;
    QString axisOpt;

    if (axisIndex == 1)
    {
        //Right axis
        A = (u1 - u2) / (cy1 - cy2);
        B = u1 - A * cy1;
        axisOpt = "+L";
    }
    else
    {
        //Top axis
        A = (u1 - u2) / (cx1 - cx2);
        B = u1 - A * cx1;
        axisOpt = "-L";
    }

    return QString("same;%1;%2;%3").arg(axisIndex == 0 ? "X" : "Y").arg(A).arg(B);
}

void ADrawExplorerWidget::constructIconForObject(QIcon & icon, const ADrawObject & drObj)
{
    const TObject * tObj = drObj.Pointer;
    const QString   Type = tObj->ClassName();
    const QString & Opt  = drObj.Options;

    const TAttLine   * line = nullptr;
    const TAttMarker * mark = nullptr;
    const TAttFill   * fill = nullptr;

    if (Opt.contains("colz", Qt::CaseInsensitive))
    {
        construct2DIcon(icon);
        return;
    }

    if (Type.startsWith("TH2") || Type.startsWith("TF2") || Type.startsWith("TGraph2D"))
    {
        //todo: invent something 3D-ish to show surf and lego
        construct2DIcon(icon);
        return;
    }

    if (Type.startsWith("TF"))
        line = dynamic_cast<const TAttLine*>(tObj);
    else if (Type.startsWith("TH") || Type.startsWith("TGraph") || Type.startsWith("TProfile"))
    {
        line = dynamic_cast<const TAttLine*>(tObj);
        mark = dynamic_cast<const TAttMarker*>(tObj);
        fill = dynamic_cast<const TAttFill*>(tObj);
    }

    if (Type.startsWith("TH1") || Type.startsWith("TProfile"))
    {
        // * always enables * markers
        // C or L disables P and forces line
        //in all other cases presence of P enables markers and removes line
        if (!Opt.contains('*', Qt::CaseInsensitive))
        {
            if ( !Opt.contains('P', Qt::CaseInsensitive) || Opt.contains('C', Qt::CaseInsensitive) || Opt.contains('L', Qt::CaseInsensitive) )
                mark = nullptr;
        }

        if ( Opt.contains('P', Qt::CaseInsensitive) && !Opt.contains('C', Qt::CaseInsensitive) && !Opt.contains('L', Qt::CaseInsensitive) )
            line = nullptr;
    }
    else if (Type.startsWith("TGraph"))
    {
        QString redOpt = Opt;
        redOpt.remove("same", Qt::CaseInsensitive);
        if (redOpt.isEmpty())
        {
            mark = nullptr;
            fill = nullptr;
        }
        else
        {
            if (!Opt.contains('P', Qt::CaseInsensitive) && !Opt.contains('*', Qt::CaseInsensitive)) mark = nullptr;
            if (!Opt.contains('C', Qt::CaseInsensitive) && !Opt.contains('L', Qt::CaseInsensitive) && !Opt.contains('B', Qt::CaseInsensitive)) line = nullptr;
            if (!Opt.contains('F', Qt::CaseInsensitive)) fill = nullptr;
        }
    }
    else if (Type == "TLine" || Type == "TArrow")
    {
        line = dynamic_cast<const TAttLine*>(tObj);
    }
    else if (Type == "TBox" || Type == "TEllipse")
    {
        line = dynamic_cast<const TAttLine*>(tObj);
        fill = dynamic_cast<const TAttFill*>(tObj);
        if (fill->GetFillStyle() == 0) fill = nullptr;
    }

    construct1DIcon(icon, line, mark, fill);
}

void ADrawExplorerWidget::convertRootColoToQtColor(int rootColor, QColor & qtColor)
{
    TColor * tc = gROOT->GetColor(rootColor);

    if (!tc) qtColor = Qt::white;
    else
    {
        int red   = 255 * tc->GetRed();
        int green = 255 * tc->GetGreen();
        int blue  = 255 * tc->GetBlue();

        qtColor = QColor(red, green, blue);
    }
}

void ADrawExplorerWidget::construct1DIcon(QIcon & icon, const TAttLine * line, const TAttMarker * marker, const TAttFill * fill)
{
    QPixmap pm(IconWidth, IconHeight);
    QPainter Painter(&pm);
    Painter.setRenderHint(QPainter::Antialiasing, false);
    Painter.setRenderHint(QPainter::TextAntialiasing, false);
    Painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
//    Painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
    QColor Color;

    // Background of FillColor
    int RootColor = ( fill ? fill->GetFillColor() : -1 );
    convertRootColoToQtColor(RootColor, Color);
    pm.fill(Color);

    // Line
    if (line)
    {
        RootColor     = line->GetLineColor();
        convertRootColoToQtColor(RootColor, Color);

        int LineWidth = 2 * line->GetLineWidth();
        if (LineWidth > 10) LineWidth = 10;

        Qt::PenStyle style = Qt::SolidLine;
        switch (line->GetLineStyle())
        {
        case 2: case 7: case 9:
            style = Qt::DashLine; break;
        case 3:
            style = Qt::DotLine; break;
        case 4: case 5: case 10:
            style = Qt::DashDotLine; break;
        case 6: case 8:
            style = Qt::DashDotDotLine; break;
        }

        Painter.setPen(QPen(Color, LineWidth, style, Qt::SquareCap, Qt::BevelJoin));
        Painter.drawLine(0, 0.5*IconHeight, IconWidth, 0.5*IconHeight);
    }

    // Marker
    if (marker)
    {
        RootColor = marker->GetMarkerColor();
        convertRootColoToQtColor(RootColor, Color);
        Painter.setBrush(QBrush(Color));
        //Painter.setPen(QPen(Qt::NoPen));
        Painter.setPen(QPen(Color, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
        int Diameter = 20;
        Painter.drawEllipse( 0.5*IconWidth - 0.5*Diameter, 0.5*IconHeight - 0.5*Diameter, Diameter, Diameter );
    }

    icon = QIcon(pm);
}

void ADrawExplorerWidget::construct2DIcon(QIcon &icon)
{
    QPixmap pm(IconWidth, IconHeight);
    QPainter Painter(&pm);
    Painter.setRenderHint(QPainter::Antialiasing, false);
    Painter.setRenderHint(QPainter::TextAntialiasing, false);
    Painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
//    Painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
    QColor Color;

    static const TArrayI & Palette = TColor::GetPalette();
    int Size = Palette.GetSize();
    //qDebug() << "Palette size:" << Size;
    if (Size == 0) return;

    int RootColor = Palette.At(0);
    convertRootColoToQtColor(RootColor, Color);
    Painter.setBrush(QBrush(Color));
    Painter.setPen(QPen(Qt::NoPen));
    Painter.drawRect(0, 0, IconWidth, IconHeight);

    int Divisions = 4;
    int Delta = Size / Divisions;
    for (int i = 1; i <= Divisions; i++)
    {
        int iColor = Delta * i + 1;
        if (iColor >= Size) iColor = Size-1;

        RootColor = Palette.At(iColor);
        convertRootColoToQtColor(RootColor, Color);
        Painter.setBrush(QBrush(Color));
        //Painter.setPen(Color);

        double fraction = (1.0 - 1.0* i / (Divisions+1));
        int w = fraction * IconWidth;
        int h = fraction * IconHeight;
        //qDebug() << "Step:" << i << "iinA/Size"<< iColor << "color:" << RootColor << "w:"<<w << "h:" << h;
        Painter.drawEllipse( 0.5*IconWidth - 0.5*w, 0.5*IconHeight - 0.5*h, w, h );
    }

    icon = QIcon(pm);
}

void ADrawExplorerWidget::addAxis(int axisIndex)
{
    TGaxis *axis = new TGaxis(0,0,1,1,  0,1, 510, (axisIndex == 1 ? "+L" : "-L") );
    axis->SetLabelFont(42);
    axis->SetTextFont(42);
    axis->SetLabelSize(0.035);
    const QString opt = generateOptionForSecondaryAxis(axisIndex, 0, 1.0);
    DrawObjects.push_back( ADrawObject(axis, opt) );

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::saveRoot(ADrawObject &obj)
{
    QString fileName = guitools::dialogSaveFile(this, "Save ROOT object", "Root files(*.root)");
    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".root";

    TObject * tobj = obj.Pointer;

    //exceptions first
    TF2 * tf2 = dynamic_cast<TF2*>(tobj);
    if (tf2) tobj = tf2->GetHistogram();
    else
    {
        TF1 * tf1 = dynamic_cast<TF1*>(tobj);
        if (tf1) tobj = tf1->GetHistogram();
    }

    tobj->SaveAs(fileName.toLatin1().data());  // sometimes crashes on load!
}

void ADrawExplorerWidget::saveAsTxt(ADrawObject & obj, bool fUseBinCenters)
{
    TObject * tobj = obj.Pointer;
    QString cn = tobj->ClassName();

    if (cn != "TGraph" && cn != "TGraphErrors" && !cn.startsWith("TH1") && !cn.startsWith("TH2") && cn != "TF1" && cn != "TF2")
    {
        guitools::message("Not implemented for this object type", &GraphWindow);
        return;
    }

    //exceptions first
    TF2 * tf2 = dynamic_cast<TF2*>(tobj);
    if (tf2) tobj = tf2->GetHistogram();
    else
    {
        TF1 * tf1 = dynamic_cast<TF1*>(tobj);
        if (tf1) tobj = tf1->GetHistogram();
    }

    QString fileName = guitools::dialogSaveFile(this, "Save as text", "Text files(*.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".txt";

    TH2 * h2 = dynamic_cast<TH2*>(tobj);
    if (h2)
    {
        std::vector<double> x, y, f;
        for (int iX=1; iX<h2->GetNbinsX()+1; iX++)
            for (int iY=1; iY<h2->GetNbinsY()+1; iY++)
            {
                const double X = (fUseBinCenters ? h2->GetXaxis()->GetBinCenter(iX) : h2->GetXaxis()->GetBinLowEdge(iX));
                const double Y = (fUseBinCenters ? h2->GetYaxis()->GetBinCenter(iY) : h2->GetYaxis()->GetBinLowEdge(iY));
                x.push_back(X);
                y.push_back(Y);

                int iBin = h2->GetBin(iX, iY);
                double F = h2->GetBinContent(iBin);
                f.push_back(F);
            }

        std::vector<std::vector<double> *> V = {&x, &y, &f};
        ftools::saveDoubleVectorsToFile(V, fileName);
        return;
    }

    TH1 * h1 = dynamic_cast<TH1*>(tobj);
    if (h1)
    {
        std::vector<double> x,y;
        if (fUseBinCenters)
        {
            for (int i=1; i<h1->GetNbinsX()+1; i++)
            {
                x.push_back(h1->GetBinCenter(i));
                y.push_back(h1->GetBinContent(i));
            }
        }
        else
        { //bin starts
            for (int i=1; i<h1->GetNbinsX()+2; i++)  // *** should it be +2?
            {
                x.push_back(h1->GetBinLowEdge(i));
                y.push_back(h1->GetBinContent(i));
            }
        }

        //SaveDoubleVectorsToFile(fileName, &x, &y);
        QString err = ftools::saveDoubleVectorsToFile( {&x, &y}, fileName );
        if (!err.isEmpty()) guitools::message(err, this); // before GraphWindow was the parent
        return;
    }

    TGraph* g = dynamic_cast<TGraph*>(tobj);
    if (g)
    {
        std::vector<double> x,y;
        for (int i = 0; i < g->GetN(); i++)
        {
            double xx, yy;
            int ok = g->GetPoint(i, xx, yy);
            if (ok != -1)
            {
                x.push_back(xx);
                y.push_back(yy);
            }
        }
        //SaveDoubleVectorsToFile(fileName, &x, &y);
        QString err = ftools::saveDoubleVectorsToFile( {&x, &y}, fileName );
        if (!err.isEmpty()) guitools::message(err, this); // before GraphWindow was the parent
        return;
    }

    qWarning() << "Unsupported type:" << cn;
}

void ADrawExplorerWidget::extract(ADrawObject &obj)
{
    if (!obj.Pointer) return;

    QString cType = obj.Pointer->ClassName();
    if (cType == "TLegend")
    {
        guitools::message("Unsupported object type", &GraphWindow);
        return;
    }

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();
    GraphWindow.clearBasketActiveId();

    ADrawObject thisObj = obj;

    //update options
    thisObj.Options.remove("same", Qt::CaseInsensitive);
    if (cType.startsWith("TGraph") && !thisObj.Options.contains('a', Qt::CaseInsensitive))
        thisObj.Options += "A";

    thisObj.Pointer = thisObj.Pointer->Clone();
    GraphWindow.registerTObject(thisObj.Pointer);

    DrawObjects.clear();
    DrawObjects.push_back( thisObj );

    GraphWindow.redrawAll();
}

void ADrawExplorerWidget::editPave(ADrawObject &obj)
{
    TPaveText * Pave = dynamic_cast<TPaveText*>(obj.Pointer);
    if (Pave)
    {
        GraphWindow.makeCopyOfDrawObjects();

        TPaveText * PaveCopy = new TPaveText(*Pave);
        GraphWindow.registerTObject(PaveCopy);
        obj.Pointer = PaveCopy;

        ATextPaveDialog D(*PaveCopy);
        connect(&D, &ATextPaveDialog::requestRedraw, &GraphWindow, &AGraphWindow::redrawAll);

        D.exec();
        GraphWindow.redrawAll();
        GraphWindow.highlightUpdateBasketButton(true);
    }
}

void ADrawExplorerWidget::copyAxisProperties(TGaxis & grAxis, TAxis & axis)
{
    axis.SetNdivisions(grAxis.GetNdiv());
    axis.SetDrawOption(grAxis.GetDrawOption());
    axis.SetTickLength(grAxis.GetTickSize());

    axis.SetTitle(grAxis.GetTitle());
    axis.SetTitleOffset(grAxis.GetTitleOffset());

    axis.SetTitleFont(grAxis.GetTextFont());
    axis.SetTitleColor(grAxis.GetTextColor());
    axis.SetTitleSize(grAxis.GetTextSize());

    axis.SetLabelColor(grAxis.GetLabelColor());
    axis.SetLabelFont(grAxis.GetLabelFont());
    axis.SetLabelOffset(grAxis.GetLabelOffset());
    axis.SetLabelSize(grAxis.GetLabelSize());
}

void ADrawExplorerWidget::copyAxisProperties(TAxis & axis, TGaxis & grAxis)
{
    grAxis.SetNdivisions(abs(axis.GetNdivisions()));
    grAxis.SetDrawOption(axis.GetDrawOption());
    grAxis.SetTickLength(axis.GetTickLength());

    grAxis.SetTitle(axis.GetTitle());
    grAxis.SetTitleOffset(axis.GetTitleOffset());
    grAxis.SetTitleFont(axis.GetTitleFont());
    grAxis.SetTitleColor(axis.GetTitleColor());
    grAxis.SetTitleSize(axis.GetTitleSize());

    grAxis.SetLabelColor(axis.GetLabelColor());
    grAxis.SetLabelFont(axis.GetLabelFont());
    grAxis.SetLabelOffset(axis.GetLabelOffset());
    grAxis.SetLabelSize(axis.GetLabelSize());
}

void ADrawExplorerWidget::editTGaxis(ADrawObject &obj)
{
    int axisIndex = 1;
    if (obj.Options.contains("X")) axisIndex = 0;

    TGaxis * grAxis = dynamic_cast<TGaxis*>(obj.Pointer);
    if (grAxis)
    {
        TAxis axis;
        copyAxisProperties(*grAxis, axis);

        std::vector<TAxis *> vec;
        vec.push_back( &axis );
        AAxesDialog D(vec, 0, this);

        QHBoxLayout * lay = new QHBoxLayout();
        lay->addStretch();
            QLabel * lab = new QLabel("Min:");
        lay->addWidget(lab);
            QLineEdit * leMin = new QLineEdit();
            leMin->setText( QString::number(grAxis->GetWmin()) );
        lay->addWidget(leMin);
            lab = new QLabel("Max:");
        lay->addWidget(lab);
            QLineEdit * leMax = new QLineEdit();
            leMax->setText( QString::number(grAxis->GetWmax()) );
        lay->addWidget(leMax);
        lay->addStretch();
        D.addLayout(lay);

        auto UpdateTAxis = [this, leMin, leMax, grAxis, &obj, axisIndex, &axis]()
        {
            double u1 = leMin->text().toDouble();
            double u2 = leMax->text().toDouble();
            grAxis->SetWmin(u1);
            grAxis->SetWmax(u2);
            obj.Options = generateOptionForSecondaryAxis(axisIndex, u1, u2);

            copyAxisProperties(axis, *grAxis);

            GraphWindow.redrawAll();
        };
        connect(leMin, &QLineEdit::editingFinished, UpdateTAxis);
        connect(leMax, &QLineEdit::editingFinished, UpdateTAxis);

        connect(&D, &AAxesDialog::requestRedraw, UpdateTAxis);
        D.exec();

        UpdateTAxis();

        GraphWindow.highlightUpdateBasketButton(true);
    }
}

void ADrawExplorerWidget::linDraw(int index)
{
    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DLine();
    if (!Raster.waitForExtractionFinished()) return;

    double startX = Raster.Line2DstartX;
    double stopX  = Raster.Line2DstopX;
    double startY = Raster.Line2DstartY;
    double stopY  = Raster.Line2DstopY;

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    TLine *ln = new TLine(startX, startY, stopX, stopY);
    GraphWindow.registerTObject(ln);
    ln->SetLineStyle(2);
    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(ln, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::boxDraw(int index)
{
    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DBox();
    if (!Raster.waitForExtractionFinished()) return; //cancel

    double startX = Raster.extractedX1;
    double stopX  = Raster.extractedX2;
    double startY = Raster.extractedY1;
    double stopY  = Raster.extractedY2;

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    TBox *bx = new TBox(startX, startY, stopX, stopY);
    GraphWindow.registerTObject(bx);
    bx->SetLineStyle(2);
    bx->SetFillStyle(0);
    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(bx, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

#include "TEllipse.h"
void ADrawExplorerWidget::ellipseDraw(int index)
{
    GraphWindow.triggerGlobalBusy(true);

    Raster.Extract2DEllipse();
    if (!Raster.waitForExtractionFinished()) return;

    double centerX = Raster.extracted2DEllipseX;
    double centerY = Raster.extracted2DEllipseY;
    double r1      = Raster.extracted2DEllipseR1;
    double r2      = Raster.extracted2DEllipseR2;
    double theta   = Raster.extracted2DEllipseTheta;

    GraphWindow.makeCopyOfDrawObjects();
    GraphWindow.makeCopyOfActiveBasketId();

    TEllipse *el = new TEllipse(centerX, centerY, r1, r2, 0, 360, theta);
    GraphWindow.registerTObject(el);
    el->SetLineStyle(2);
    el->SetFillStyle(0);
    DrawObjects.insert(DrawObjects.begin()+index+1, ADrawObject(el, "same"));

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::onMoveUpAction(int index)
{
    if (DrawObjects.empty()) return;
    if (index == 0) return;

    if (DrawObjects.front().Multidraw)
    {
        std::swap( DrawObjects.front().MultidrawSettings.BasketItems[index], DrawObjects.front().MultidrawSettings.BasketItems[index-1] );
    }
    else
    {
        if (index == 1)
        {
            guitools::message("Cannot change the first draw object!", this);
            return;
        }
        std::swap( DrawObjects[index], DrawObjects[index-1] );
    }

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}

void ADrawExplorerWidget::onMoveDownAction(int index)
{
    if (DrawObjects.empty()) return;

    if (DrawObjects.front().Multidraw)
    {
        if (index == DrawObjects.front().MultidrawSettings.BasketItems.size()-1) return;
        std::swap( DrawObjects.front().MultidrawSettings.BasketItems[index], DrawObjects.front().MultidrawSettings.BasketItems[index+1] );
    }
    else
    {
        if (index == 0)
        {
            guitools::message("Cannot change the first draw object!", this);
            return;
        }
        if (index == DrawObjects.size()-1) return;
        std::swap( DrawObjects[index], DrawObjects[index+1] );
    }

    GraphWindow.redrawAll();
    GraphWindow.highlightUpdateBasketButton(true);
}
