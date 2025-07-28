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

#include "TObject.h"

AViewer3D::AViewer3D(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::AViewer3D)
{
    ui->setupUi(this);
    ui->leTitle->setVisible(false);

    createViewWidgets();

    QAction * aSettings = ui->menubar->addAction("Settings");
    connect(aSettings, &QAction::triggered, this, &AViewer3D::showSettings);

    restoreGeomStatus();
}

void AViewer3D::showSettings()
{
    AViewer3DSettingsDialog D(Settings, this);
    D.move(this->x()+25, this->y()+25);

    auto applySettings = [this](bool needRecalc)
    {
        if (needRecalc) calculateGlobalMaximum();
        updateGui();
    };
    connect(&D, &AViewer3DSettingsDialog::requestUpdate, this, applySettings);

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

    connect(from, &AViewer3DWidget::cursorPositionChanged, this, &AViewer3D::onCursorPositionChangedOnRasterWindow);
    connect(from, &AViewer3DWidget::cursorLeftVisibleArea, this, &AViewer3D::onCursorLeftRasterWindow);

    connect(from, &AViewer3DWidget::requestExportToBasket, this, &AViewer3D::requestExportToBasket);
}

void AViewer3D::onCursorPositionChangedOnRasterWindow(double x, double y, double z, double val)
{
    int  fieldWidth = 7;
    char format = 'f';
    int  precision = 2;
    ui->statusBar->showMessage(QString("X: %0  Y: %1  Z: %2  Value: %3")
                                   .arg(x, fieldWidth, format, precision, ' ')
                                   .arg(y, fieldWidth, format, precision, ' ')
                                   .arg(z, fieldWidth, format, precision, ' ')
                                   .arg(val, 0, 'g', 4) );
}

void AViewer3D::onCursorLeftRasterWindow()
{
    ui->statusBar->clearMessage();
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
    storeGeomStatus();
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

int AViewer3D::positionToBin(EAxis axis, double pos) const
{
    switch (axis)
    {
    case Xaxis: return (pos - StartZeroBinX) / mmPerPixelX;
    case Yaxis: return (pos - StartZeroBinY) / mmPerPixelY;
    case Zaxis: return (pos - StartZeroBinZ) / mmPerPixelZ;
    }
    return 0;
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

    json["GlobalMaximum"] = GlobalMaximum;
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

    jstools::parseJson(json, "GlobalMaximum", GlobalMaximum);
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
    if (Settings.FixedMaximum == 0) Settings.FixedMaximum = GlobalMaximum;
    if (!Settings.ApplyScaling) Settings.ScalingFactor = GlobalMaximum;
    //qDebug() << "Global max in the defined filed of view fraction:" << GlobalMaximum;

    return true;
}

#include <QDialog>
#include <QLabel>
void AViewer3D::on_actionMake_a_copy_triggered()
{
    QDialog d(this);
    d.setWindowTitle("");
    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowStaysOnTopHint);
    QHBoxLayout * l = new QHBoxLayout(&d);
    l->addWidget(new QLabel("Creating a copy..."));

    d.show();
    qApp->processEvents();

    emit requestMakeCopy(this);
}

#include <QFileInfo>
void AViewer3D::on_actionSave_as_png_images_triggered()
{
    QString fn = guitools::dialogSaveFile(this, "Select file name template for png image files.", "All files (*.*)");
    if (fn.isEmpty()) return;

    QFileInfo fi(fn);
    View1->saveImage(fi.path() + "/" + fi.baseName() + "_XY.png");
    View2->saveImage(fi.path() + "/" + fi.baseName() + "_XZ.png");
    View3->saveImage(fi.path() + "/" + fi.baseName() + "_YZ.png");
}

void AViewer3D::on_actionSave_as_TH2D_histograms_triggered()
{
    QString fn = guitools::dialogSaveFile(this, "Select file name template for png image files.", "All files (*.*)");
    if (fn.isEmpty()) return;

    QFileInfo fi(fn);
    QString suffix = fi.suffix();
    if (suffix.isEmpty()) suffix = "root";
    else if (suffix != "c" && suffix != "root") suffix = "root";

    View1->saveRoot(fi.path() + "/" + fi.baseName() + "_XY." + suffix);
    View2->saveRoot(fi.path() + "/" + fi.baseName() + "_XZ." + suffix);
    View3->saveRoot(fi.path() + "/" + fi.baseName() + "_YZ." + suffix);
}

void AViewer3D::on_actionExport_to_basket_of_graph_window_triggered()
{
    View1->exportToBasket("XY");
    View2->exportToBasket("XZ");
    View3->exportToBasket("YZ");
}

#include "TH1D.h"
void AViewer3D::on_actionExport_projections_to_basket_triggered()
{
    TH1D hX("XProj", "XProj", NumBinsX, StartZeroBinX, StartZeroBinX + NumBinsX*mmPerPixelX);
    TH1D hY("YProj", "YProj", NumBinsY, StartZeroBinY, StartZeroBinY + NumBinsY*mmPerPixelY);
    TH1D hZ("ZProj", "ZProj", NumBinsZ, StartZeroBinZ, StartZeroBinZ + NumBinsZ*mmPerPixelZ);

    for (int ix = 0; ix < NumBinsX; ix++)
    {
        double x = StartZeroBinX + ix * mmPerPixelX;
        for (int iy = 0; iy < NumBinsY; iy++)
        {
            double y = StartZeroBinY + iy * mmPerPixelY;
            for (int iz = 0; iz < NumBinsZ; iz++)
            {
                double z = StartZeroBinZ + iz * mmPerPixelZ;
                hX.Fill(x, Data[ix][iy][iz]);
                hY.Fill(y, Data[ix][iy][iz]);
                hZ.Fill(z, Data[ix][iy][iz]);
            }
        }
    }

    emit requestExportToBasket(&hX, "hist", "X_proj");
    emit requestExportToBasket(&hY, "hist", "Y_proj");
    emit requestExportToBasket(&hZ, "hist", "Z_proj");
}

#include <QSettings>
void AViewer3D::storeGeomStatus()
{
    QSettings settings;
    settings.beginGroup("Viewer3D");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("visible", isVisible());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void AViewer3D::restoreGeomStatus()
{
    QSettings settings;
    settings.beginGroup("Viewer3D");
    restoreGeometry(settings.value("geometry").toByteArray());
    bool bVisible = settings.value("visible", false).toBool();
    bool bmax = settings.value("maximized", false).toBool();
    if (bVisible)
    {
        if (bmax) showMaximized();
        else      showNormal();
    }
    settings.endGroup();
}

#include "a3global.h"
void AViewer3D::on_actionMake_template_triggered()
{
    QJsonObject json;
    Settings.writeToJson(json);
    writeViewersToJson(json);
    QString fn = A3Global::getConstInstance().ConfigDir + "/settingsViewer3D.json";
    jstools::saveJsonToFile(json, fn);
}

void AViewer3D::on_actionApply_template_triggered()
{
    QString fn = A3Global::getConstInstance().ConfigDir + "/settingsViewer3D.json";
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fn);
    if (!ok)
    {
        guitools::message("Cannot read template file, probably it was not created yet", this);
        return;
    }

    Settings.readFromJson(json);
    readViewersFromJson(json);
    updateGui();
}
