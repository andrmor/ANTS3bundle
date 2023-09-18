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

void AViewer3DWidget::redraw()
{
    if (!Hist) return;

    Hist->Reset("ICESM");
    //QString title;
    QString labName;
    QString horName, vertName, offName;
    double offPos = 0;

    switch (ViewType)
    {
    case XY:
      {
        labName = "Transverse";
        horName = "X"; vertName = "Y"; offName = "Z=";
        int iz = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Zaxis, iz);
        //title = QString("Transverse (XY at Z = %0 mm)").arg(z);
        for (size_t iy = 0; iy < Viewer->NumBinsY; iy++)
            for (size_t ix = 0; ix < Viewer->NumBinsX; ix++)
                Hist->SetBinContent(ix, iy, Viewer->Data[ix][iy][iz] * Viewer->ScalingFactor);
        break;
      }
    case XZ:
      {
        labName = "Coronal";
        horName = "X"; vertName = "Z"; offName = "Y=";
        int iy = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Yaxis, iy);
        //title = QString("Coronal (XZ at Y = %0 mm)").arg(y);
        for (size_t iz = 0; iz < Viewer->NumBinsZ; iz++)
            for (size_t ix = 0; ix < Viewer->NumBinsX; ix++)
                Hist->SetBinContent(ix, iz, Viewer->Data[ix][iy][iz] * Viewer->ScalingFactor);
        break;
      }
    case YZ:
      {
        labName = "Sagittal";
        horName = "Y"; vertName = "Z"; offName = "X=";
        int ix = ui->sbPosition->value();
        offPos = Viewer->binToCenterPosition(AViewer3D::Xaxis, ix);
        //title = QString("Sagittal (YZ at X = %0 mm)").arg(x);
        for (size_t iz = 0; iz < Viewer->NumBinsZ; iz++)
            for (size_t iy = 0; iy < Viewer->NumBinsY; iy++)
                Hist->SetBinContent(iy, iz, Viewer->Data[ix][iy][iz] * Viewer->ScalingFactor);
        break;
      }
    }
    //ui->lView->setText(title);
    ui->lLabName->setText(labName + " (");
    ui->lLabHorAxisName->setText(horName);
    ui->lLabVertAxisName->setText(vertName);
    ui->lLabOffAxisName->setText(offName);
    ui->lLabHorAxisPos->setText("");
    ui->lLabVertAxisPos->setText("");
    ui->lLabOffAxisPos->setText(QString::number(offPos, 'g', 4));
    ui->lPosition->setText( QString::number(offPos) );

    switch (Viewer->MaximumMode)
    {
    case AViewer3D::GlobalMax : Hist->SetMaximum(Viewer->GlobalMaximum * Viewer->ScalingFactor); break;
    case AViewer3D::FixedMax  : Hist->SetMaximum(Viewer->FixedMaximum  * Viewer->ScalingFactor); break;
    default: break;
    }

    RasterWindow->fCanvas->cd();
    Hist->Draw("colz");
    RasterWindow->fCanvas->Update();
}

void AViewer3DWidget::onRasterCursorPositionChanged(double x, double y, bool bOn)
{
    //qDebug() << x << y << bOn;
    ui->lLabHorAxisPos->setText("=" + QString::number(x, 'g', 4));
    ui->lLabVertAxisPos->setText("=" + QString::number(y, 'g', 4));
    ui->lLabHorAxisPos->setVisible(true);
    ui->lLabVertAxisPos->setVisible(true);
}

void AViewer3DWidget::onCursorLeftRaster()
{
    //if (rect().contains(mapFromGlobal(QCursor::pos()))) return;
    //qDebug() << "leave!!!!!!!";

    //ui->lLabHorAxisPos->setText("");  // Bug in Qt: leaves ghost marks on the label, need to use vis/invisble switch to fix
    //ui->lLabVertAxisPos->setText("");
    //ui->hlLabel->update();            // does not help
    ui->lLabHorAxisPos->setVisible(false);
    ui->lLabVertAxisPos->setVisible(false);
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

