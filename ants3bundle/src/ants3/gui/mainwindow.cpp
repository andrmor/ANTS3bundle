#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "a3global.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "guitools.h"
#include "afiletools.h"
#include "a3geoconwin.h"
#include "ageometrywindow.h"
#include "ageometryhub.h"
#include "a3matwin.h"
#include "asensorwindow.h"
#include "a3photsimwin.h"
#include "ainterfacerulewin.h"
#include "graphwindowclass.h"
#include "aremotewindow.h"
#include "aparticlesimwin.h"
#include "ajscripthub.h"
#include "ascriptwindow.h"
#include "ademowindow.h"

#include <QDebug>
#include <QTimer>

#include "TObject.h"

MainWindow::MainWindow() :
    AGuiWindow("Main", nullptr),
    Config(AConfig::getInstance()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

  // Start-up
    AGeometryHub::getInstance().populateGeoManager();

  // Main signal->slot lines
    connect(&Config, &AConfig::configLoaded, this, &MainWindow::updateAllGuiFromConfig);

  // Create and configure windows
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

    SensWin = new ASensorWindow(this);

    PhotSimWin = new A3PhotSimWin(this);
    connect(PhotSimWin, &A3PhotSimWin::requestShowGeometry,           GeoWin, &AGeometryWindow::ShowGeometry);
    connect(PhotSimWin, &A3PhotSimWin::requestShowTracks,             GeoWin, &AGeometryWindow::ShowTracks);
    connect(PhotSimWin, &A3PhotSimWin::requestClearGeoMarkers,        GeoWin, &AGeometryWindow::clearGeoMarkers);
    connect(PhotSimWin, &A3PhotSimWin::requestAddPhotonNodeGeoMarker, GeoWin, &AGeometryWindow::addPhotonNodeGeoMarker);
    connect(PhotSimWin, &A3PhotSimWin::requestShowGeoMarkers,         GeoWin, &AGeometryWindow::showGeoMarkers);

    GraphWin = new GraphWindowClass(this);
    connect(PhotSimWin, &A3PhotSimWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    FarmWin = new ARemoteWindow(this);

    PartSimWin = new AParticleSimWin(this);
    connect(PartSimWin, &AParticleSimWin::requestShowGeometry, GeoWin,   &AGeometryWindow::ShowGeometry);
    connect(PartSimWin, &AParticleSimWin::requestShowTracks,   GeoWin,   &AGeometryWindow::ShowTracks);
    connect(PartSimWin, &AParticleSimWin::requestShowPosition, GeoWin,   &AGeometryWindow::ShowPoint);
    connect(PartSimWin, &AParticleSimWin::requestCenterView,   GeoWin,   &AGeometryWindow::CenterView);
    connect(PartSimWin, &AParticleSimWin::requestDraw,         GraphWin, &GraphWindowClass::onDrawRequest);

    //qDebug() << ">JScript window";
    JScriptWin = new AScriptWindow(this);
    AJScriptHub * SH = &AJScriptHub::getInstance();
    JScriptWin->registerInterfaces();
    connect(SH, &AJScriptHub::clearOutput,      JScriptWin, &AScriptWindow::clearOutput);
    connect(SH, &AJScriptHub::outputText,       JScriptWin, &AScriptWindow::outputText);
    connect(SH, &AJScriptHub::outputHtml,       JScriptWin, &AScriptWindow::outputHtml);
    connect(SH, &AJScriptHub::showAbortMessage, JScriptWin, &AScriptWindow::outputAbortMessage);
    JScriptWin->updateGui();

    DemoWin = new ADemoWindow(this);

    loadWindowGeometries();

  // Start ROOT update cycle
    RootUpdateTimer = new QTimer(this);
    RootUpdateTimer->setInterval(100);
    QObject::connect(RootUpdateTimer, &QTimer::timeout, this, &MainWindow::rootTimerTimeout);
    RootUpdateTimer->start();
    qDebug()<<">Timer to refresh Root events started";

  // Config load explorer -> tips
    // TODO !!!***
    /*
    QString mss = ui->menuFile->styleSheet();
    mss += "; QMenu::tooltip {wakeDelay: 1;}";
    ui->menuFile->setStyleSheet(mss);
    ui->menuFile->setToolTipsVisible(true);
    ui->menuFile->setToolTipDuration(1000);
    void MainWindow::on_actionQuick_save_1_hovered()
    {ui->actionQuick_save_1->setToolTip(ELwindow->getQuickSlotMessage(1));}
    */

  // Finalizing
    updateGui();
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "TSystem.h"
void MainWindow::rootTimerTimeout()
{
    gSystem->ProcessEvents();
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

void MainWindow::on_actionSave_configuration_triggered()
{
    QString fileName = guitools::dialogSaveFile(this, "Save configuration file", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = Config.save(fileName);
    if (!err.isEmpty()) guitools::message(err, this);
}

void MainWindow::on_actionLoad_configuration_triggered()
{
    QString fileName = guitools::dialogLoadFile(this, "Load configuration file", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = Config.load(fileName);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }
    // gui is updated in updateAllGuiFromConfig() slot triggered from Config hub, since script also can load config
}

void MainWindow::on_actionLoad_last_config_triggered()
{
    const QString fileName = A3Global::getInstance().QuicksaveDir + "/QuickSave0.json";
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName);
}

void MainWindow::on_actionQuickSave_slot_1_triggered()
{
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave1.json");
}

void MainWindow::on_actionQuickSave_slot_2_triggered()
{
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave2.json");
}

void MainWindow::on_actionQuickSave_slot_3_triggered()
{
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave3.json");
}

void MainWindow::on_actionQuickLoad_slot_1_triggered()
{
    const QString fileName = A3Global::getInstance().QuicksaveDir + "/QuickSave1.json";
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName);
}

void MainWindow::on_actionQuickLoad_slot_2_triggered()
{
    const QString fileName = A3Global::getInstance().QuicksaveDir + "/QuickSave2.json";
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName);
}

void MainWindow::on_actionQuickLoad_slot_3_triggered()
{
    const QString fileName = A3Global::getInstance().QuicksaveDir + "/QuickSave3.json";
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

// ---

void MainWindow::updateAllGuiFromConfig()
{
    updateGui();

    GeoConWin->updateGui();
    MatWin->initWindow();

    PhotSimWin->updateGui();
    PartSimWin->updateGui();

    //rules !!!***

    GeoWin->ShowGeometry(false, false, true);
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
    Config.ConfigDescription = ui->pteConfigDescription->document()->toPlainText();
}

void MainWindow::on_pushButton_clicked()
{
    SensWin->showNormal();
    SensWin->activateWindow();
    SensWin->updateGui();
}

#include <QThread>
void MainWindow::closeEvent(QCloseEvent *)
{
    qDebug() << "\n<MainWindow shutdown initiated";
    clearFocus();

    qDebug()<<"<Saving position/status of all windows";
    saveWindowGeometries();

    qDebug() << "<Preparing graph window for shutdown";
    GraphWin->close();
    GraphWin->ClearDrawObjects_OnShutDown(); //to avoid any attempts to redraw deleted objects

    //saving ANTS master-configuration file
    JScriptWin->WriteToJson();
    A3Global::getInstance().saveConfig();

    qDebug()<<"<Saving ANTS configuration";
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave0.json");

    qDebug() << "<Stopping Root update timer-based cycle";
    RootUpdateTimer->stop();
    disconnect(RootUpdateTimer, &QTimer::timeout, this, &MainWindow::rootTimerTimeout);
    QThread::msleep(110);

    std::vector<AGuiWindow*> wins{ GeoConWin, GeoWin,   MatWin,  SensWin,    PhotSimWin,
                                   RuleWin,   GraphWin, FarmWin, PartSimWin, JScriptWin, DemoWin };

    for (auto * win : wins) delete win;

    qDebug() << "<MainWindow close event processing finished";
}

void MainWindow::saveWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this,    GeoConWin, GeoWin,  MatWin,     SensWin,    PhotSimWin,
                                   RuleWin, GraphWin,  FarmWin, PartSimWin, JScriptWin, DemoWin };

    for (auto * w : wins) w->storeGeomStatus();
}

void MainWindow::loadWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this,    GeoConWin, GeoWin,  MatWin,     SensWin,    PhotSimWin,
                                   RuleWin, GraphWin,  FarmWin, PartSimWin, JScriptWin, DemoWin };

    for (auto * w : wins) w->restoreGeomStatus();
}
