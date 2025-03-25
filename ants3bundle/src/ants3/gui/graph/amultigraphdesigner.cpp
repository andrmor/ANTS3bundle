#include "amultigraphdesigner.h"
#include "ui_amultigraphdesigner.h"
#include "arasterwindow.h"
#include "guitools.h"
#include "abasketlistwidget.h"
#include "abasketmanager.h"
#include "ajsontools.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QDebug>
#include <QJsonArray>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QDoubleValidator>

#include <algorithm>

#include "TObject.h"
#include "TCanvas.h"
#include "TPad.h"

AMultiGraphDesigner::AMultiGraphDesigner(ABasketManager & Basket, QWidget *parent) :
    QMainWindow(parent), Basket(Basket),
    ui(new Ui::AMultiGraphDesigner)
{
    ui->setupUi(this);

    RasterWindow = new ARasterWindow(0);
    RasterWindow->resize(400, 400);
    RasterWindow->forceResize();
    ui->lMainLayout->insertWidget(0, RasterWindow);

    lwBasket = new ABasketListWidget();
    ui->lBasketLayout->addWidget(lwBasket);
    //connect(lwBasket, &ABasketListWidget::customContextMenuRequested, this, &AMultiGraphDesigner::BasketCustomContextMenuRequested);

    connect(lwBasket,     &ABasketListWidget::itemDoubleClicked, this, &AMultiGraphDesigner::onBasketItemDoubleClicked);
    connect(ui->lwCoords, &QListWidget::itemDoubleClicked,       this, &AMultiGraphDesigner::onCoordItemDoubleClicked);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    for (QLineEdit * w : qAsConst(list)) if (w->objectName().startsWith("led"))
            w->setValidator(dv);

    updateBasketGUI();
}

AMultiGraphDesigner::~AMultiGraphDesigner()
{
    //qDebug() << "Destr for multigraph window";
    clearGraphs();
    delete ui;
}

void AMultiGraphDesigner::updateBasketGUI()
{
    lwBasket->clear();
    lwBasket->addItems(Basket.getItemNames());

    //qDebug() << "---Basket items:"<< Basket.getItemNames();

    for (int i=0; i < lwBasket->count(); i++)
    {
        QListWidgetItem * item = lwBasket->item(i);
        item->setForeground(QBrush(Qt::black));
        item->setBackground(QBrush(Qt::white));
    }
}

void AMultiGraphDesigner::requestAutoconfigureAndDraw(const std::vector<int> & basketItems)
{
    clearGraphs();

    DrawOrder = basketItems;

    const int size = basketItems.size();
    if      (size == 2) fillOutBasicLayout(2, 1);
    else if (size == 3) fillOutBasicLayout(3, 1);
    else if (size == 4) fillOutBasicLayout(2, 2);
    else if (size == 5) fillOutBasicLayout(2, 3);
    else if (size == 6) fillOutBasicLayout(2, 3);
    else if (size >= 7) fillOutBasicLayout(3, 3);
}

void AMultiGraphDesigner::on_actionAs_pdf_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export multigraph\nFile suffix defines the file type", "");
    if (fileName.isEmpty()) return;

    RasterWindow->saveAs(fileName);
}

void AMultiGraphDesigner::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save this multigraph");
    if (fileName.isEmpty()) return;

    QJsonObject js;
    writeToJson(js);
    jstools::saveJsonToFile(js, fileName);

    Basket.saveAll(fileName+".basket");
}

void AMultiGraphDesigner::on_actionLoad_triggered()
{
    if (Basket.size() != 0)
    {
        bool ok = guitools::confirm("The Basket is not empty. If you proceed, the content will be replaced!\nContinue?", this);
        if (!ok) return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Load multigraph");
    if (fileName.isEmpty()) return;

    QString basketFileName = fileName + ".basket";
    if (!QFileInfo::exists(basketFileName))
    {
        guitools::message(QString("Not found corresponding basket file:\n%1").arg(basketFileName));
        return;
    }
    Basket.clear();
    QString err = Basket.appendBasket(basketFileName);
    emit basketChanged();
    updateBasketGUI();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    clearGraphs();
    DrawOrder.clear();

    QJsonObject js;
    jstools::loadJsonFromFile(js, fileName);
    err = readFromJson(js);
    if (!err.isEmpty()) guitools::message(err, this);

    updateGUI();
}

void AMultiGraphDesigner::addDraw(QListWidget * lw)
{
    const int currentRow = lw->currentRow();

    //if (DrawOrder.contains(currentRow))
    if (std::find(DrawOrder.begin(), DrawOrder.end(), currentRow) != DrawOrder.end())
        guitools::message("Already drawn!", lwBasket);
    else
    {
        DrawOrder.push_back(currentRow);
        on_pbRefactor_clicked();
    }
}

void AMultiGraphDesigner::onCoordItemDoubleClicked(QListWidgetItem *)
{
    addDraw(ui->lwCoords);
}

void AMultiGraphDesigner::onBasketItemDoubleClicked(QListWidgetItem *)
{
    addDraw(lwBasket);
}

void AMultiGraphDesigner::clearGraphs()
{
    TCanvas *c1 = RasterWindow->fCanvas;
    c1->Clear();

    for (const APadProperties & pad : Pads)
        for (const TObject * obj : pad.tmpObjects)
            delete obj;

    Pads.clear();
}

void AMultiGraphDesigner::updateGUI()
{
    updateCanvas();
    updateBasketGUI();
    updateNumbers();
}

void AMultiGraphDesigner::drawGraph(const std::vector<ADrawObject> & DrawObjects, APadProperties & pad)
{
    for (const ADrawObject & drObj : DrawObjects)
    {
        TObject * tObj = drObj.Pointer;
        tObj->Draw(drObj.Options.toLatin1().data());
        pad.tmpObjects.push_back(tObj);
    }
}

#include "TGraph.h"
#include "TH1.h"
void AMultiGraphDesigner::updateCanvas()
{
    TCanvas * canvas = RasterWindow->fCanvas;

    for (size_t iPad = 0; iPad < Pads.size(); iPad++)
    {
        APadProperties & pad = Pads[iPad];
        canvas->cd();
        pad.tPad->Draw();

        if (iPad < DrawOrder.size())
        {
            int iBasketIndex = DrawOrder.at(iPad);
            if (iBasketIndex < Basket.size() && iBasketIndex >= 0)
            {
                const std::vector<ADrawObject> DrawObjects = Basket.getCopy(iBasketIndex);
                pad.tPad->cd();

                if (!DrawObjects.empty())
                {
                    pad.tPad->SetLogx(DrawObjects.front().bLogScaleX);
                    pad.tPad->SetLogy(DrawObjects.front().bLogScaleY);

                    if (ui->cbEnforceMargins->isChecked())
                    {
                        float left = ui->ledLeft->text().toFloat();
                        float right = ui->ledRight->text().toFloat();
                        float top = ui->ledTop->text().toFloat();
                        float bot = ui->ledBottom->text().toFloat();
                        pad.tPad->SetMargin(left, right, bot, top);
                    }

                    TAxis * xAxis = nullptr;
                    TAxis * yAxis = nullptr;
                    TGraph * gr = dynamic_cast<TGraph*>(DrawObjects.front().Pointer);
                    if (gr)
                    {
                        xAxis = gr->GetXaxis();
                        yAxis = gr->GetYaxis();
                    }
                    else
                    {
                        TH1 * h = dynamic_cast<TH1*>(DrawObjects.front().Pointer);
                        if (h)
                        {
                            xAxis = h->GetXaxis();
                            yAxis = h->GetYaxis();
                        }
                    }
                    std::vector<TAxis*> axes = {xAxis, yAxis};

                    if (ui->cbScaleLabels->isChecked())
                    {
                        double factor = ui->ledScaleFactorForlabel->text().toDouble();
                        for (TAxis * axis : axes)
                            if (axis)
                            {
                                //axis.SetTitleOffset(grAxis.GetTitleOffset());
                                axis->SetTitleSize(axis->GetTitleSize()*factor);

                                //axis.SetLabelOffset(grAxis.GetLabelOffset());
                                axis->SetLabelSize(axis->GetLabelSize()*factor);
                            }
                    }

                    if (ui->cbScaleXoffsets->isChecked())
                    {
                        double factor = ui->ledScaleFactorForXoffset->text().toDouble();
                        if (xAxis)
                        {
                            xAxis->SetTitleOffset(xAxis->GetTitleOffset()*factor);
                            //axis.SetLabelOffset(grAxis.GetLabelOffset());
                        }
                    }
                    if (ui->cbScaleYoffsets->isChecked())
                    {
                        double factor = ui->ledScaleFactorForYoffset->text().toDouble();
                        if (yAxis)
                        {
                            yAxis->SetTitleOffset(yAxis->GetTitleOffset()*factor);
                            //axis.SetLabelOffset(grAxis.GetLabelOffset());
                        }
                    }

                    drawGraph(DrawObjects, pad);
                }
            }
        }
    }
    //canvas->Modified();
    canvas->Update();
}

void AMultiGraphDesigner::updateNumbers()
{
    ui->lwCoords->clear();

    const int numX = ui->sbNumX->value();
    const int numY = ui->sbNumY->value();

    int max = std::min((int)DrawOrder.size(), numX * numY);
    max     = std::min(max, Basket.size());  // paranoic

    for (int iItem = 0; iItem < Basket.size(); iItem++)
    {
        QListWidgetItem * it = new QListWidgetItem("-");
        it->setTextAlignment(Qt::AlignCenter);
        ui->lwCoords->addItem(it);
    }

    int counter = 0;
    for (int iy = 0; iy < numY; iy++)
        for (int ix = 0; ix < numX; ix++)
        {
            if (counter >= max) break;
            int iBasket = DrawOrder.at(counter);
            ui->lwCoords->item(iBasket)->setText(QString("%1-%2").arg(ix).arg(iy));
            //qDebug() << "---->" << ui->lwCoords->item(iBasket) << QString("%1-%2").arg(ix).arg(iy);
            counter++;
        }
}

void AMultiGraphDesigner::fillOutBasicLayout(int numX, int numY)
{
    if (numX < 1) numX = 1;
    if (numY < 1) numY = 1;

    ui->sbNumX->setValue(numX);
    ui->sbNumY->setValue(numY);

    double margin = 0;
    double padY   = 1.0 / numY;
    double padX   = 1.0 / numX;

    for (int iY = 0; iY < numY; iY++)
    {
        double y2 = 1.0 - (iY * padY);
        double y1 = y2 - padY;

        if (iY == 0)        y2 = 1 - margin;
        if (iY == numY - 1) y1 = margin;

        for (int iX = 0; iX < numX; iX++)
        {
            double x1 = iX * padX;
            double x2 = x1 + padX;

            if (iX == 0)        x1 = margin;
            if (iX == numX - 1) x2 = 1.0 - margin;

            QString padName = "pad" + QString::number(iX) + "_" + QString::number(iY);

            TPad * ipad = new TPad(padName.toLatin1().data(), "", x1, y1, x2, y2);
            APadProperties apad(ipad);

            Pads.push_back(apad);
        }
    }

    updateGUI();
}

void AMultiGraphDesigner::writeToJson(QJsonObject & json)
{
    json["NumX"] = ui->sbNumX->value();
    json["NumY"] = ui->sbNumY->value();

    QJsonArray ar;
    for (APadProperties & p : Pads)
    {
        QJsonObject js;
        p.updatePadGeometry();
        p.writeToJson(js);
        ar << js;
    }
    json["Pads"] = ar;

    QJsonArray arI;
    for (int i : DrawOrder) arI << i;
    json["DrawOrder"] = arI;

    json["WinWidth"]  = width();
    json["WinHeight"] = height();
}

QString AMultiGraphDesigner::readFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    if (!jstools::parseJson(json, "Pads", ar)) return "Wrong file format!";

    const int size = ar.size();
    if (size == 0) return "No pads in the file!";

    for (int i = 0; i < size; i++)
    {
        QJsonObject js = ar.at(i).toObject();

        APadProperties newPad;
        newPad.readFromJson(js);
        Pads.push_back(newPad);
    }

    int numX = 2, numY = 1;
    jstools::parseJson(json, "NumX", numX);
    jstools::parseJson(json, "NumY", numY);
    ui->sbNumX->setValue(numX);
    ui->sbNumY->setValue(numY);

    QJsonArray arI;
    jstools::parseJson(json, "DrawOrder", arI);
    for (int i = 0; i < arI.size(); i++)
        DrawOrder.push_back( arI[i].toInt() );

    int w = 600, h = 400;
    jstools::parseJson(json, "WinWidth", w);
    jstools::parseJson(json, "WinHeight", h);
    resize(w, h);

    return "";
}

QString AMultiGraphDesigner::PadsToString()
{
    QString str;
    for (const APadProperties & pad : Pads)
    {
        str += "{";
        str += pad.toString();
        str += "}, ";
    }
    str.chop(2);
    return str;
}

void AMultiGraphDesigner::on_pbRefactor_clicked()
{
    clearGraphs();
    fillOutBasicLayout(ui->sbNumX->value(), ui->sbNumY->value());
}

#include <QTimer>
bool AMultiGraphDesigner::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
    {
        RasterWindow->updateRootCanvas();
    }

    if (event->type() == QEvent::Show)
    {
        if (bColdStart)
        {
            //first time this window is shown
            bColdStart = false;
            this->resize(width()+1, height());
            this->resize(width()-1, height());
        }
        else
        {
            //qDebug() << "Graph win show event";
            //updateGUI();
            //RasterWindow->UpdateRootCanvas();
            QTimer::singleShot(100, RasterWindow, [this](){RasterWindow->updateRootCanvas();}); // without delay canvas is not shown in Qt 5.9.5
        }
    }

    return QMainWindow::event(event);
}

#include <QCloseEvent>
void AMultiGraphDesigner::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}

void AMultiGraphDesigner::on_pbClear_clicked()
{
    DrawOrder.clear();
    clearGraphs();
    updateGUI();
}

void AMultiGraphDesigner::on_cbEnforceMargins_clicked()
{
    updateCanvas();
}

void AMultiGraphDesigner::checkMargin(QLineEdit * le)
{
    float val = le->text().toFloat();
    if (val < 0 || val > 1)
    {
        le->setText("0.1");
        guitools::message("The margin value should be in th erange of 0..1", this);
        return;
    }

    if (ui->cbEnforceMargins->isChecked()) updateCanvas();
}

void AMultiGraphDesigner::on_ledLeft_editingFinished()
{
    checkMargin(ui->ledLeft);
}

void AMultiGraphDesigner::on_ledRight_editingFinished()
{
    checkMargin(ui->ledRight);
}

void AMultiGraphDesigner::on_ledTop_editingFinished()
{
    checkMargin(ui->ledTop);
}

void AMultiGraphDesigner::on_ledBottom_editingFinished()
{
    checkMargin(ui->ledBottom);
}

void AMultiGraphDesigner::on_cbScaleLabels_clicked()
{
    updateCanvas();
}

void AMultiGraphDesigner::on_ledScaleFactorForlabel_editingFinished()
{
    if (ui->cbScaleLabels->isChecked()) updateCanvas();
}

void AMultiGraphDesigner::on_cbScaleXoffsets_clicked()
{
    updateCanvas();
}

void AMultiGraphDesigner::on_cbScaleYoffsets_clicked()
{
    updateCanvas();
}

void AMultiGraphDesigner::on_ledScaleFactorForXoffset_editingFinished()
{
    if (ui->cbScaleXoffsets->isChecked()) updateCanvas();
}

void AMultiGraphDesigner::on_ledScaleFactorForYoffset_editingFinished()
{
    if (ui->cbScaleYoffsets->isChecked()) updateCanvas();
}

