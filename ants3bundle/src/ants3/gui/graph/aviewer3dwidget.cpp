#include "TCanvas.h"

#include "aviewer3dwidget.h"
#include "ui_aviewer3dwidget.h"
#include "rasterwindowbaseclass.h"

#include "TH2D.h"

AViewer3DWidget::AViewer3DWidget(AViewer3D * viewer, EViewType viewType) :
    QWidget(viewer), Viewer(viewer), ViewType(viewType),
    ui(new Ui::AViewer3DWidget)
{
    ui->setupUi(this);

    ui->pbRedraw->setVisible(false);

    RasterWindow = new RasterWindowBaseClass(nullptr);
    RasterWindow->resize(400, 400);
    RasterWindow->ForceResize();
    RasterWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->horizontalLayout->insertWidget(0, RasterWindow);

    switch (ViewType)
    {
    case XY:
        ui->lAxis->setText("Z:");

        ui->sbPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->hsPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBins[0]-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBins[0]-1));

        Hist = new TH2D("", "", Viewer->NumBins[2], Viewer->binToEdgePosition(2, 0), Viewer->binToEdgePosition(2, Viewer->NumBins[2]),
                                Viewer->NumBins[1], Viewer->binToEdgePosition(1, 0), Viewer->binToEdgePosition(1, Viewer->NumBins[1]));
        Hist->GetXaxis()->SetTitle("X, mm");
        Hist->GetYaxis()->SetTitle("Y, mm");
        break;
    case XZ:
        ui->lAxis->setText("Y:");

        ui->sbPosition->setMaximum(Viewer->NumBins[1]-1);
        ui->hsPosition->setMaximum(Viewer->NumBins[1]-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBins[1]-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBins[1]-1));

        Hist = new TH2D("", "", Viewer->NumBins[2], Viewer->binToEdgePosition(2, 0), Viewer->binToEdgePosition(2, Viewer->NumBins[2]),
                                Viewer->NumBins[0], Viewer->binToEdgePosition(0, 0), Viewer->binToEdgePosition(0, Viewer->NumBins[0]));
        Hist->GetXaxis()->SetTitle("X, mm");
        Hist->GetYaxis()->SetTitle("Z, mm");
        break;
    case YZ:
        ui->lAxis->setText("X:");

        ui->sbPosition->setMaximum(Viewer->NumBins[2]-1);
        ui->hsPosition->setMaximum(Viewer->NumBins[2]-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBins[2]-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBins[2]-1));

        Hist = new TH2D("", "", Viewer->NumBins[1], Viewer->binToEdgePosition(1, 0), Viewer->binToEdgePosition(1, Viewer->NumBins[1]),
                                Viewer->NumBins[0], Viewer->binToEdgePosition(0, 0), Viewer->binToEdgePosition(0, Viewer->NumBins[0]));
        Hist->GetXaxis()->SetTitle("Y, mm");
        Hist->GetYaxis()->SetTitle("Z, mm");
        break;
    }

    Hist->SetStats(false);
    Hist->GetYaxis()->SetTitleOffset(1.3);
}

AViewer3DWidget::~AViewer3DWidget()
{
    ui->horizontalLayout->removeWidget(RasterWindow);
    delete RasterWindow; RasterWindow = nullptr;

    delete ui;
}

void AViewer3DWidget::redraw()
{
    Hist->Reset("ICESM");
    QString title;

    switch (ViewType)
    {
    case XY:
      {
        int iz = ui->sbPosition->value();
        double z = Viewer->binToCenterPosition(0, iz);
        title = QString("Transverse (XY at Z = %0 mm)").arg(z);
        for (size_t iy = 0; iy < Viewer->NumBins[1]; iy++)
        {
            double y = Viewer->binToEdgePosition(1, iy);
            for (size_t ix = 0; ix < Viewer->NumBins[2]; ix++)
                //Hist->Fill(ix+0.5, iy+0.5, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
                Hist->SetBinContent(ix, iy, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
        }
        break;
      }
    case XZ:
      {
        int iy = ui->sbPosition->value();
        double y = Viewer->binToCenterPosition(1, iy);
        title = QString("Coronal (XZ at Y = %0 mm)").arg(y);
        for (size_t iz = 0; iz < Viewer->NumBins[0]; iz++)
            for (size_t ix = 0; ix < Viewer->NumBins[2]; ix++)
                //Hist->Fill(ix+0.5, iz+0.5, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
                Hist->SetBinContent(ix, iz, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
        break;
      }
    case YZ:
      {
        int ix = ui->sbPosition->value();
        double x = Viewer->binToCenterPosition(2, ix);
        title = QString("Sagittal (YZ at X = %0 mm)").arg(x);
        for (size_t iz = 0; iz < Viewer->NumBins[0]; iz++)
            for (size_t iy = 0; iy < Viewer->NumBins[1]; iy++)
                //Hist->Fill(iy+0.5, iz+0.5, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
                Hist->SetBinContent(iy, iz, Viewer->Data[iz][iy][ix] * Viewer->ScalingFactor);
        break;
      }
    }
    ui->lView->setText(title);

    switch (Viewer->MaximumMode)
    {
    case AViewer3D::GlobalMax : Hist->SetMaximum(Viewer->GlobalMaximum * Viewer->ScalingFactor); break;
    case AViewer3D::FixedMax  : Hist->SetMaximum(Viewer->FixedMaximum * Viewer->ScalingFactor);  break;
    default: break;
    }

    RasterWindow->fCanvas->cd();
    Hist->Draw("colz");
    RasterWindow->fCanvas->Update();
}

void AViewer3DWidget::on_pbRedraw_clicked()
{
    redraw();
}

void AViewer3DWidget::on_pbMinus_clicked()
{
    int pos = ui->sbPosition->value();
    if (pos == 0) return;

    ui->sbPosition->setValue(pos-1);
    redraw();
}

void AViewer3DWidget::on_pbPlus_clicked()
{
    int pos = ui->sbPosition->value();
    if (pos == ui->sbPosition->maximum()) return;

    ui->sbPosition->setValue(pos+1);
    redraw();
}

void AViewer3DWidget::on_hsPosition_sliderMoved(int position)
{
    ui->sbPosition->setValue(position);
    redraw();
}

void AViewer3DWidget::on_pbUnzoom_clicked()
{
    Hist->GetXaxis()->UnZoom();
    Hist->GetYaxis()->UnZoom();
    RasterWindow->fCanvas->Modified();
    RasterWindow->fCanvas->Update();
}

void AViewer3DWidget::on_sbPosition_valueChanged(int arg1)
{
    size_t index;
    switch (ViewType)
    {
    case XY: index = 0; break;
    case XZ: index = 1; break;
    case YZ: index = 2; break;
    }

    ui->lPosition->setText( QString::number(Viewer->binToCenterPosition(index, arg1)) );
}

