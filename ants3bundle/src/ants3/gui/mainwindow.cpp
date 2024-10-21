#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "a3global.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "guitools.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "ageotreewin.h"
#include "ageometrywindow.h"
#include "ageometryhub.h"
#include "amatwin.h"
#include "asensorwindow.h"
#include "aphotontunnelwindow.h"
#include "aphotsimwin.h"
#include "ainterfacerulewin.h"
#include "graphwindowclass.h"
#include "aremotewindow.h"
#include "aparticlesimwin.h"
#include "ascripthub.h"
#include "ascriptwindow.h"
#include "aglobsetwindow.h"
#include "ademowindow.h"
#include "escriptlanguage.h"
#include "ageowin_si.h"
#include "aguifromscrwin.h"

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QString>

#include "TObject.h"

MainWindow::MainWindow() :
    AGuiWindow("Main", nullptr),
    Config(AConfig::getInstance()),
    GlobSet(A3Global::getInstance()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

  // Start-up
    AGeometryHub::getInstance().populateGeoManager();

  // Main signal->slot lines
    connect(&Config, &AConfig::configLoaded,           this, &MainWindow::updateAllGuiFromConfig);
    connect(&Config, &AConfig::requestSaveGuiSettings, this, &MainWindow::onRequestSaveGuiSettings);

  // Create and configure windows
    GeoTreeWin = new AGeoTreeWin(this);
    connect(GeoTreeWin, &AGeoTreeWin::requestRebuildGeometry, this,   &MainWindow::onRebuildGeometryRequested);

    GeoWin = new AGeometryWindow(false, this);
    AScriptHub::getInstance().addCommonInterface(new AGeoWin_SI(GeoWin), "geowin");
    // WARNING! signal / slots for GeoWin have to be connected in the connectSignalSlotsForGeoWin() method below
    // the reason is that the window has to be re-created if viewer is changed to JSROOT

    GraphWin = new GraphWindowClass(this);
    //GraphWindowClass::connectScriptUnitDrawRequests is used to connect draw requests
    connect(GeoTreeWin, &AGeoTreeWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    MatWin = new AMatWin(this);
    MatWin->initWindow();
    connect(MatWin, &AMatWin::requestRebuildDetector, this,     &MainWindow::onRebuildGeometryRequested);
    connect(MatWin, &AMatWin::requestDraw,            GraphWin, &GraphWindowClass::onDrawRequest);

    RuleWin = new AInterfaceRuleWin(this);
    connect(RuleWin, &AInterfaceRuleWin::requestDraw,       GraphWin, &GraphWindowClass::onDrawRequest);
    connect(RuleWin, &AInterfaceRuleWin::requestDrawLegend, GraphWin, &GraphWindowClass::drawLegend);

    SensWin = new ASensorWindow(this);
    connect(SensWin, &ASensorWindow::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    PhotFunWin = new APhotonTunnelWindow(this);
    connect(PhotFunWin, &APhotonTunnelWindow::requestDraw,  GraphWin, &GraphWindowClass::onDrawRequest);

    PhotSimWin = new APhotSimWin(this);
    connect(PhotSimWin, &APhotSimWin::requestDraw, GraphWin, &GraphWindowClass::onDrawRequest);

    FarmWin = new ARemoteWindow(this);

    PartSimWin = new AParticleSimWin(nullptr); // Qt::WindowModality for the source dialog requires another parent for the window!
    connect(PartSimWin, &AParticleSimWin::requestDraw,                  GraphWin,   &GraphWindowClass::onDrawRequest);
    connect(PartSimWin, &AParticleSimWin::requestAddToBasket,           GraphWin,   &GraphWindowClass::addCurrentToBasket);
    connect(PartSimWin, &AParticleSimWin::requestShowGeoObjectDelegate, GeoTreeWin, &AGeoTreeWin::UpdateGeoTree);

    AScriptHub * ScriptHub = &AScriptHub::getInstance();
    GuiFromScrWin = new AGuiFromScrWin(this);
    ScriptHub->addGuiScriptUnit(GuiFromScrWin);
    qDebug() << "Creating JScript window";
    JScriptWin = new AScriptWindow(EScriptLanguage::JavaScript, this);
    JScriptWin->registerInterfaces();
    connect(ScriptHub,  &AScriptHub::clearOutput_JS,      JScriptWin, &AScriptWindow::clearOutput, Qt::QueuedConnection);
    connect(ScriptHub,  &AScriptHub::outputText_JS,       JScriptWin, &AScriptWindow::outputText, Qt::QueuedConnection);
    connect(ScriptHub,  &AScriptHub::outputHtml_JS,       JScriptWin, &AScriptWindow::outputHtml, Qt::QueuedConnection);
    connect(ScriptHub,  &AScriptHub::outputFromBuffer_JS, JScriptWin, &AScriptWindow::outputFromBuffer, Qt::QueuedConnection);
    connect(ScriptHub,  &AScriptHub::reportProgress_JS,   JScriptWin, &AScriptWindow::onProgressChanged, Qt::QueuedConnection);
    connect(ScriptHub,  &AScriptHub::showAbortMessage_JS, JScriptWin, &AScriptWindow::outputAbortMessage);
    connect(JScriptWin, &AScriptWindow::requestUpdateGui, this,       &MainWindow::updateAllGuiFromConfig);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddJavaScript,   JScriptWin, &AScriptWindow::onRequestAddScript);
    JScriptWin->updateGui();

#ifdef ANTS3_PYTHON
    qDebug() << "Creating Python window";
    PythonWin = new AScriptWindow(EScriptLanguage::Python, this);
    PythonWin->registerInterfaces();
    connect(ScriptHub,  &AScriptHub::clearOutput_P,       PythonWin, &AScriptWindow::clearOutput);
    connect(ScriptHub,  &AScriptHub::outputText_P,        PythonWin, &AScriptWindow::outputText);
    connect(ScriptHub,  &AScriptHub::outputHtml_P,        PythonWin, &AScriptWindow::outputHtml);
    connect(ScriptHub,  &AScriptHub::outputFromBuffer_P,  PythonWin, &AScriptWindow::outputFromBuffer);
    connect(ScriptHub,  &AScriptHub::reportProgress_P,    PythonWin, &AScriptWindow::onProgressChanged);
    connect(ScriptHub,  &AScriptHub::showAbortMessage_P,  PythonWin, &AScriptWindow::outputAbortMessage);
    connect(PythonWin,  &AScriptWindow::requestUpdateGui, this,      &MainWindow::updateAllGuiFromConfig);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddPythonScript,   PythonWin, &AScriptWindow::onRequestAddScript);
    PythonWin->updateGui();
#endif

    GlobSetWin = new AGlobSetWindow(this);
    connect(PhotSimWin, &APhotSimWin::requestConfigureExchangeDir,    GlobSetWin, &AGlobSetWindow::onRequestConfigureExchangeDir);
    connect(PartSimWin, &AParticleSimWin::requestConfigureExchangeDir, GlobSetWin, &AGlobSetWindow::onRequestConfigureExchangeDir);

    DemoWin = new ADemoWindow(this);

    connect(&AScriptHub::getInstance(), &AScriptHub::requestUpdateGui, this, &MainWindow::updateAllGuiFromConfig);

    // called where all windows connecting to GeoWin are already defined
    connectSignalSlotsForGeoWin();

    loadWindowGeometries();

    bool bShown = GeoWin->isVisible();
    GeoWin->show();
    GeoWin->resize(GeoWin->width()+1, GeoWin->height());
    GeoWin->resize(GeoWin->width()-1, GeoWin->height());
    GeoWin->ShowGeometry(false);
    //if (!bShown) GeoWin->hide(); // has to be in the end!

  // Start ROOT update cycle
    RootUpdateTimer = new QTimer(this);
    RootUpdateTimer->setInterval(100);
    QObject::connect(RootUpdateTimer, &QTimer::timeout, this, &MainWindow::rootTimerTimeout);
    RootUpdateTimer->start();
    qDebug()<<">Timer to refresh Root events started";

    QString mss = ui->menuFile->styleSheet();
    mss += "; QMenu::tooltip {wakeDelay: 1;}";
    ui->menuFile->setStyleSheet(mss);
    ui->menuFile->setToolTipsVisible(true);
    ui->menuFile->setToolTipDuration(1000);

    std::vector<QLabel*> labels = {ui->lSpaceHolder1, ui->lSpaceHolder2};
    for (QLabel * l : labels)
    {
        l->setMinimumHeight(ui->pbFunctionalModels->height());
        l->setText("");
    }

  // Finalizing
    updateAllGuiFromConfig(); //updateGui();
    ScriptHub->finalizeInit();

    if (!bShown) GeoWin->hide(); // has to be last, if before updateAllGuiFromConfig() and window is hidden --> dark on open

    int size = ui->pbFunctionalModels->size().height();
    ui->lSpaceHolder1->setMinimumHeight(size);
    ui->lSpaceHolder2->setMinimumHeight(size);
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
    GeoTreeWin->updateGui();
    MatWin->updateGui();
    RuleWin->updateGui();
    PartSimWin->onMaterialsChanged();
    emit GeoTreeWin->requestClearGeoMarkers(0);
    if (GeoWin->isVisible()) GeoWin->ShowGeometry();
}

void MainWindow::on_pbGeometry_clicked()
{
    GeoTreeWin->onMainWinButtonClicked(true);
    GeoTreeWin->updateGui();
}
void MainWindow::on_pbGeometry_customContextMenuRequested(const QPoint &)
{
    GeoTreeWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbGeoWin_clicked()
{
    GeoWin->onMainWinButtonClicked(true);
    GeoWin->ShowGeometry();
}
void MainWindow::on_pbGeoWin_customContextMenuRequested(const QPoint &)
{
    GeoWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbMaterials_clicked()
{
    MatWin->onMainWinButtonClicked(true);
    //MatWin->update(); // why no update?
}
void MainWindow::on_pbMaterials_customContextMenuRequested(const QPoint &)
{
    MatWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbPhotSim_clicked()
{
    PhotSimWin->onMainWinButtonClicked(true);
    PhotSimWin->updateGui();
}
void MainWindow::on_pbPhotSim_customContextMenuRequested(const QPoint &)
{
    PhotSimWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbInterfaceRules_clicked()
{
    RuleWin->onMainWinButtonClicked(true);
    RuleWin->updateGui();
}
void MainWindow::on_pbInterfaceRules_customContextMenuRequested(const QPoint &)
{
    RuleWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbGraphWin_clicked()
{
    GraphWin->onMainWinButtonClicked(true);
}
void MainWindow::on_pbGraphWin_customContextMenuRequested(const QPoint &)
{
    GraphWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbFarm_clicked()
{
    FarmWin->onMainWinButtonClicked(true);
    FarmWin->updateGui();
}
void MainWindow::on_pbFarm_customContextMenuRequested(const QPoint &)
{
    FarmWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbGlobSet_clicked()
{
    GlobSetWin->onMainWinButtonClicked(true);
    GlobSetWin->updateGui();
}
void MainWindow::on_pbGlobSet_customContextMenuRequested(const QPoint &)
{
    GlobSetWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbParticleSim_clicked()
{
    PartSimWin->onMainWinButtonClicked(true);
    PartSimWin->updateGui();
}
void MainWindow::on_pbParticleSim_customContextMenuRequested(const QPoint &)
{
    PartSimWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbJavaScript_clicked()
{
    JScriptWin->onMainWinButtonClicked(true);
    JScriptWin->updateGui();
}
void MainWindow::on_pbJavaScript_customContextMenuRequested(const QPoint &)
{
    JScriptWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbPython_clicked()
{
#ifdef ANTS3_PYTHON
    PythonWin->onMainWinButtonClicked(true);
    PythonWin->updateGui();
#else
    guitools::message("Ants3 was compiled without Python support.\nIt can be enabled in ants3.pro by uncommenting:\n#CONFIG += ants3_Python", this);
#endif
}
void MainWindow::on_pbPython_customContextMenuRequested(const QPoint &)
{
#ifdef ANTS3_PYTHON
    PythonWin->onMainWinButtonClicked(false);
#endif
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
    if (!fileName.endsWith(".json")) fileName += ".json";

    QString err = Config.save(fileName);
    if (!err.isEmpty()) guitools::message(err, this);
}

void MainWindow::on_actionLoad_configuration_triggered()
{
    QString fileName = guitools::dialogLoadFile(this, "Load configuration file", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = Config.load(fileName, true);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }
    // gui is updated in updateAllGuiFromConfig() slot triggered from Config hub, since script also can load config
}

void MainWindow::on_actionLoad_last_config_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(0);
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName, true);
}

void MainWindow::on_actionQuickSave_slot_1_triggered()
{
    Config.save(GlobSet.getQuickFileName(1));
}

void MainWindow::on_actionQuickSave_slot_2_triggered()
{
    Config.save(GlobSet.getQuickFileName(2));
}

void MainWindow::on_actionQuickSave_slot_3_triggered()
{
    Config.save(GlobSet.getQuickFileName(1));
}

void MainWindow::on_actionQuickLoad_slot_1_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(1);
    if (!QFile::exists(fileName)) return;

    Config.load(fileName, true);
}

void MainWindow::on_actionQuickLoad_slot_2_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(2);
    if (!QFile::exists(fileName)) return;

    Config.load(fileName, true);
}

void MainWindow::on_actionQuickLoad_slot_3_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(3);
    if (!QFile::exists(fileName)) return;

    Config.load(fileName, true);
}

void MainWindow::on_actionClose_ants3_triggered()
{
    close();
}

// ---

void MainWindow::updateAllGuiFromConfig()
{
    updateGui();

    GeoTreeWin->updateGui();
    //MatWin->initWindow();
    MatWin->updateGui();
    SensWin->updateGui();
    RuleWin->updateGui();
    PhotFunWin->updateGui();

    PhotSimWin->updateGui();
    PartSimWin->updateGui();

    /*
    JScriptWin->updateGui();
#ifdef ANTS3_PYTHON
    PythonWin->updateGui();
#endif
    */

    QJsonObject json = AConfig::getInstance().JSON["gui"].toObject();
    // Geometry
    {
        QJsonObject js;
        bool ok = jstools::parseJson(json, "GeometryWindow", js);
        if (ok) GeoWin->readFromJson(js);
    }
    // Particle sim
    {
        QJsonObject js;
        bool ok = jstools::parseJson(json, "ParticleSimWindow", js);
        if (ok) PartSimWin->readFromJson(js);
    }
    // Photon sim
    {
        QJsonObject js;
        bool ok = jstools::parseJson(json, "PhotonSimWindow", js);
        if (ok) PhotSimWin->readFromJson(js);
    }

    GeoWin->fRecallWindow = false;
    GeoWin->ShowGeometry(false, false, true);
    GeoWin->ShowTracks();
}

void MainWindow::onRequestSaveGuiSettings()
{
    QJsonObject & JSON = AConfig::getInstance().JSON;

    QJsonObject json;

    // Geometry
    {
        QJsonObject js;
            GeoWin->writeToJson(js);
        json["GeometryWindow"] = js;
    }
    // Particle sim
    {
        QJsonObject js;
            PartSimWin->writeToJson(js);
        json["ParticleSimWindow"] = js;
    }
    // Photon sim
    {
        QJsonObject js;
            PhotSimWin->writeToJson(js);
        json["PhotonSimWindow"] = js;
    }

    JSON["gui"] = json;
}

void MainWindow::onRequestChangeGeoViewer(bool useJSRoot)
{
    QTimer::singleShot(0, this,
                       [useJSRoot, this]()
                       {
                            changeGeoViewer(useJSRoot);
                       } );
}

void MainWindow::changeGeoViewer(bool useJSRoot)
{
    delete GeoWin; GeoWin = new AGeometryWindow(useJSRoot, this);
    GeoWin->restoreGeomStatus();

    connectSignalSlotsForGeoWin();

    GeoWin->show();
    if (!useJSRoot)
    {
        GeoWin->resize(GeoWin->width()+1, GeoWin->height());
        GeoWin->resize(GeoWin->width()-1, GeoWin->height());
    }
    GeoWin->ShowGeometry(true);

    AScriptHub::getInstance().updateGeoWin(GeoWin);
}

void MainWindow::connectSignalSlotsForGeoWin()
{
    connect(GeoWin,     &AGeometryWindow::requestChangeGeoViewer,       this,   &MainWindow::onRequestChangeGeoViewer);

    connect(GeoTreeWin, &AGeoTreeWin::requestShowGeometry,              GeoWin, &AGeometryWindow::ShowGeometry);
    connect(GeoTreeWin, &AGeoTreeWin::requestShowRecursive,             GeoWin, &AGeometryWindow::showRecursive);
    connect(GeoTreeWin, &AGeoTreeWin::requestShowTracks,                GeoWin, &AGeometryWindow::ShowTracks);
    connect(GeoTreeWin, &AGeoTreeWin::requestFocusVolume,               GeoWin, &AGeometryWindow::FocusVolume);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddGeoMarkers,             GeoWin, &AGeometryWindow::addGeoMarkers);
    connect(GeoTreeWin, &AGeoTreeWin::requestClearGeoMarkers,           GeoWin, &AGeometryWindow::clearGeoMarkers);

    connect(MatWin,     &AMatWin::requestShowGeometry,                  GeoWin, &AGeometryWindow::ShowGeometry);

    connect(RuleWin,    &AInterfaceRuleWin::requestClearGeometryViewer, GeoWin, &AGeometryWindow::ClearRootCanvas);
    connect(RuleWin,    &AInterfaceRuleWin::requestShowTracks,          GeoWin, &AGeometryWindow::ShowTracks);

    connect(SensWin,    &ASensorWindow::requestShowSensorModels,        GeoWin, &AGeometryWindow::showSensorModelIndexes);

    connect(PhotSimWin, &APhotSimWin::requestShowGeometry,              GeoWin, &AGeometryWindow::ShowGeometry);
    connect(PhotSimWin, &APhotSimWin::requestShowTracks,                GeoWin, &AGeometryWindow::ShowTracks);
    connect(PhotSimWin, &APhotSimWin::requestClearGeoMarkers,           GeoWin, &AGeometryWindow::clearGeoMarkers);
    connect(PhotSimWin, &APhotSimWin::requestAddPhotonNodeGeoMarker,    GeoWin, &AGeometryWindow::addPhotonNodeGeoMarker);
    connect(PhotSimWin, &APhotSimWin::requestShowGeoMarkers,            GeoWin, &AGeometryWindow::showGeoMarkers);
    connect(PhotSimWin, &APhotSimWin::requestShowPosition,              GeoWin, &AGeometryWindow::ShowPoint);

    connect(PartSimWin, &AParticleSimWin::requestShowGeometry,          GeoWin, &AGeometryWindow::ShowGeometry);
    connect(PartSimWin, &AParticleSimWin::requestShowTracks,            GeoWin, &AGeometryWindow::ShowTracks);
    connect(PartSimWin, &AParticleSimWin::requestShowPosition,          GeoWin, &AGeometryWindow::ShowPoint);
    connect(PartSimWin, &AParticleSimWin::requestAddMarker,             GeoWin, &AGeometryWindow::addGenerationMarker);
    connect(PartSimWin, &AParticleSimWin::requestClearMarkers,          GeoWin, &AGeometryWindow::clearGeoMarkers);
    connect(PartSimWin, &AParticleSimWin::requestCenterView,            GeoWin, &AGeometryWindow::CenterView);

    connect(PhotFunWin, &APhotonTunnelWindow::requestShowConnection,     GeoWin, &AGeometryWindow::onRequestShowConnection);
    connect(PhotFunWin, &APhotonTunnelWindow::requestShowAllConnections, GeoWin, &AGeometryWindow::onRequestShowAllConnections);
}

void MainWindow::on_leConfigName_editingFinished()
{
    Config.ConfigName = ui->leConfigName->text();
}

void MainWindow::on_pteConfigDescription_textChanged()
{
    Config.ConfigDescription = ui->pteConfigDescription->document()->toPlainText();
}

void MainWindow::on_pbSensors_clicked()
{
    SensWin->onMainWinButtonClicked(true);
    SensWin->updateGui();
}
void MainWindow::on_pbSensors_customContextMenuRequested(const QPoint &)
{
    SensWin->onMainWinButtonClicked(false);
}

void MainWindow::on_pbFunctionalModels_clicked()
{
    PhotFunWin->onMainWinButtonClicked(true);
    PhotFunWin->updateGui();
}

void MainWindow::on_pbFunctionalModels_customContextMenuRequested(const QPoint &)
{
    PhotFunWin->onMainWinButtonClicked(false);
}

#include <QThread>
void MainWindow::closeEvent(QCloseEvent *)
{
    qDebug() << "\n<MainWindow shutdown initiated";
    clearFocus();

    qDebug() << "<Saving position/status of all windows";
    saveWindowGeometries();

    qDebug() << "<Preparing graph window for shutdown";
    GraphWin->close3DviewWindow();
    GraphWin->close();
    GraphWin->ClearDrawObjects_OnShutDown(); //to avoid any attempts to redraw deleted objects

    qDebug() << "<Saving JavaScript scipts";
    JScriptWin->WriteToJson();
#ifdef ANTS3_PYTHON
    qDebug() << "<Saving Python scripts";
    PythonWin->WriteToJson();
#endif

    qDebug() << "<Saving global settings";
    A3Global::getInstance().saveConfig();

    qDebug() << "<Saving current configuration";
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave0.json");

    qDebug() << "<Stopping Root update timer-based cycle";
    RootUpdateTimer->stop();
    disconnect(RootUpdateTimer, &QTimer::timeout, this, &MainWindow::rootTimerTimeout);
    QThread::msleep(110);

    std::vector<AGuiWindow*> wins{ GeoTreeWin, GeoWin, MatWin, SensWin, PhotFunWin, PhotSimWin,
                                   RuleWin, GraphWin, FarmWin, PartSimWin, JScriptWin, GlobSetWin, DemoWin };
#ifdef ANTS3_PYTHON
    wins.push_back(PythonWin);
#endif

    for (auto * win : wins) delete win;

    qDebug() << "<MainWindow close event processing finished";
}

void MainWindow::saveWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this, GeoTreeWin, GeoWin, MatWin, SensWin, PhotFunWin, PhotSimWin,
                                   RuleWin, GraphWin, FarmWin, PartSimWin, JScriptWin, JScriptWin->ScriptMsgWin,
                                   GlobSetWin, GuiFromScrWin, DemoWin };
#ifdef ANTS3_PYTHON
    wins.push_back(PythonWin);
    wins.push_back(PythonWin->ScriptMsgWin);
#endif

    GuiFromScrWin->hide();
    for (auto * w : wins) w->storeGeomStatus();
}

void MainWindow::loadWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this, GeoTreeWin, GeoWin, MatWin, SensWin, PhotFunWin, PhotSimWin,
                                   RuleWin, GraphWin,  FarmWin, PartSimWin, JScriptWin, JScriptWin->ScriptMsgWin,
                                   GlobSetWin, GuiFromScrWin, DemoWin };
#ifdef ANTS3_PYTHON
    wins.push_back(PythonWin);
    wins.push_back(PythonWin->ScriptMsgWin);
#endif

    for (auto * w : wins) w->restoreGeomStatus();
}

#include "amaterialhub.h"
#include "asensorhub.h"
#include "ainterfacerulehub.h"
#include "aparticlesimhub.h"
#include "aphotonsimhub.h"
#include "ageoconsts.h"
#include "aparticleanalyzerhub.h"
void MainWindow::on_pbNew_clicked()
{
    bool ok = guitools::confirm("Start a new configuration?\nUnsaved changes will be lost", this);
    if (!ok) return;

    AMaterialHub::getInstance().clear();

    ASensorHub::getInstance().clear();

    AInterfaceRuleHub::getInstance().clearRules();

    AParticleSimHub::getInstance().clear();
    PartSimWin->onNewConfigStartedInGui();

    APhotonSimHub::getInstance().clear();

    AParticleAnalyzerHub::getInstance().clear();

    // Reconstruction
    // LRFs

    AGeoConsts::getInstance().clearConstants();
    AGeometryHub::getInstance().clearWorld();
    onRebuildGeometryRequested();

    Config.ConfigName = "";
    Config.ConfigDescription = "";

    AConfig::getInstance().updateJSONfromConfig();

    updateAllGuiFromConfig();
}

void MainWindow::on_actionQuickLoad_slot_1_hovered()
{
    ui->actionQuickLoad_slot_1->setToolTip(getQuickLoadMessage(1));
}

void MainWindow::on_actionQuickLoad_slot_2_hovered()
{
    ui->actionQuickLoad_slot_2->setToolTip(getQuickLoadMessage(2));
}

void MainWindow::on_actionQuickLoad_slot_3_hovered()
{
    ui->actionQuickLoad_slot_3->setToolTip(getQuickLoadMessage(3));
}

void MainWindow::on_actionLoad_last_config_hovered()
{
    ui->actionLoad_last_config->setToolTip(getQuickLoadMessage(0));
}

#include <QFileInfo>
QString MainWindow::getQuickLoadMessage(int index)
{
    QString txt;
    if ((index < 0) || index > 3) return txt;
    const QString fileName = GlobSet.getQuickFileName(index);
    if (fileName.isEmpty()) return txt;
    QFile file(fileName);
    if (!file.exists()) return txt;

    if (!file.open(QIODevice::ReadOnly | QFile::Text)) return txt;

    QString name, desc;
    QTextStream in(&file);
    for (int iLine = 0; iLine < 10; iLine++) // asssuming the needed lines are on top of the file!
    {
        const QString line = in.readLine().simplified();
        const QStringList sl = line.split(':', Qt::SkipEmptyParts);
        if (sl.size() < 2) continue;
        if      (sl[0] == "\"ConfigName\"")
        {
            name = sl[1].simplified();
            name.remove(0, 1);
            name.chop(2);
        }
        else if (sl[0] == "\"ConfigDescription\"")
        {
            desc = sl[1].simplified();
            desc.remove(0, 1);
            desc.chop(2);
        }
    }
    file.close();

    QString ret = name + '\n' + desc;
    if (ret == '\n') ret.clear();

    ret += '\n';
    ret += QFileInfo(fileName).lastModified().toString();
    return ret;
}

void MainWindow::on_actionShow_hints_triggered()
{
    QString str = ""
                  "Mouse right-button click on a window button closes that window.\n\n"
                  "Hovering with mouse over a quick load slot shows name and description for that configuration."
                  "";

    guitools::message1(str, "main window hints", this);
}

#include "aconfigexamplebrowser.h"
void MainWindow::on_pbExamples_clicked()
{
    if (!ConfigExampleBrowser)
    {
        ConfigExampleBrowser = new AConfigExampleBrowser(this);
        ConfigExampleBrowser->setWindowModality(Qt::ApplicationModal);
        connect(ConfigExampleBrowser, &AConfigExampleBrowser::requestLoadFile, this, &MainWindow::onRequestLoadConfiguration);
    }

    ConfigExampleBrowser->show();
}

void MainWindow::onRequestLoadConfiguration(QString fileName)
{
    fileName = GlobSet.ExamplesDir + "/configs/" + fileName;
    qDebug() << "Loading configuration from file" << fileName;
    QString err = Config.load(fileName, true);
    if (!err.isEmpty()) guitools::message(err, this);
}

#include "TROOT.h"
void MainWindow::on_actionVersions_triggered()
{
    int majVer = ANTS3_MAJOR;
    QString mav = QString::number(majVer);
    int minVer = ANTS3_MINOR;
    QString miv = QString::number(minVer);
    if (miv.length() == 1) miv = "0" + miv;

    QString qv = QT_VERSION_STR;

    QString out = "ANTS3\n"
                  "   version:  " + mav + "." + miv + "\n"
                  "   build date:  " + QString::fromLocal8Bit(__DATE__) + "\n"
                  "\n"
                  "Qt version:  " + qv + "\n"
                  "\n"
                  "ROOT version:  " + gROOT->GetVersion() + "\n"
                  "\n"
                  "Optional components:\n"
                  "  Python scripting: "
#ifdef ANTS3_PYTHON
                  "on (Python v" + AScriptHub::getInstance().getPythonVersion() + ")"
#else
                  "off"
#endif
                  "\n"
                  "  ROOT html server: "
#ifdef USE_ROOT_HTML
                  "on"
#else
                  "off"
#endif
                  "\n"
                  "  JSROOT: "
#ifdef __USE_ANTS_JSROOT__
                  "on"
#else
                  "off"
#endif
                  "\n"
                  "  Farm: "
#ifdef WEBSOCKETS
                  "on"
#else
                  "off"
#endif
                  "\n"
/*
#ifdef __USE_ANTS_NCRYSTAL__
#else
#endif
" - NCrystal library (neutron scattering)   \n"
*/
                  "";

    guitools::message(out, this);
}

void MainWindow::on_pbLoadConfig_customContextMenuRequested(const QPoint &)
{
    on_actionLoad_last_config_triggered();
}

void MainWindow::on_actionDataTransport_demo_triggered()
{
    DemoWin->onMainWinButtonClicked(true);
}

