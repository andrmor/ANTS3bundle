#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ajscriptmanager.h"
#include "adispatcherinterface.h"
#include "a3config.h"
#include "ageometryhub.h"
#include "guitools.h"
#include "afiletools.h"
#include "ademomanager.h"
#include "a3geoconwin.h"
#include "ageometrywindow.h"
#include "ageometryhub.h"
#include "a3matwin.h"
#include "a3photsimwin.h"
#include "ainterfacerulewin.h"
#include "graphwindowclass.h"
#include "aremotewindow.h"
#include "aparticlesimwin.h"

#include <QDebug>

#include "TObject.h"

MainWindow::MainWindow(AJScriptManager & SM) :
    Config(A3Config::getInstance()), ScriptManager(SM),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    connect(&Dispatcher, &ADispatcherInterface::updateProgress, this, &MainWindow::onProgressReceived);

    ADemoManager & DemoMan = ADemoManager::getInstance();
    connect(&SM, &AJScriptManager::finished, this, &MainWindow::onScriptEvaluationFinished);
    connect(&DemoMan, &ADemoManager::finished, this, &MainWindow::onDemoFinsihed);

    ui->pteData->appendPlainText(Config.lines);
    ui->leFrom->setText(Config.from);
    ui->leTo->setText(Config.to);

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
    connect(&A3Config::getInstance(), &A3Config::requestUpdatePhotSimGui, PartSimWin, &AParticleSimWin::updateGui);
    connect(PartSimWin, &AParticleSimWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);
}

MainWindow::~MainWindow()
{
    delete GeoConWin;
    delete ui;
}

void MainWindow::onRebuildGeometryRequested()
{
    AGeometryHub & geom = AGeometryHub::getInstance();
    geom.populateGeoManager();
    GeoConWin->updateGui();
    GeoWin->ShowGeometry();
}

void MainWindow::onProgressReceived(double progress)
{
    ui->prBar->setValue(progress*100.0);
}

void MainWindow::on_pbEvaluateScript_clicked()
{
    ui->prBar->setValue(0);
    Config.lines = ui->pteData->document()->toPlainText();

    QString script = ui->pteScript->document()->toPlainText();
    bool ok = ScriptManager.evaluate(script);
    if (!ok)
    {
        ui->pteScriptOut->clear();
        ui->pteScriptOut->appendPlainText("Script manager is busy!");
    }

    disableInterface(true);
}

#include <QJSValue>
void MainWindow::onScriptEvaluationFinished(bool bSuccess)
{
    disableInterface(false);

    ui->pteScriptOut->clear();
    if (!bSuccess)
    {
        int ln = ScriptManager.getErrorLineNumber();
        ui->pteScriptOut->appendPlainText("Error in line " + QString::number(ln));
    }
    ui->pteScriptOut->appendPlainText(ScriptManager.getResult().toString());
}

#include "a3global.h"
void MainWindow::onDemoFinsihed()
{
    disableInterface(false);

    ADemoManager & DemoMan = ADemoManager::getInstance();
    if (!DemoMan.ErrorString.isEmpty())
    {
        guitools::message1(DemoMan.ErrorString, "Error!", this);
        ui->prBar->setValue(0);
    }
    else
    {
        A3Global & GlobSet = A3Global::getInstance();
        QString res;
        ftools::loadTextFromFile(res, GlobSet.ExchangeDir + '/' + DemoMan.ResultsFileName);
        ui->pteData->clear();
        ui->pteData->appendPlainText(res);
        ui->prBar->setValue(100.0);
    }
    qApp->processEvents();
}

void MainWindow::on_pbSimulate_clicked()
{
    ui->prBar->setValue(0);
    disableInterface(true);
    qApp->processEvents();

    Config.lines = ui->pteData->document()->toPlainText();
    ADemoManager & DemoMan = ADemoManager::getInstance();
    DemoMan.run();
}

void MainWindow::on_leFrom_editingFinished()
{
    Config.from = ui->leFrom->text();
}

void MainWindow::on_leTo_editingFinished()
{
    Config.to = ui->leTo->text();
}

void MainWindow::disableInterface(bool flag)
{
    setDisabled(flag);
}

void MainWindow::on_pbAbort_clicked()
{
    ScriptManager.abort();
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

#include "ajsontools.h"
#include <QFileDialog>
void MainWindow::on_actionSave_configuration_triggered()
{
    QJsonObject json;
    A3Config::getInstance().writeToJson(json);

    QString fileName = QFileDialog::getSaveFileName(this, "Save configuration file");  // !!!***
    if (fileName.isEmpty()) return;

    jstools::saveJsonToFile(json, fileName);
}

void MainWindow::on_actionLoad_configuration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load configuration file");
    if (fileName.isEmpty()) return;

    QJsonObject json;
    jstools::loadJsonFromFile(json, fileName);

    A3Config::getInstance().readFromJson(json);

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

