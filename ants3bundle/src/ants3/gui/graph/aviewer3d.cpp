#include "aviewer3d.h"
#include "ui_aviewer3d.h"
#include "aviewer3dwidget.h"
#include "afiletools.h"
#include "guitools.h"

#include <QFileInfo>
#include <QComboBox>
#include <QDoubleValidator>

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

    StartUp = false;
}

void AViewer3D::createViewWidgets()
{
    View1 = new AViewer3DWidget(this, AViewer3DWidget::XY);
    View2 = new AViewer3DWidget(this, AViewer3DWidget::XZ);
    View3 = new AViewer3DWidget(this, AViewer3DWidget::YZ);

    ui->horizontalLayout->insertWidget(0, View1);
    ui->horizontalLayout->insertWidget(1, View2);
    ui->horizontalLayout->insertWidget(2, View3);
}

void AViewer3D::updateGui()
{
    View1->redraw();
    View2->redraw();
    View3->redraw();
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

double AViewer3D::binToEdgePosition(size_t iDimension, size_t iBin) const
{
    return StartZeroBin[iDimension] + iBin * Scaling_mmPerPixel[iDimension];
}

double AViewer3D::binToCenterPosition(size_t iDimension, size_t iBin) const
{
    return StartZeroBin[iDimension] + (0.5 + iBin) * Scaling_mmPerPixel[iDimension];
}

bool AViewer3D::extractDoubleFromPair(const QStringList & twoFields, const QString & identifierTxt, std::array<double, 3> & array)
{
    const QString key = twoFields.front();
    if (!key.contains(identifierTxt)) return true;

    bool ok;
    double val = twoFields[1].toDouble(&ok);
    if (!ok)
    {
        ErrorString = "Format error in the image header";
        return false;
    }

    int index = 0;
    if      (key.contains('1')) index = 1;
    else if (key.contains('2')) index = 2;
    else if (key.contains('3')) index = 3;
    if (index == 0)
    {
        ErrorString = "Format error in the image header";
        return false;
    }

    array[index-1] = val;
    return true;
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
            int index = 0;
            if      (key.contains('1')) index = 1;
            else if (key.contains('2')) index = 2;
            else if (key.contains('3')) index = 3;
            if (index == 0)
            {
                ErrorString = "Format error in the image header";
                return false;
            }
            NumBins[index-1] = num;
            continue;
        }

        ok = extractDoubleFromPair(fields, "scaling factor (mm/pixel)", Scaling_mmPerPixel);
        if (!ok) return false;

        ok = extractDoubleFromPair(fields, "first pixel offset (mm)", Offset);
        if (!ok) return false;
    }

    qDebug() << "---> Num bins:" << NumBins[0] << NumBins[1] << NumBins[2];
    qDebug() << "---> mm/pixel:" << Scaling_mmPerPixel[0] << Scaling_mmPerPixel[1] << Scaling_mmPerPixel[2];
    qDebug() << "---> Offset:" << Offset[0] << Offset[1] << Offset[2];

    for (size_t i = 0; i < 3; i++)
        StartZeroBin[i] = Offset[i] - 0.5 * NumBins[i] * Scaling_mmPerPixel[i];

    Data.resize(NumBins[2]);
    for (size_t iz = 0; iz < NumBins[2]; iz++)
    {
        Data[iz].resize(NumBins[1]);
        for (size_t iy = 0; iy < NumBins[1]; iy++)
            Data[iz][iy].resize(NumBins[2], 0);
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

    GlobalMaximum = 0;
    float buffer;
    for (size_t iz = 0; iz < NumBins[2]; iz++)
    {
        for (size_t iy = 0; iy < NumBins[1]; iy++)
        {
            for (size_t ix = 0; ix < NumBins[0]; ix++)
            {
                inStream.read((char*)&buffer, sizeof(float));
                Data[iz][iy][ix] = buffer;
                if (buffer > GlobalMaximum) GlobalMaximum = buffer;
            }
        }
    }
    ui->ledMaximum->setText(QString::number(GlobalMaximum));
    //ui->ledScaling->setText(QString::number(1/GlobalMaximum));

    // error control

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

    ui->ledMaximum->setVisible(index == 2);
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

