#include "aviewer3dwidget.h"
#include "ui_aviewer3dwidget.h"
#include "rasterwindowbaseclass.h"

#include "TH2D.h"

AViewer3DWidget::AViewer3DWidget(AViewer3D * viewer, EViewType viewType) :
    QWidget(viewer), Viewer(viewer), ViewType(viewType),
    ui(new Ui::AViewer3DWidget)
{
    ui->setupUi(this);

    RasterWindow = new RasterWindowBaseClass(nullptr);
    RasterWindow->resize(400, 400);
    RasterWindow->ForceResize();
    RasterWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->horizontalLayout->insertWidget(0, RasterWindow);

    switch (ViewType)
    {
    case XY:
        ui->sbPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->hsPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBins[0]-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBins[0]-1));

        Hist = new TH2D("", "", Viewer->NumBins[2], 0, Viewer->NumBins[2],   Viewer->NumBins[1], 0, Viewer->NumBins[1]);
        Hist->GetXaxis()->SetTitle("X, mm");
        Hist->GetYaxis()->SetTitle("Y, mm");

        ui->lAxis->setText("Z:");

        break;

    }

    Hist->SetStats(false);
}

AViewer3DWidget::~AViewer3DWidget()
{
    ui->horizontalLayout->removeWidget(RasterWindow);
    delete RasterWindow; RasterWindow = nullptr;

    delete ui;
}

#include "TCanvas.h"
void AViewer3DWidget::redraw()
{
    Hist->Reset("ICESM");

    switch (ViewType)
    {
    case XY:
    {
        int iz = ui->hsPosition->value();
        for (int iy = 0; iy < Viewer->NumBins[1]; iy++)
            for (int ix = 0; ix < Viewer->NumBins[2]; ix++)
                Hist->Fill(ix+0.5, iy+0.5, Viewer->Data[iz][iy][ix]);
    }
        break;
    }

    if (Viewer->UseGlobalMaximum) Hist->SetMaximum(Viewer->GlobalMaximum);

    RasterWindow->fCanvas->cd();
    Hist->Draw("colz");
    RasterWindow->fCanvas->Update();
}

void AViewer3DWidget::on_pbRedraw_clicked()
{
    qDebug() << "Redraw button clicked!";
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

