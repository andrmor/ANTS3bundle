#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "guitools.h"
#include "afiletools.h"
#include "a3geoconwin.h"
#include "ageometrywindow.h"
#include "ageometryhub.h"
#include "a3matwin.h"
#include "a3photsimwin.h"
#include "ainterfacerulewin.h"
#include "graphwindowclass.h"
#include "aremotewindow.h"
#include "aparticlesimwin.h"
#include "ajscripthub.h"
#include "ascriptwindow.h"
#include "ademowindow.h" // tmp

#include <QDebug>

#include "TObject.h"

MainWindow::MainWindow() :
    Config(AConfig::getInstance()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    AGeometryHub::getInstance().populateGeoManager();

    GeoConWin = new A3GeoConWin(this);
    connect(GeoConWin, &A3GeoConWin::requestRebuildGeometry, this,   &MainWindow::onRebuildGeometryRequested);

    GeoWin = new AGeometryWindow(this);
    connect(GeoConWin, &A3GeoConWin::requestShowGeometry, GeoWin, &AGeometryWindow::ShowGeometry);
    connect(GeoConWin, &A3GeoConWin::requestShowTracks,   GeoWin, &AGeometryWindow::ShowTracks);
    connect(GeoConWin, &A3GeoConWin::requestFocusVolume,  GeoWin, &AGeometryWindow::FocusVolume);
    GeoWin->show();
    GeoWin->resize(GeoWin->width()+1, GeoWin->height());
    GeoWin->resize(GeoWin->width()-1, GeoWin->height());
    GeoWin->ShowGeometry(false);
    GeoWin->hide();

    MatWin = new A3MatWin(this);
    MatWin->initWindow();

    RuleWin = new AInterfaceRuleWin(this);
    RuleWin->updateGui();

    PhotSimWin = new A3PhotSimWin(this);
    connect(PhotSimWin, &A3PhotSimWin::requestShowGeometry, GeoWin, &AGeometryWindow::ShowGeometry);
    connect(PhotSimWin, &A3PhotSimWin::requestShowTracks,   GeoWin, &AGeometryWindow::ShowTracks);

    GraphWin = new GraphWindowClass(this);
    connect(PhotSimWin, &A3PhotSimWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    FarmWin = new ARemoteWindow(this);

    PartSimWin = new AParticleSimWin(this);
    connect(PartSimWin, &AParticleSimWin::requestShowGeometry, GeoWin, &AGeometryWindow::ShowGeometry);
    connect(PartSimWin, &AParticleSimWin::requestShowTracks,   GeoWin, &AGeometryWindow::ShowTracks);
    connect(&AConfig::getInstance(), &AConfig::requestUpdatePhotSimGui, PartSimWin, &AParticleSimWin::updateGui);
    connect(PartSimWin, &AParticleSimWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    JScriptWin = new AScriptWindow(this);
    JScriptWin->registerInterfaces();
    AJScriptHub * SH = &AJScriptHub::getInstance();
    connect(SH, &AJScriptHub::clearOutput, JScriptWin, &AScriptWindow::clearOutput);
    connect(SH, &AJScriptHub::outputText, JScriptWin, &AScriptWindow::outputText);
    connect(SH, &AJScriptHub::outputHtml, JScriptWin, &AScriptWindow::outputHtml);

    DemoWin = new ADemoWindow(this);

    updateGui();
}

MainWindow::~MainWindow()
{
    delete GeoConWin;
    delete ui;
}

void MainWindow::updateGui()
{
    ui->leConfigName->setText(Config.ConfigName);

    ui->pteConfigDescription->blockSignals(true);
    ui->pteConfigDescription->clear();
    ui->pteConfigDescription->appendPlainText(Config.ConfigDescription);
    ui->pteConfigDescription->blockSignals(false);
}

void MainWindow::onRebuildGeometryRequested()
{
    AGeometryHub & geom = AGeometryHub::getInstance();
    geom.populateGeoManager();
    GeoConWin->updateGui();
    GeoWin->ShowGeometry();
}

void MainWindow::on_pbGeometry_clicked()
{
    GeoConWin->showNormal();
    GeoConWin->updateGui();
}

void MainWindow::on_pbGeoWin_clicked()
{
    GeoWin->showNormal();
    GeoWin->ShowGeometry();
}

void MainWindow::on_pbMaterials_clicked()
{
    MatWin->showNormal();
}

void MainWindow::on_pbLoadConfig_clicked()
{
    on_actionLoad_configuration_triggered();
}

void MainWindow::on_pbSaveConfig_clicked()
{
    on_actionSave_configuration_triggered();
}

#include "ajsontools.h"
#include <QFileDialog>
void MainWindow::on_actionSave_configuration_triggered()
{
    QJsonObject json;
    AConfig::getInstance().writeToJson(json);

    QString fileName = QFileDialog::getSaveFileName(this, "Save configuration file");  // !!!***
    if (fileName.isEmpty()) return;

    jstools::saveJsonToFile(json, fileName);
}

void MainWindow::on_actionLoad_configuration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load configuration file");
    if (fileName.isEmpty()) return;

    QString err = AConfig::getInstance().load(fileName);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    updateGui();

    GeoConWin->updateGui();
    MatWin->initWindow();

    PhotSimWin->updateGui();
    PartSimWin->updateGui();
}

void MainWindow::on_pbPhotSim_clicked()
{
    PhotSimWin->showNormal();
    PhotSimWin->activateWindow();
    PhotSimWin->updateGui();
}

void MainWindow::on_pbInterfaceRules_clicked()
{
    RuleWin->showNormal();
    RuleWin->activateWindow();
    RuleWin->updateGui();
}

void MainWindow::on_pbGraphWin_clicked()
{
    GraphWin->showNormal();
    GraphWin->activateWindow();
}

void MainWindow::on_pbFarm_clicked()
{
    FarmWin->showNormal();
    FarmWin->activateWindow();
    FarmWin->updateGui();
}

void MainWindow::on_pbParticleSim_clicked()
{
    PartSimWin->showNormal();
    PartSimWin->activateWindow();
    PartSimWin->updateGui();
}

void MainWindow::on_pbJavaScript_clicked()
{
    JScriptWin->showNormal();
    JScriptWin->activateWindow();
    JScriptWin->updateGui();
}

void MainWindow::on_pbDemo_clicked()
{
    DemoWin->showNormal();
    DemoWin->activateWindow();
}

void MainWindow::on_leConfigName_editingFinished()
{
    Config.ConfigName = ui->leConfigName->text();
}

void MainWindow::on_pteConfigDescription_textChanged()
{
    qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    Config.ConfigDescription = ui->pteConfigDescription->document()->toPlainText();
}

