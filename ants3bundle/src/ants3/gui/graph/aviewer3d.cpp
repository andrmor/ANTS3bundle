#include "aviewer3d.h"
#include "ui_aviewer3d.h"
#include "aviewer3dwidget.h"
#include "afiletools.h"
#include "guitools.h"

#include <QFileInfo>
#include <QComboBox>
#include <QDoubleValidator>
#include <QDebug>

#include <iostream>
#include <fstream>
#include <ostream>
#include <ios>

AViewer3D::AViewer3D(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::AViewer3D)
{
    ui->setupUi(this);

    blockSignals(true);
    Palettes = { {"Default ROOT",57}, {"Deep sea",51}, {"Grey scale",52}, {"Dark body radiator",53}, {"Two color hue",54}, {"Rainbow",55}, {"Inverted dark body",56} };
    for (const auto & p : Palettes)
        ui->cobPalette->addItem(p.first);
    blockSignals(false);

    createViewWidgets();

    QDoubleValidator * dval = new QDoubleValidator(this);
    ui->ledMaximum->setValidator(dval);
    ui->ledScaling->setValidator(dval);

    on_cobMaximum_activated(0);

    PercentFieldOfView = ui->sbMaxInFractionFoV->value();

    StartUp = false;
}

void AViewer3D::configureConnections(AViewer3DWidget * from, AViewer3DWidget * to1, AViewer3DWidget * to2)
{
    connect(from, &AViewer3DWidget::cursorPositionChanged, to1, &AViewer3DWidget::requestShowCrossHair);
    connect(from, &AViewer3DWidget::cursorPositionChanged, to2, &AViewer3DWidget::requestShowCrossHair);
    connect(from, &AViewer3DWidget::cursorLeftVisibleArea, to1, &AViewer3DWidget::redraw);
    connect(from, &AViewer3DWidget::cursorLeftVisibleArea, to2, &AViewer3DWidget::redraw);
}

void AViewer3D::createViewWidgets()
{
    View1 = new AViewer3DWidget(this, AViewer3DWidget::XY);
    View2 = new AViewer3DWidget(this, AViewer3DWidget::XZ);
    View3 = new AViewer3DWidget(this, AViewer3DWidget::YZ);

    ui->horizontalLayout->insertWidget(0, View1);
    ui->horizontalLayout->insertWidget(1, View2);
    ui->horizontalLayout->insertWidget(2, View3);

    configureConnections(View1, View2, View3);
    configureConnections(View2, View1, View3);
    configureConnections(View3, View1, View2);
}

void AViewer3D::updateGui()
{
    View1->redraw();
    View2->redraw();
    View3->redraw();
}

void AViewer3D::calculateGlobalMaximum()
{
    const double removedFraction = 1.0 - ui->sbMaxInFractionFoV->value() / 100.0;

    const size_t minX = 0.5 * removedFraction * NumBinsX;
    const size_t maxX = NumBinsX - minX;

    const size_t minY = 0.5 * removedFraction * NumBinsY;
    const size_t maxY = NumBinsY - minY;

    const size_t minZ = 0.5 * removedFraction * NumBinsZ;
    const size_t maxZ = NumBinsZ - minZ;

    GlobalMaximum = 0;
    for (size_t iz = minZ; iz < maxZ; iz++)
        for (size_t iy = minY; iy < maxY; iy++)
            for (size_t ix = minX; ix < maxX; ix++)
                if (Data[ix][iy][iz] > GlobalMaximum) GlobalMaximum = Data[ix][iy][iz];
}

AViewer3D::~AViewer3D()
{
    delete ui;
}

bool AViewer3D::loadCastorImage(const QString & castorFileName)
{
    bool ok = doLoadCastorImage(castorFileName);
    if (!ok)
    {
        guitools::message(ErrorString, this);
        return false;
    }

    View1->init();
    View2->init();
    View3->init();

    updateGui();

    return true;
}

double AViewer3D::binToEdgePosition(EAxis axis, size_t iBin) const
{
    //return StartZeroBin[iDimension] + iBin * Scaling_mmPerPixel[iDimension];

    switch (axis)
    {
    case Xaxis: return StartZeroBinX + iBin * mmPerPixelX;
    case Yaxis: return StartZeroBinY + iBin * mmPerPixelY;
    case Zaxis: return StartZeroBinZ + iBin * mmPerPixelZ;
    }
    return 0; // just to avoid the warning
}

double AViewer3D::binToCenterPosition(EAxis axis, size_t iBin) const
{
    //return StartZeroBin[iDimension] + (0.5 + iBin) * Scaling_mmPerPixel[iDimension];

    switch (axis)
    {
    case Xaxis: return StartZeroBinX + (0.5 + iBin) * mmPerPixelX;
    case Yaxis: return StartZeroBinY + (0.5 + iBin) * mmPerPixelY;
    case Zaxis: return StartZeroBinZ + (0.5 + iBin) * mmPerPixelZ;
    }
    return 0; // just to avoid the warning
}

bool AViewer3D::doLoadCastorImage(const QString & fileName)
{
    QFileInfo fi(fileName);
    if (fi.suffix() != "hdr")
    {
        ErrorString = "File name should have suffix 'hdr'";
        return false;
    }

    QString header;
    bool ok = ftools::loadTextFromFile(header, fileName);
    if (!ok || header.isEmpty())
    {
        ErrorString = "Could not read image header file or it is empty";
        return false;
    }

    const QStringList sl = header.split('\n', Qt::SkipEmptyParts);

    for (const QString & line : sl)
    {
        QString txt = line.simplified();
        if (txt.isEmpty()) continue;

        qDebug() << txt;
        QStringList fields = txt.split(":=", Qt::SkipEmptyParts);
        if (fields.size() != 2) continue;

        const QString key = fields.front();
        bool ok;
        if (key.contains("!matrix size"))
        {
            int num = fields[1].toInt(&ok);
            if (!ok || num < 1)
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            if      (key.contains('1')) NumBinsX = num;
            else if (key.contains('2')) NumBinsY = num;
            else if (key.contains('3')) NumBinsZ = num;
            else
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            continue;
        }

        if (key.contains("scaling factor (mm/pixel)"))
        {
            double val = fields[1].toDouble(&ok);
            if (!ok)
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            if      (key.contains('1')) mmPerPixelX = val;
            else if (key.contains('2')) mmPerPixelY = val;
            else if (key.contains('3')) mmPerPixelZ = val;
            else
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            continue;
        }

        if (key.contains("first pixel offset (mm)"))
        {
            double val = fields[1].toDouble(&ok);
            if (!ok)
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            if      (key.contains('1')) OffsetX = val;
            else if (key.contains('2')) OffsetY = val;
            else if (key.contains('3')) OffsetZ = val;
            else
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            continue;
        }
    }

    qDebug() << "---> Num bins:" << NumBinsX << NumBinsY << NumBinsZ;
    qDebug() << "---> mm/pixel:" << mmPerPixelX << mmPerPixelY << mmPerPixelZ;
    qDebug() << "---> Offset:" << OffsetX << OffsetY << OffsetZ;

    StartZeroBinX = OffsetX - 0.5 * NumBinsX * mmPerPixelX;
    StartZeroBinY = OffsetY - 0.5 * NumBinsY * mmPerPixelY;
    StartZeroBinZ = OffsetZ - 0.5 * NumBinsZ * mmPerPixelZ;

    Data.resize(NumBinsX);
    for (size_t ix = 0; ix < NumBinsX; ix++)
    {
        Data[ix].resize(NumBinsY);
        for (size_t iy = 0; iy < NumBinsY; iy++)
            Data[ix][iy].resize(NumBinsZ, 0);
    }

    QString binFileName = fileName;
    binFileName.replace(".hdr", ".img");

    std::ifstream inStream;
    inStream.open(binFileName.toLatin1().data(), std::ios::in | std::ios::binary);

    if (!inStream.is_open() || inStream.fail() || inStream.bad())
    {
        ErrorString = "Cannot open image file: " + binFileName;
        return false;
    }

    float buffer;
    for (size_t iz = 0; iz < NumBinsZ; iz++)
        for (size_t iy = 0; iy < NumBinsY; iy++)
            for (size_t ix = 0; ix < NumBinsX; ix++)
            {
                inStream.read((char*)&buffer, sizeof(float));
                Data[ix][iy][iz] = buffer;
            }

    calculateGlobalMaximum();
    qDebug() << "Global max in the defined filed of view fraction:" << GlobalMaximum;

    ui->ledMaximum->setText(QString::number(GlobalMaximum));

    // !!!*** error control

    return true;
}

#include "TStyle.h"
void AViewer3D::on_cobPalette_currentTextChanged(const QString & arg1)
{
    if (StartUp) return;

    for (const auto & p : Palettes)
        if (arg1 == p.first) gStyle->SetPalette(p.second);

    updateGui();
}


void AViewer3D::on_cobMaximum_activated(int index)
{
    switch (index)
    {
        case 0 : MaximumMode = IndividualMax; break;
        case 1 : MaximumMode = GlobalMax;     break;
        case 2 : MaximumMode = FixedMax;      break;
    }
}

void AViewer3D::on_ledMaximum_editingFinished()
{
    FixedMaximum = ui->ledMaximum->text().toDouble();
    updateGui();
}

void AViewer3D::on_ledScaling_editingFinished()
{
    ScalingFactor = 1 / ui->ledScaling->text().toDouble();
    updateGui();
}

void AViewer3D::on_sbMaxInFractionFoV_editingFinished()
{
    PercentFieldOfView = ui->sbMaxInFractionFoV->value();
    calculateGlobalMaximum();
    updateGui();
}

