#include "aviewer3d.h"
#include "ui_aviewer3d.h"
#include "aviewer3dwidget.h"
#include "aviewer3dsettingsdialog.h"
#include "afiletools.h"
#include "guitools.h"

#include <QFileInfo>
#include <QComboBox>
#include <QJsonObject>
#include <QDebug>

#include <iostream>
#include <fstream>
#include <ostream>
#include <ios>

AViewer3D::AViewer3D(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::AViewer3D)
{
    ui->setupUi(this);
    ui->leTitle->setVisible(false);

    createViewWidgets();

    QAction * aSettings = ui->menubar->addAction("Settings");
    connect(aSettings, &QAction::triggered, this, &AViewer3D::showSettings);
}

void AViewer3D::showSettings()
{
    AViewer3DSettingsDialog D(Settings, this);
    int res = D.exec();
    if (res == QDialog::Rejected) return;

    if (D.isRecalcMaxRequired()) calculateGlobalMaximum();
    updateGui();
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

    ui->leTitle->setVisible(Settings.TitleVisible);
}

QString AViewer3D::getTitle()
{
    return ui->leTitle->text();
}

void AViewer3D::setTitle(const QString & title)
{
    ui->leTitle->setText(title);
}

void AViewer3D::calculateGlobalMaximum()
{
    const double removedFraction = 1.0 - Settings.PercentFieldOfView / 100.0;

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

    initViewers();
    return true;
}

void AViewer3D::initViewers()
{
    View1->init();
    View2->init();
    View3->init();

    updateGui();
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

#include "ajsontools.h"
void AViewer3D::writeDataToJson(QJsonObject & json) const
{
    json["NumBinsX"] = NumBinsX;
    json["NumBinsY"] = NumBinsY;
    json["NumBinsZ"] = NumBinsZ;

    json["mmPerPixelX"] = mmPerPixelX;
    json["mmPerPixelY"] = mmPerPixelY;
    json["mmPerPixelZ"] = mmPerPixelZ;

    json["OffsetX"] = OffsetX;
    json["OffsetY"] = OffsetY;
    json["OffsetZ"] = OffsetZ;

    json["StartZeroBinX"] = StartZeroBinX;
    json["StartZeroBinY"] = StartZeroBinY;
    json["StartZeroBinZ"] = StartZeroBinZ;

    //std::vector<std::vector<std::vector<double>>> Data[ix][iy][iz]
    QJsonArray ar;
    for (int ix = 0; ix < NumBinsX; ix++)
        for (int iy = 0; iy < NumBinsY; iy++)
            for (int iz = 0; iz < NumBinsZ; iz++)
                ar.push_back(Data[ix][iy][iz]);
    json["Data"] = ar;
}

void AViewer3D::readDataFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "NumBinsX", NumBinsX);
    jstools::parseJson(json, "NumBinsY", NumBinsY);
    jstools::parseJson(json, "NumBinsZ", NumBinsZ);

    jstools::parseJson(json, "mmPerPixelX", mmPerPixelX);
    jstools::parseJson(json, "mmPerPixelY", mmPerPixelY);
    jstools::parseJson(json, "mmPerPixelZ", mmPerPixelZ);

    jstools::parseJson(json, "OffsetX", OffsetX);
    jstools::parseJson(json, "OffsetY", OffsetY);
    jstools::parseJson(json, "OffsetZ", OffsetZ);

    jstools::parseJson(json, "StartZeroBinX", StartZeroBinX);
    jstools::parseJson(json, "StartZeroBinY", StartZeroBinY);
    jstools::parseJson(json, "StartZeroBinZ", StartZeroBinZ);

    Data.resize(NumBinsX);
    for (int ix = 0; ix < NumBinsX; ix++)
    {
        Data[ix].resize(NumBinsY);
        for (int iy = 0; iy < NumBinsX; iy++)
            Data[ix][iy].resize(NumBinsZ);
    }
    QJsonArray ar;
    jstools::parseJson(json, "Data", ar);
    size_t index = 0;
    //std::vector<std::vector<std::vector<double>>> Data[ix][iy][iz]
    for (int ix = 0; ix < NumBinsX; ix++)
        for (int iy = 0; iy < NumBinsY; iy++)
            for (int iz = 0; iz < NumBinsZ; iz++)
            {
                Data[ix][iy][iz] = ar[index].toDouble();
                index++;
            }
}

void AViewer3D::writeViewersToJson(QJsonObject & json) const
{
    QJsonObject js;
    View1->writeToJson(js); json["Viewer1"] = js;
    View2->writeToJson(js); json["Viewer2"] = js;
    View3->writeToJson(js); json["Viewer3"] = js;
}

void AViewer3D::readViewersFromJson(const QJsonObject & json)
{
    QJsonObject js;
    jstools::parseJson(json, "Viewer1", js); View1->readFromJson(js);
    jstools::parseJson(json, "Viewer2", js); View2->readFromJson(js);
    jstools::parseJson(json, "Viewer3", js); View3->readFromJson(js);
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
    for (int ix = 0; ix < NumBinsX; ix++)
    {
        Data[ix].resize(NumBinsY);
        for (int iy = 0; iy < NumBinsY; iy++)
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
    for (int iz = 0; iz < NumBinsZ; iz++)
        for (int iy = 0; iy < NumBinsY; iy++)
            for (int ix = 0; ix < NumBinsX; ix++)
            {
                inStream.read((char*)&buffer, sizeof(float));
                Data[ix][iy][iz] = buffer;
            }

    calculateGlobalMaximum();
    qDebug() << "Global max in the defined filed of view fraction:" << GlobalMaximum;

    ui->statusBar->showMessage("Max in defined FoV: " + QString::number(GlobalMaximum));

    // !!!*** error control

    return true;
}

void AViewer3D::on_actionShow_title_toggled(bool arg1)
{
    Settings.TitleVisible = arg1;
    ui->leTitle->setVisible(arg1);
}

void AViewer3D::on_actionMake_a_copy_triggered()
{
    emit requestMakeCopy(this);
}

