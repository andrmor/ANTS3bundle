#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "a3scriptmanager.h"
#include "adispatcherinterface.h"
#include "a3scriptres.h"
#include "a3config.h"
#include "guitools.h"
#include "afiletools.h"
#include "a3particlesimmanager.h"
#include "a3geoconwin.h"
#include "ageometrywindow.h"
#include "ageometryhub.h"
#include "a3matwin.h"
#include "a3photsimwin.h"
#include "ainterfacerulewin.h"
#include "graphwindowclass.h"

#include <QDebug>

#include "TObject.h"

MainWindow::MainWindow(A3ScriptManager & SM, A3ScriptRes & ScrRes) :
    Config(A3Config::getInstance()), ScriptManager(SM), ScrRes(ScrRes),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    connect(&Dispatcher, &ADispatcherInterface::updateProgress, this, &MainWindow::onProgressReceived);

    connect(&SM, &A3ScriptManager::finished, this, &MainWindow::onScriptEvaluationFinished);
    connect(ScrRes.ParticleSim, &A3ParticleSimManager::simFinished, this, &MainWindow::onParticleSimulationFinsihed);

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
}

MainWindow::~MainWindow()
{
    delete GeoConWin;
    delete ui;
}

#include "ageometryhub.h"
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
void MainWindow::onParticleSimulationFinsihed()
{
    disableInterface(false);

    if (!ScrRes.ParticleSim->ErrorString.isEmpty())
    {
        guitools::message1(ScrRes.ParticleSim->ErrorString, "Error!", this);
        ui->prBar->setValue(0);
    }
    else
    {
        A3Global & GlobSet = A3Global::getInstance();
        QString res;
        ftools::loadTextFromFile(res, GlobSet.ExchangeDir + '/' + ScrRes.ParticleSim->ResultsFileName);
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
    ScrRes.ParticleSim->simulate();
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

    QString fileName = QFileDialog::getSaveFileName(this, "Save configuration file");
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

