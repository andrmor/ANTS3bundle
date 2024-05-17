#include "aviewer3dwidget.h"
#include "ui_aviewer3dwidget.h"
//#include "rasterwindowbaseclass.h"
#include "rasterwindowgraphclass.h"

#include "TH2D.h"
#include "TCanvas.h"

AViewer3DWidget::AViewer3DWidget(AViewer3D * viewer, EViewType viewType) :
    QWidget(viewer), Viewer(viewer), Settings(viewer->Settings), ViewType(viewType),
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
    RasterWindow->fCanvas->SetTopMargin(0.05);
    //RasterWindow->fCanvas->SetBottomMargin(0.1);

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
        ui->lAxis2->setText("Z");

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
        ui->lAxis2->setText("Y");

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
        ui->lAxis2->setText("X");

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

    double xfrom, yfrom, xto, yto;
    getShownHistRange(xfrom, yfrom, xto, yto);

    json["Xfrom"] = xfrom;
    json["Yfrom"] = yfrom;
    json["Xto"] = xto;
    json["Yto"] = yto;
}

void AViewer3DWidget::readFromJson(const QJsonObject & json)
{
    int pos = 0;
    jstools::parseJson(json, "Position", pos);
    ui->sbPosition->setValue(pos);

    double xfrom, yfrom, xto, yto;
    jstools::parseJson(json, "Xfrom", xfrom);
    jstools::parseJson(json, "Yfrom", yfrom);
    jstools::parseJson(json, "Xto", xto);
    jstools::parseJson(json, "Yto", yto);
    applyShownHistRange(xfrom, yfrom, xto, yto);
}

void AViewer3DWidget::saveImage(const QString & fileName)
{
    RasterWindow->SaveAs(fileName);
}

void AViewer3DWidget::saveRoot(const QString & fileName)
{
    Hist->SaveAs(fileName.toLatin1().data());
}

void AViewer3DWidget::exportToBasket(const QString & name)
{
    emit requestExportToBasket(Hist, "colz", name);
}

double AViewer3DWidget::getAveragedValue(int ixCenter, int iyCenter, int izCenter)
{
    const int ixFrom = ixCenter - Settings.AdjacentBeforeAfter[0].first;
    const int ixTo   = ixCenter + Settings.AdjacentBeforeAfter[0].second;
    const int iyFrom = iyCenter - Settings.AdjacentBeforeAfter[1].first;
    const int iyTo   = iyCenter + Settings.AdjacentBeforeAfter[1].second;
    const int izFrom = izCenter - Settings.AdjacentBeforeAfter[2].first;
    const int izTo   = izCenter + Settings.AdjacentBeforeAfter[2].second;

    double val = 0;
    int num = 0;
    for (int ix = ixFrom; ix <= ixTo; ix++)
    {
        if (ix < 0 || ix >= Viewer->NumBinsX) continue;
        for (int iy = iyFrom; iy < iyTo+1; iy++)
        {
            if (iy < 0 || iy >= Viewer->NumBinsY) continue;
            for (int iz = izFrom; iz < izTo+1; iz++)
            {
                if (iz < 0 || iz >= Viewer->NumBinsZ) continue;
                val += Viewer->Data[ix][iy][iz];
                num++;
            }
        }
    }
    return val / num; // there will be always at least one (in the center)
}

void AViewer3DWidget::redraw()
{
    if (!Hist) return;

    Hist->Reset("ICESM");
    double offPos = 0;

    double factor = (Settings.ApplyScaling ? 1.0/Settings.ScalingFactor : 1.0);

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
            {
                double val;
                if (!Settings.ApplyAdjacentAveraging)
                    val = Viewer->Data[ix][iy][iz];
                else
                    val = getAveragedValue(ix, iy, iz);
                Hist->SetBinContent(ix, iy, val * factor);
            }
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
            {
                double val;
                if (!Settings.ApplyAdjacentAveraging)
                    val = Viewer->Data[ix][iy][iz];
                else
                    val = getAveragedValue(ix, iy, iz);
                Hist->SetBinContent(ix, iz, val * factor);
            }
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
            {
                double val;
                if (!Settings.ApplyAdjacentAveraging)
                    val = Viewer->Data[ix][iy][iz];
                else
                    val = getAveragedValue(ix, iy, iz);
                Hist->SetBinContent(iy, iz, val * factor);
            }
        break;
      }
    }
    ui->ledPosition->setText( QString::number(offPos) );

    switch (Settings.MaximumMode)
    {
    case AViewer3DSettings::GlobalMax : Hist->SetMaximum(Viewer->GlobalMaximum * factor); break;
    case AViewer3DSettings::FixedMax  : Hist->SetMaximum(Settings.FixedMaximum * factor); break;
    default: break;
    }

    if (Settings.SuppressZero)
    {
        Hist->SetMinimum(-1e-300);
        if (Hist->GetMaximum() == 0) Hist->SetMaximum(1e-99);
    }

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
    if (!Settings.ShowPositionLines) return;

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
        Z = ui->ledPosition->text().toDouble(&ok);
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
        Y = ui->ledPosition->text().toDouble(&ok);
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
        X = ui->ledPosition->text().toDouble(&ok);
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

    ui->ledPosition->setText( QString::number(val) );
}

void AViewer3DWidget::on_ledPosition_editingFinished()
{
    double pos = ui->ledPosition->text().toDouble();
    AViewer3D::EAxis axis;

    switch (ViewType)
    {
      case XY: axis = AViewer3D::Zaxis; break;
      case XZ: axis = AViewer3D::Yaxis; break;
      case YZ: axis = AViewer3D::Xaxis; break;
    }

    int bin = Viewer->positionToBin(axis, pos);
    ui->sbPosition->setValue(bin);
    redraw();
}

#include <QDialog>
#include "TMath.h"
void AViewer3DWidget::getShownHistRange(double & xfrom, double & yfrom, double & xto, double & yto) const
{
    RasterWindow->fCanvas->GetRangeAxis(xfrom, yfrom, xto, yto);
    if (RasterWindow->fCanvas->GetLogx())
    {
        xfrom = TMath::Power(10.0, xfrom);
        xto = TMath::Power(10.0, xto);
    }
    if (RasterWindow->fCanvas->GetLogy())
    {
        yfrom = TMath::Power(10.0, yfrom);
        yto = TMath::Power(10.0, yto);
    }
}

void AViewer3DWidget::applyShownHistRange(double & xfrom, double & yfrom, double & xto, double & yto)
{
    Hist->GetXaxis()->SetRangeUser(xfrom, xto);
    Hist->GetYaxis()->SetRangeUser(yfrom, yto);
}

void AViewer3DWidget::on_pbZoom_clicked()
{
    double xfrom, yfrom, xto, yto;
    getShownHistRange(xfrom, yfrom, xto, yto);

    QDialog d(this);
    d.setWindowTitle("Apply zoom");
    QGridLayout * gl = new QGridLayout(&d);
    gl->addWidget(new QLabel("From"), 0, 1);
    gl->addWidget(new QLabel("To"),   0, 2);
    gl->addWidget(new QLabel("X:"),   1, 0);
    gl->addWidget(new QLabel("Y:"),   2, 0);

    QLineEdit * ledXfrom = new QLineEdit(QString::number(xfrom)); gl->addWidget(ledXfrom, 1,1);
    QLineEdit * ledXto   = new QLineEdit(QString::number(xto));   gl->addWidget(ledXto,   1,2);

    QLineEdit * ledYfrom = new QLineEdit(QString::number(yfrom)); gl->addWidget(ledYfrom, 2,1);
    QLineEdit * ledYto   = new QLineEdit(QString::number(yto));   gl->addWidget(ledYto,   2,2);

    QHBoxLayout * hl = new QHBoxLayout();
    QPushButton * pbAccept = new QPushButton("Accept"); hl->addWidget(pbAccept);
    QPushButton * pbCancel = new QPushButton("Cancel"); hl->addWidget(pbCancel);
    gl->addLayout(hl, 3, 0, 1, 3);

    connect(pbAccept, &QPushButton::clicked, &d, &QDialog::accept);
    connect(pbCancel, &QPushButton::clicked, &d, &QDialog::reject);

    QDoubleValidator * dv = new QDoubleValidator(&d);
    ledXfrom->setValidator(dv);
    ledXto->setValidator(dv);
    ledYfrom->setValidator(dv);
    ledYto->setValidator(dv);

    int res = d.exec();
    if (res == QDialog::Rejected) return;

    xfrom = ledXfrom->text().toDouble();
    xto   = ledXto  ->text().toDouble();
    yfrom = ledYfrom->text().toDouble();
    yto   = ledYto  ->text().toDouble();

    applyShownHistRange(xfrom, yfrom, xto, yto);
    redraw();
}

