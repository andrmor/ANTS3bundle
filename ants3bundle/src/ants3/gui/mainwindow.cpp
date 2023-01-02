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

    GeoWin = new AGeometryWindow(this);
    connect(GeoTreeWin, &AGeoTreeWin::requestShowGeometry,    GeoWin, &AGeometryWindow::ShowGeometry);
    connect(GeoTreeWin, &AGeoTreeWin::requestShowRecursive,   GeoWin, &AGeometryWindow::showRecursive);
    connect(GeoTreeWin, &AGeoTreeWin::requestShowTracks,      GeoWin, &AGeometryWindow::ShowTracks);
    connect(GeoTreeWin, &AGeoTreeWin::requestFocusVolume,     GeoWin, &AGeometryWindow::FocusVolume);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddGeoMarkers,   GeoWin, &AGeometryWindow::addGeoMarkers);
    connect(GeoTreeWin, &AGeoTreeWin::requestClearGeoMarkers, GeoWin, &AGeometryWindow::clearGeoMarkers);

    GraphWin = new GraphWindowClass(this);
    //GraphWindowClass::connectScriptUnitDrawRequests is used to connect draw requests

    MatWin = new AMatWin(this);
    MatWin->initWindow();
    connect(MatWin, &AMatWin::requestRebuildDetector, this,     &MainWindow::onRebuildGeometryRequested);
    connect(MatWin, &AMatWin::requestShowGeometry,    GeoWin,   &AGeometryWindow::ShowGeometry);
    connect(MatWin, &AMatWin::requestDraw,            GraphWin, &GraphWindowClass::onDrawRequest);

    RuleWin = new AInterfaceRuleWin(this);
    connect(RuleWin, &AInterfaceRuleWin::requestClearGeometryViewer, GeoWin,   &AGeometryWindow::ClearRootCanvas);
    connect(RuleWin, &AInterfaceRuleWin::requestShowTracks,          GeoWin,   &AGeometryWindow::ShowTracks);
    connect(RuleWin, &AInterfaceRuleWin::requestDraw,                GraphWin, &GraphWindowClass::onDrawRequest);
    connect(RuleWin, &AInterfaceRuleWin::requestDrawLegend,          GraphWin, &GraphWindowClass::drawLegend);

    SensWin = new ASensorWindow(this);
    connect(SensWin, &ASensorWindow::requestShowSensorModels, GeoWin,   &AGeometryWindow::showSensorModelIndexes);
    connect(SensWin, &ASensorWindow::requestDraw,             GraphWin, &GraphWindowClass::onDrawRequest);

    PhotSimWin = new APhotSimWin(this);
    connect(PhotSimWin, &APhotSimWin::requestShowGeometry,           GeoWin,   &AGeometryWindow::ShowGeometry);
    connect(PhotSimWin, &APhotSimWin::requestShowTracks,             GeoWin,   &AGeometryWindow::ShowTracks);
    connect(PhotSimWin, &APhotSimWin::requestClearGeoMarkers,        GeoWin,   &AGeometryWindow::clearGeoMarkers);
    connect(PhotSimWin, &APhotSimWin::requestAddPhotonNodeGeoMarker, GeoWin,   &AGeometryWindow::addPhotonNodeGeoMarker);
    connect(PhotSimWin, &APhotSimWin::requestShowGeoMarkers,         GeoWin,   &AGeometryWindow::showGeoMarkers);
    connect(PhotSimWin, &APhotSimWin::requestShowPosition,           GeoWin,   &AGeometryWindow::ShowPoint);
    connect(PhotSimWin, &APhotSimWin::requestDraw,                   GraphWin, &GraphWindowClass::onDrawRequest);

    FarmWin = new ARemoteWindow(this);

    PartSimWin = new AParticleSimWin(nullptr); // Qt::WindowModality for the source dialog requires another parent for the window!
    connect(PartSimWin, &AParticleSimWin::requestShowGeometry, GeoWin,   &AGeometryWindow::ShowGeometry);
    connect(PartSimWin, &AParticleSimWin::requestShowTracks,   GeoWin,   &AGeometryWindow::ShowTracks);
    connect(PartSimWin, &AParticleSimWin::requestShowPosition, GeoWin,   &AGeometryWindow::ShowPoint);
    connect(PartSimWin, &AParticleSimWin::requestAddMarker,    GeoWin,   &AGeometryWindow::addGenerationMarker);
    connect(PartSimWin, &AParticleSimWin::requestClearMarkers, GeoWin,   &AGeometryWindow::clearGeoMarkers);
    connect(PartSimWin, &AParticleSimWin::requestCenterView,   GeoWin,   &AGeometryWindow::CenterView);
    connect(PartSimWin, &AParticleSimWin::requestDraw,         GraphWin, &GraphWindowClass::onDrawRequest);
    connect(PartSimWin, &AParticleSimWin::requestAddToBasket,  GraphWin, &GraphWindowClass::addCurrentToBasket);
    connect(PartSimWin, &AParticleSimWin::requestShowGeoObjectDelegate, GeoTreeWin, &AGeoTreeWin::UpdateGeoTree);

    AScriptHub * SH = &AScriptHub::getInstance();
    qDebug() << "Creating JScript window";
    JScriptWin = new AScriptWindow(EScriptLanguage::JavaScript, this);
    JScriptWin->registerInterfaces();
    connect(SH, &AScriptHub::clearOutput_JS,      JScriptWin, &AScriptWindow::clearOutput);
    connect(SH, &AScriptHub::outputText_JS,       JScriptWin, &AScriptWindow::outputText);
    connect(SH, &AScriptHub::outputHtml_JS,       JScriptWin, &AScriptWindow::outputHtml);
    connect(SH, &AScriptHub::showAbortMessage_JS, JScriptWin, &AScriptWindow::outputAbortMessage);
    connect(JScriptWin, &AScriptWindow::requestUpdateGui, this,       &MainWindow::updateAllGuiFromConfig);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddScript,   JScriptWin, &AScriptWindow::onRequestAddScript);
    JScriptWin->updateGui();

#ifdef ANTS3_PYTHON
    qDebug() << "Creating Python window";
    PythonWin = new AScriptWindow(EScriptLanguage::Python, this);
    PythonWin->registerInterfaces();
    connect(SH, &AScriptHub::clearOutput_JS,     PythonWin, &AScriptWindow::clearOutput);
    connect(SH, &AScriptHub::outputText_P,       PythonWin, &AScriptWindow::outputText);
    connect(SH, &AScriptHub::outputHtml_P,       PythonWin, &AScriptWindow::outputHtml);
    connect(SH, &AScriptHub::showAbortMessage_P, PythonWin, &AScriptWindow::outputAbortMessage);
    connect(PythonWin,  &AScriptWindow::requestUpdateGui, this,      &MainWindow::updateAllGuiFromConfig);
    connect(GeoTreeWin, &AGeoTreeWin::requestAddScript,   PythonWin, &AScriptWindow::onRequestAddScript);
    PythonWin->updateGui();
#endif

    GlobSetWin = new AGlobSetWindow(this);
    connect(PhotSimWin, &APhotSimWin::requestConfigureExchangeDir,    GlobSetWin, &AGlobSetWindow::onRequestConfigureExchangeDir);
    connect(PartSimWin, &AParticleSimWin::requestConfigureExchangeDir, GlobSetWin, &AGlobSetWindow::onRequestConfigureExchangeDir);

    DemoWin = new ADemoWindow(this);

    connect(&AScriptHub::getInstance(), &AScriptHub::requestUpdateGui, this, &MainWindow::updateAllGuiFromConfig);

    loadWindowGeometries();

    bool bShown = GeoWin->isVisible();
    GeoWin->show();
    GeoWin->resize(GeoWin->width()+1, GeoWin->height());
    GeoWin->resize(GeoWin->width()-1, GeoWin->height());
    GeoWin->ShowGeometry(false);
    if (!bShown) GeoWin->hide();

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

  // Finalizing
    updateAllGuiFromConfig(); //updateGui();
    SH->finalizeInit();
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
    emit GeoTreeWin->requestClearGeoMarkers(0);
    GeoWin->ShowGeometry();
}

void MainWindow::on_pbGeometry_clicked()
{
    GeoTreeWin->showNormal();
    GeoTreeWin->activateWindow();
    GeoTreeWin->updateGui();
}

void MainWindow::on_pbGeoWin_clicked()
{
    GeoWin->showNormal();
    GeoWin->activateWindow();
    GeoWin->ShowGeometry();
}

void MainWindow::on_pbMaterials_clicked()
{
    MatWin->showNormal();
    MatWin->activateWindow();
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

void MainWindow::on_pbGlobSet_clicked()
{
    GlobSetWin->showNormal();
    GlobSetWin->activateWindow();
    GlobSetWin->updateGui();
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

void MainWindow::on_pbPython_clicked()
{
#ifdef ANTS3_PYTHON
    PythonWin->showNormal();
    PythonWin->activateWindow();
    PythonWin->updateGui();
#else
    guitools::message("Ants3 was compiled without Python support.\nIt can be enabled in ants3.pro by uncommenting:\n#CONFIG += ants3_Python", this);
#endif
}

void MainWindow::on_pbDemo_clicked()
{
    DemoWin->showNormal();
    DemoWin->activateWindow();
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
    const QString fileName = GlobSet.getQuickFileName(0);
    if (!QFile::exists(fileName)) return;

    AConfig::getInstance().load(fileName);
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

    Config.load(fileName);
}

void MainWindow::on_actionQuickLoad_slot_2_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(2);
    if (!QFile::exists(fileName)) return;

    Config.load(fileName);
}

void MainWindow::on_actionQuickLoad_slot_3_triggered()
{
    const QString fileName = GlobSet.getQuickFileName(3);
    if (!QFile::exists(fileName)) return;

    Config.load(fileName);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

// ---

void MainWindow::updateAllGuiFromConfig()
{
    updateGui();

    GeoTreeWin->updateGui();
    MatWin->initWindow();
    SensWin->updateGui();
    RuleWin->updateGui();

    PhotSimWin->updateGui();
    PartSimWin->updateGui();

    QJsonObject json = AConfig::getInstance().JSON["gui"].toObject();
    {
        QJsonObject js;
        bool ok = jstools::parseJson(json, "GeometryWindow", js);
        if (ok) GeoWin->readFromJson(js);
    }

    GeoWin->fRecallWindow = false;
    GeoWin->ShowGeometry(false, false, true);
}

void MainWindow::onRequestSaveGuiSettings()
{
    QJsonObject & JSON = AConfig::getInstance().JSON;

    QJsonObject json;

    {
        QJsonObject js;
        GeoWin->writeToJson(js);
        json["GeometryWindow"] = js;
    }

    JSON["gui"] = json;
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
    if (SensWin->isVisible())
    {
        SensWin->storeGeomStatus();
        SensWin->hide();
    }
    else
    {
        SensWin->restoreGeomStatus();
        SensWin->showNormal();
        SensWin->activateWindow();
        SensWin->updateGui();
    }
}

#include <QThread>
void MainWindow::closeEvent(QCloseEvent *)
{
    qDebug() << "\n<MainWindow shutdown initiated";
    clearFocus();

    qDebug() << "<Saving position/status of all windows";
    saveWindowGeometries();

    qDebug() << "<Preparing graph window for shutdown";
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

    std::vector<AGuiWindow*> wins{ GeoTreeWin, GeoWin,   MatWin,  SensWin,    PhotSimWin,
                                   RuleWin,   GraphWin, FarmWin, PartSimWin, JScriptWin, GlobSetWin, DemoWin };
#ifdef ANTS3_PYTHON
    wins.push_back(PythonWin);
#endif

    for (auto * win : wins) delete win;

    qDebug() << "<MainWindow close event processing finished";
}

void MainWindow::saveWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this,    GeoTreeWin, GeoWin,  MatWin,     SensWin,    PhotSimWin,
                                   RuleWin, GraphWin,  FarmWin, PartSimWin, JScriptWin, JScriptWin->ScriptMsgWin,
                                   GlobSetWin, DemoWin };
#ifdef ANTS3_PYTHON
    wins.push_back(PythonWin);
    wins.push_back(PythonWin->ScriptMsgWin);
#endif

    for (auto * w : wins) w->storeGeomStatus();
}

void MainWindow::loadWindowGeometries()
{
    std::vector<AGuiWindow*> wins{ this,    GeoTreeWin, GeoWin,  MatWin,     SensWin,    PhotSimWin,
                                   RuleWin, GraphWin,  FarmWin, PartSimWin, JScriptWin, JScriptWin->ScriptMsgWin,
                                   GlobSetWin, DemoWin };
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
void MainWindow::on_pbNew_clicked()
{
    bool ok = guitools::confirm("Start a new configuration?\nUnsaved changes will be lost", this);
    if (!ok) return;

    AMaterialHub::getInstance().clear();

    ASensorHub::getInstance().clear();

    AInterfaceRuleHub::getInstance().clearRules();

    AParticleSimHub::getInstance().clear();

    APhotonSimHub::getInstance().clear();

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
