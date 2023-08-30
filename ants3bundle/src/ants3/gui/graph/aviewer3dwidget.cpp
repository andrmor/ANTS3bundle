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
    ui->verticalLayout->insertWidget(1, RasterWindow);

    switch (ViewType)
    {
    case XY:
        ui->hsTop->setMaximum(Viewer->NumBins[2]-1);
        ui->hsBottom->setMaximum(Viewer->NumBins[2]-1);
        ui->hsTop->setValue(0.5*(Viewer->NumBins[2]-1));
        ui->hsBottom->setValue(0.5*(Viewer->NumBins[2]-1));

        ui->vsLeft->setMaximum(Viewer->NumBins[1]-1);
        ui->vsRight->setMaximum(Viewer->NumBins[1]-1);
        ui->vsLeft->setValue(0.5*(Viewer->NumBins[1]-1));
        ui->vsRight->setValue(0.5*(Viewer->NumBins[1]-1));

        ui->sbPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->hsPosition->setMaximum(Viewer->NumBins[0]-1);
        ui->sbPosition->setValue(0.5*(Viewer->NumBins[0]-1));
        ui->hsPosition->setValue(0.5*(Viewer->NumBins[0]-1));

        Hist = new TH2D("", "", Viewer->NumBins[2], 0, Viewer->NumBins[2],   Viewer->NumBins[1], 0, Viewer->NumBins[1]);
        Hist->SetStats(false);
        break;

    }

}

AViewer3DWidget::~AViewer3DWidget()
{
    ui->verticalLayout->removeWidget(RasterWindow);
    delete RasterWindow; RasterWindow = nullptr;

    delete ui;
}

#include "TCanvas.h"
void AViewer3DWidget::redraw()
{
    qDebug() << "aaaaaaaaaaa\n\n\n";
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

