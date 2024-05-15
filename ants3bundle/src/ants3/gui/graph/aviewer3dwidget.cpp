#include "aviewer3dwidget.h"
#include "ui_aviewer3dwidget.h"
//#include "rasterwindowbaseclass.h"
#include "rasterwindowgraphclass.h"

#include "TH2D.h"
#include "TCanvas.h"

AViewer3DWidget::AViewer3DWidget(AViewer3D * viewer, EViewType viewType) :
    QWidget(viewer), Viewer(viewer), ViewType(viewType),
    ui(new Ui::AViewer3DWidget)
{
    ui->setupUi(this);

    ui->pbRedraw->setVisible(false);

    //RasterWindow = new RasterWindowBaseClass(nullptr);
    RasterWindow = new RasterWindowGraphClass(nullptr);
    RasterWindow->resize(400, 400);
    ui->horizontalLayout->insertWidget(0, RasterWindow);
    RasterWindow->ForceResize();
    RasterWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    RasterWindow->fCanvas->SetRightMargin(0.15);

    connect(RasterWindow, &RasterWindowGraphClass::reportCursorPosition, this, &AViewer3DWidget::onRasterCursorPositionChanged);
    connect(RasterWindow, &RasterWindowGraphClass::cursorLeftBoundaries, this, &AViewer3DWidget::onCursorLeftRaster);
}

AViewer3DWidget::~AViewer3DWidget()
{
    ui->horizontalLayout->removeWidget(RasterWindow);
    delete RasterWindow; RasterWindow = nullptr;

    delete ui;
}

bool AViewer3DWidget::init()
{
    delete Hist; Hist = nullptr;

    switch (ViewType)
    {
    case XY:
        ui->lAxis->setText("Z:");

        ui->sbPosition->setMaximum(Viewer->NumBinsZ-1);
        ui->hsPosition->setMaximum(Viewer->NumBinsZ-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBinsZ-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBinsZ-1));

        Hist = new TH2D("", "", Viewer->NumBinsX, Viewer->binToEdgePosition(AViewer3D::Xaxis, 0), Viewer->binToEdgePosition(AViewer3D::Xaxis, Viewer->NumBinsX),
                                Viewer->NumBinsY, Viewer->binToEdgePosition(AViewer3D::Yaxis, 0), Viewer->binToEdgePosition(AViewer3D::Yaxis, Viewer->NumBinsY));
        Hist->GetXaxis()->SetTitle("X, mm");
        Hist->GetYaxis()->SetTitle("Y, mm");
        break;
    case XZ:
        ui->lAxis->setText("Y:");

        ui->sbPosition->setMaximum(Viewer->NumBinsY-1);
        ui->hsPosition->setMaximum(Viewer->NumBinsY-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBinsY-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBinsY-1));

        Hist = new TH2D("", "", Viewer->NumBinsX, Viewer->binToEdgePosition(AViewer3D::Xaxis, 0), Viewer->binToEdgePosition(AViewer3D::Xaxis, Viewer->NumBinsX),
                                Viewer->NumBinsZ, Viewer->binToEdgePosition(AViewer3D::Zaxis, 0), Viewer->binToEdgePosition(AViewer3D::Zaxis, Viewer->NumBinsZ));
        Hist->GetXaxis()->SetTitle("X, mm");
        Hist->GetYaxis()->SetTitle("Z, mm");
        break;
    case YZ:
        ui->lAxis->setText("X:");

        ui->sbPosition->setMaximum(Viewer->NumBinsX-1);
        ui->hsPosition->setMaximum(Viewer->NumBinsX-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBinsX-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBinsX-1));

        Hist = new TH2D("", "", Viewer->NumBinsY, Viewer->binToEdgePosition(AViewer3D::Yaxis, 0), Viewer->binToEdgePosition(AViewer3D::Yaxis, Viewer->NumBinsY),
                                Viewer->NumBinsZ, Viewer->binToEdgePosition(AViewer3D::Zaxis, 0), Viewer->binToEdgePosition(AViewer3D::Zaxis, Viewer->NumBinsZ));
        Hist->GetXaxis()->SetTitle("Y, mm");
        Hist->GetYaxis()->SetTitle("Z, mm");
        break;
    }

    Hist->SetStats(false);
    Hist->GetYaxis()->SetTitleOffset(1.3);
    return true;
}

#include "ajsontools.h"
void AViewer3DWidget::writeToJson(QJsonObject & json) const
{
    json["Position"] = ui->sbPosition->value();
    // zoom of histo?
}

void AViewer3DWidget::readFromJson(const QJsonObject & json)
{
    int pos = 0;
    jstools::parseJson(json, "Position", pos);
    ui->sbPosition->setValue(pos);
}

void AViewer3DWidget::redraw()
{
    if (!Hist) return;

    Hist->Reset("ICESM");
    double offPos = 0;

    switch (ViewType)
    {
    case XY:
      {
        //labName = "Transverse";
        //horName = "X"; vertName = "Y"; offName = "Z=";
        int iz = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Zaxis, iz);
        //title = QString("Transverse (XY at Z = %0 mm)").arg(z);
        for (int iy = 0; iy < Viewer->NumBinsY; iy++)
            for (int ix = 0; ix < Viewer->NumBinsX; ix++)
                Hist->SetBinContent(ix, iy, Viewer->Data[ix][iy][iz] / Viewer->Settings.ScalingFactor);
        break;
      }
    case XZ:
      {
        //labName = "Coronal";
        //horName = "X"; vertName = "Z"; offName = "Y=";
        int iy = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Yaxis, iy);
        //title = QString("Coronal (XZ at Y = %0 mm)").arg(y);
        for (int iz = 0; iz < Viewer->NumBinsZ; iz++)
            for (int ix = 0; ix < Viewer->NumBinsX; ix++)
                Hist->SetBinContent(ix, iz, Viewer->Data[ix][iy][iz] / Viewer->Settings.ScalingFactor);
        break;
      }
    case YZ:
      {
        //labName = "Sagittal";
        //horName = "Y"; vertName = "Z"; offName = "X=";
        int ix = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Xaxis, ix);
        //title = QString("Sagittal (YZ at X = %0 mm)").arg(x);
        for (int iz = 0; iz < Viewer->NumBinsZ; iz++)
            for (int iy = 0; iy < Viewer->NumBinsY; iy++)
                Hist->SetBinContent(iy, iz, Viewer->Data[ix][iy][iz] / Viewer->Settings.ScalingFactor);
        break;
      }
    }
    ui->lPosition->setText( QString::number(offPos) );

    switch (Viewer->Settings.MaximumMode)
    {
    case AViewer3DSettings::GlobalMax : Hist->SetMaximum(Viewer->GlobalMaximum         / Viewer->Settings.ScalingFactor); break;
    case AViewer3DSettings::FixedMax  : Hist->SetMaximum(Viewer->Settings.FixedMaximum / Viewer->Settings.ScalingFactor); break;
    default: break;
    }

    if (Viewer->Settings.SuppressZero) Hist->SetMinimum(-1e-300);

    RasterWindow->fCanvas->cd();
    Hist->Draw("colz");
    RasterWindow->fCanvas->Update();

    //qDebug() << "Did redraw for" << horName << vertName;
}

void AViewer3DWidget::showCrossHair(double hor, double vert)
{
    RasterWindow->drawCrassHair(hor, vert);
}

void AViewer3DWidget::requestShowCrossHair(double x, double y, double z)
{
    if (!Viewer->Settings.ShowPositionLines) return;

    switch (ViewType)
    {
    case XY:
    {
        //qDebug() << "XY received" << x << y << z;
        showCrossHair(x, y);
        break;
    }
    case XZ:
    {
        //qDebug() << "XZ received" << x << y << z;
        showCrossHair(x, z);
        break;
    }
    case YZ:
    {
        //qDebug() << "YZ received" << x << y << z;
        showCrossHair(y, z);
        break;
    }
    }
}

void AViewer3DWidget::onRasterCursorPositionChanged(double x, double y, bool)
{
    //qDebug() << x << y << bOn;
    //ui->lLabHorAxisPos->setText("=" + QString::number(x, 'g', 4));
    //ui->lLabVertAxisPos->setText("=" + QString::number(y, 'g', 4));
    //ui->lLabHorAxisPos->setVisible(true);
    //ui->lLabVertAxisPos->setVisible(true);

    bool ok;
    double X, Y, Z; // absolute coordinates
    int horBin, verBin;
    double val = 0; // hist value in the bin under cursor
    switch (ViewType)
    {
     case XY:
      {
        //qDebug() << "Emitting from XY";
        Z = ui->lPosition->text().toDouble(&ok);
        if (!ok) Z = 0;
        X = x;
        Y = y;

        horBin = 1 + Viewer->positionToBin(AViewer3D::Xaxis, x);
        verBin = 1 + Viewer->positionToBin(AViewer3D::Yaxis, y);

        break;
      }
     case XZ:
      {
        //qDebug() << "Emitting from XZ";
        Y = ui->lPosition->text().toDouble(&ok);
        if (!ok) Y = 0;
        X = x;
        Z = y;

        horBin = 1 + Viewer->positionToBin(AViewer3D::Xaxis, x);
        verBin = 1 + Viewer->positionToBin(AViewer3D::Zaxis, y);

        break;
      }
     case YZ:
     {
        //qDebug() << "Emitting from YZ";
        X = ui->lPosition->text().toDouble(&ok);
        if (!ok) X = 0;
        Y = x;
        Z = y;

        horBin = 1 + Viewer->positionToBin(AViewer3D::Yaxis, x);
        verBin = 1 + Viewer->positionToBin(AViewer3D::Zaxis, y);

        break;
     }
    }

    if (Hist)
    {
        if (horBin > 0 && horBin <= Hist->GetNbinsX() && verBin > 0 && verBin <= Hist->GetNbinsY())
        {
            int bin = Hist->GetBin(horBin, verBin);
            val = Hist->GetBinContent(bin);
        }
    }

    emit cursorPositionChanged(X, Y, Z, val);
}

void AViewer3DWidget::onCursorLeftRaster()
{
    //if (rect().contains(mapFromGlobal(QCursor::pos()))) return;
    //qDebug() << "leave!!!!!!!";

    //ui->lLabHorAxisPos->setText("");  // Bug in Qt: leaves ghost marks on the label, need to use vis/invisble switch to fix
    //ui->lLabVertAxisPos->setText("");
    //ui->hlLabel->update();            // does not help
    //ui->lLabHorAxisPos->setVisible(false);
    //ui->lLabVertAxisPos->setVisible(false);

    emit cursorLeftVisibleArea();
}

void AViewer3DWidget::on_pbRedraw_clicked()
{
    redraw();
}

void AViewer3DWidget::on_pbMinus_clicked()
{
    if (!Hist) return;

    int pos = ui->sbPosition->value();
    if (pos == 0) return;

    ui->sbPosition->setValue(pos-1);
    redraw();
}

void AViewer3DWidget::on_pbPlus_clicked()
{
    if (!Hist) return;

    int pos = ui->sbPosition->value();
    if (pos == ui->sbPosition->maximum()) return;

    ui->sbPosition->setValue(pos+1);
    redraw();
}

void AViewer3DWidget::on_hsPosition_sliderMoved(int position)
{
    if (!Hist) return;

    ui->sbPosition->setValue(position);
    redraw();
}

void AViewer3DWidget::on_pbUnzoom_clicked()
{
    if (!Hist) return;

    Hist->GetXaxis()->UnZoom();
    Hist->GetYaxis()->UnZoom();
    RasterWindow->fCanvas->Modified();
    RasterWindow->fCanvas->Update();
}

void AViewer3DWidget::on_sbPosition_valueChanged(int arg1)
{
    if (!Hist) return;

    double val;
    switch (ViewType)
    {
    case XY: val = Viewer->binToCenterPosition(AViewer3D::Zaxis, arg1); break;
    case XZ: val = Viewer->binToCenterPosition(AViewer3D::Yaxis, arg1); break;
    case YZ: val = Viewer->binToCenterPosition(AViewer3D::Xaxis, arg1); break;
    }

    ui->lPosition->setText( QString::number(val) );
}

