#include "aglobsetwindow.h"
#include "ui_aglobsetwindow.h"
#include "a3global.h"
#include "ascriptwindow.h"
#include "guitools.h"
//#include "anetworkmodule.h"
//#include "agstyle_si.h"

//Qt
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QHostAddress>

#include "TGeoManager.h"

AGlobSetWindow::AGlobSetWindow(QWidget * parent) :
    AGuiWindow("Glob", parent),
    GlobSet(A3Global::getInstance()),
    ui(new Ui::AGlobSetWindow)
{
    ui->setupUi(this);

    updateGui();

    //QObject::connect(GlobSet.getNetworkModule(), &ANetworkModule::StatusChanged, this, &AGlobSetWindow::updateNetGui);
    //if (GlobSet.fRunRootServerOnStart) GlobSet.getNetworkModule()->StartRootHttpServer();  //does nothing if compilation flag is not set
}

AGlobSetWindow::~AGlobSetWindow()
{
    delete ui; ui = nullptr;
}

void AGlobSetWindow::updateGui()
{
    ui->leDataExchangeDir->setText(GlobSet.ExchangeDir);

    ui->cbOpenImageExternalEditor->setChecked(GlobSet.OpenImageExternalEditor);

    ui->sbNumBinsHistogramsX->setValue(GlobSet.BinsX);
    ui->sbNumBinsHistogramsY->setValue(GlobSet.BinsY);
    ui->sbNumBinsHistogramsZ->setValue(GlobSet.BinsZ);

    //ui->sbNumPointsFunctionX->setValue(GlobSet.FunctionPointsX);
    //ui->sbNumPointsFunctionY->setValue(GlobSet.FunctionPointsY);

    ui->sbNumSegments->setValue(GlobSet.NumSegmentsTGeo);

    /*
    ui->cbSaveRecAsTree_IncludePMsignals->setChecked(GlobSet.RecTreeSave_IncludePMsignals);
    ui->cbSaveRecAsTree_IncludeRho->setChecked(GlobSet.RecTreeSave_IncludeRho);
    ui->cbSaveRecAsTree_IncludeTrue->setChecked(GlobSet.RecTreeSave_IncludeTrue);

    ui->cbSaveSimAsText_IncludeNumPhotons->setChecked(GlobSet.SimTextSave_IncludeNumPhotons);
    ui->cbSaveSimAsText_IncludePositions->setChecked(GlobSet.SimTextSave_IncludePositions);
    */

    updateNetGui();
}

void AGlobSetWindow::updateNetGui()
{
/*
    ui->leWebSocketIP->setText(GlobSet.DefaultWebSocketIP);
    ui->leWebSocketPort->setText(QString::number(GlobSet.DefaultWebSocketPort));

    ANetworkModule* Net = GlobSet.getNetworkModule();

    bool fWebSocketRunning = Net->isWebSocketServerRunning();
    ui->cbRunWebSocketServer->setChecked( fWebSocketRunning );
    if (fWebSocketRunning)
    {
        int port = Net->getWebSocketPort();
        ui->leWebSocketPort->setText(QString::number(port));
        ui->leWebSocketURL->setText(Net->getWebSocketServerURL());
    }

    bool fRootServerRunning = Net->isRootServerRunning();
    ui->cbRunRootServer->setChecked( fRootServerRunning );
    ui->cbAutoRunRootServer->setChecked( GlobSet.fRunRootServerOnStart );
*/

/*
#ifdef USE_ROOT_HTML
    ui->leJSROOT->setText(GlobSet.ExternalJSROOT);
    const QString sPort = QString::number(GlobSet.RootServerPort);
    ui->leRootServerPort->setText(sPort);
    const QString url = ( fRootServerRunning ? "http://localhost:" + sPort : "" );
    ui->leRootServerURL->setText(url);
#else
*/
    ui->cbRunRootServer->setChecked(false);
    ui->cbRunRootServer->setEnabled(false);
    ui->cbAutoRunRootServer->setEnabled(false);
    ui->leRootServerPort->setEnabled(false);
    ui->leJSROOT->setEnabled(false);
    ui->leRootServerURL->setEnabled(false);
//#endif
}

void AGlobSetWindow::setTab(int iTab)
{
    if (iTab < 0 || iTab >= ui->twMain->count()) return;
    ui->twMain->setCurrentIndex(iTab);
}

void AGlobSetWindow::showNetSettings()
{
    showNormal();
    activateWindow();
    setTab(5);
}

bool AGlobSetWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) updateGui();

    return QMainWindow::event(event);
}

void AGlobSetWindow::on_pbgStyleScript_clicked()
{
    /*
    MW->extractGeometryOfLocalScriptWindow();
    delete MW->GenScriptWindow; MW->GenScriptWindow = 0;

    AJavaScriptManager* jsm = new AJavaScriptManager(MW->Detector->RandGen);
    MW->GenScriptWindow = new AScriptWindow(jsm, true, this);

    QString example = QString("//see https://root.cern.ch/doc/master/classTStyle.html\n"
                              "\n"
                              "//try, e.g.:\n"
                              "//SetOptStat(\"ei\") //\"nemr\" is redault");
    MW->GenScriptWindow->ConfigureForLightMode(&GlobSet.RootStyleScript,
                                               "Script to set ROOT's gStyle",
                                               example);

    GStyleInterface = new AGStyle_SI();
    MW->GenScriptWindow->RegisterInterfaceAsGlobal(GStyleInterface);
    MW->GenScriptWindow->UpdateGui();

    MW->recallGeometryOfLocalScriptWindow();
    MW->GenScriptWindow->show();
    */

    /*
  MW->extractGeometryOfLocalScriptWindow();
  if (MW->GenScriptWindow) delete MW->GenScriptWindow;
  MW->GenScriptWindow = new GenericScriptWindowClass(MW->Detector->RandGen);
  MW->recallGeometryOfLocalScriptWindow();

  //configure the script window and engine
  GStyleInterface  = new  InterfaceToGStyleScript() ; //deleted by the GenScriptWindow
  MW->GenScriptWindow->SetInterfaceObject(GStyleInterface);
  //QString HelpText = "  Avalable commands: \nsee https://root.cern.ch/root/html/TStyle.html - all commands starting from \"Set\"\n";
  MW->GenScriptWindow->SetExample("");
  MW->GenScriptWindow->SetShowEvaluationResult(false); //do not show "undefined"
  MW->GenScriptWindow->SetTitle("Script to set ROOT's gStyle");
  MW->GenScriptWindow->SetScript(&GlobSet.RootStyleScript);
  MW->GenScriptWindow->SetStarterDir(MW->GlobSet.LibScripts);

  //define what to do on evaluation success
  //connect(MW->GenScriptWindow, SIGNAL(success(QString)), this, SLOT(HolesScriptSuccess()));
  //if needed. connect signals of the interface object with the required slots of any ANTS2 objects
  MW->GenScriptWindow->show();
  */
}

void AGlobSetWindow::on_pbChangeDataExchangeDir_clicked()
{
    const QString starter = GlobSet.ExchangeDir;
    const QString dir = QFileDialog::getExistingDirectory(this, "Select data exchange directory", starter, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;

    GlobSet.ExchangeDir = dir;
    ui->leDataExchangeDir->setText(dir);
}

void AGlobSetWindow::on_leDataExchangeDir_editingFinished()
{
    const QString dir = ui->leDataExchangeDir->text();
    if ( QDir(dir).exists() )
    {
        GlobSet.ExchangeDir = dir;
        ui->leDataExchangeDir->setText(dir);
    }
    else
    {
        ui->leDataExchangeDir->setText(GlobSet.ExchangeDir);
        guitools::message("Directory does not exist:\n" + dir + "\nReverting to previously used one!", this);
    }
}

void AGlobSetWindow::on_cbOpenImageExternalEditor_clicked(bool checked)
{
    GlobSet.OpenImageExternalEditor = checked;
}

void AGlobSetWindow::on_sbNumBinsHistogramsX_editingFinished()
{
    GlobSet.BinsX = ui->sbNumBinsHistogramsX->value();
}

void AGlobSetWindow::on_sbNumBinsHistogramsY_editingFinished()
{
    GlobSet.BinsY = ui->sbNumBinsHistogramsY->value();
}

void AGlobSetWindow::on_sbNumBinsHistogramsZ_editingFinished()
{
    GlobSet.BinsZ = ui->sbNumBinsHistogramsZ->value();
}

void AGlobSetWindow::on_pbChangeDataExchangeDir_customContextMenuRequested(const QPoint & /*pos*/)
{
    if (ui->leDataExchangeDir->text().isEmpty()) return;
    QDesktopServices::openUrl(QUrl("file///:"+ui->leDataExchangeDir->text(), QUrl::TolerantMode));
}

void AGlobSetWindow::on_pbOpen_clicked()
{
    const QString sel = ui->comboBox->currentText();
    QString what;
    if      (sel == "Data exchange") what = GlobSet.ExchangeDir;
    else if (sel == "Executables")   what = GlobSet.ExecutableDir;
    else if (sel == "Local config")  what = GlobSet.ConfigDir;
    else if (sel == "Quicksaves")    what = GlobSet.QuicksaveDir;
    else if (sel == "Examples")      what = GlobSet.ExamplesDir;
    else if (sel == "Resources")     what = GlobSet.ResourcesDir;
    else
    {
        guitools::message("Unknown selector text!", this);
        return;
    }

    QDesktopServices::openUrl(QUrl("file:///" + what, QUrl::TolerantMode));
}

void AGlobSetWindow::onRequestConfigureExchangeDir()
{
    showNormal();
    activateWindow();
    setTab(0);
}

#include "ageometryhub.h"
void AGlobSetWindow::on_sbNumSegments_editingFinished()
{
    GlobSet.NumSegmentsTGeo = ui->sbNumSegments->value();
    AGeometryHub::getInstance().GeoManager->SetNsegments(GlobSet.NumSegmentsTGeo);
    //GeometryWindow->ShowGeometry(false); // !!!*** need?
}

/*
void AGlobSetWindow::on_sbNumPointsFunctionX_editingFinished()
{
    GlobSet.FunctionPointsX = ui->sbNumPointsFunctionX->value();
}
void AGlobSetWindow::on_sbNumPointsFunctionY_editingFinished()
{
    GlobSet.FunctionPointsY = ui->sbNumPointsFunctionY->value();
}
*/

/*
void AGlobSetWindow::on_cbSaveRecAsTree_IncludePMsignals_clicked(bool checked)
{
    GlobSet.RecTreeSave_IncludePMsignals = checked;
}
void AGlobSetWindow::on_cbSaveRecAsTree_IncludeRho_clicked(bool checked)
{
    GlobSet.RecTreeSave_IncludeRho = checked;
}
void AGlobSetWindow::on_cbSaveRecAsTree_IncludeTrue_clicked(bool checked)
{
    GlobSet.RecTreeSave_IncludeTrue = checked;
}
*/

/*
void AGlobSetWindow::on_cbRunWebSocketServer_clicked(bool checked)
{
    ANetworkModule* Net = GlobSet.getNetworkModule();
    Net->StopWebSocketServer();

    if (checked)
        Net->StartWebSocketServer(QHostAddress(GlobSet.DefaultWebSocketIP), GlobSet.DefaultWebSocketPort);
}
void AGlobSetWindow::on_leWebSocketPort_editingFinished()
{
    int oldp = GlobSet.DefaultWebSocketPort;
    int newp = ui->leWebSocketPort->text().toInt();

    if (oldp != newp)
    {
        GlobSet.DefaultWebSocketPort = newp;
        ui->cbRunWebSocketServer->setChecked(false);
        GlobSet.DefaultWebSocketPort = newp;
        ui->leWebSocketPort->setText(QString::number(newp));
    }
}
void AGlobSetWindow::on_cbAutoRunRootServer_clicked()
{
    GlobSet.fRunRootServerOnStart = ui->cbAutoRunRootServer->isChecked();
}
void AGlobSetWindow::on_leRootServerPort_editingFinished()
{
    int oldp = GlobSet.RootServerPort;
    int newp = ui->leRootServerPort->text().toInt();
    if (oldp == newp) return;
    GlobSet.RootServerPort = newp;

    ui->cbRunRootServer->setChecked(false);
}
void AGlobSetWindow::on_leJSROOT_editingFinished()
{
    const QString & olda = GlobSet.ExternalJSROOT;
    QString newa = ui->leJSROOT->text();
    if (olda == newa) return;
    GlobSet.ExternalJSROOT = newa;

    ui->cbRunRootServer->setChecked(false);
}
void AGlobSetWindow::on_cbRunRootServer_clicked(bool checked)
{
    ANetworkModule* Net = GlobSet.getNetworkModule();

    if (checked)
        Net->StartRootHttpServer();  //does nothing if compilation flag is not set
    else
        Net->StopRootHttpServer();
}
void AGlobSetWindow::on_leWebSocketIP_editingFinished()
{
    QString newIP = ui->leWebSocketIP->text();
    if (newIP == GlobSet.DefaultWebSocketIP) return;

    QHostAddress ip = QHostAddress(newIP);
    if (ip.isNull())
    {
        ui->leWebSocketIP->setText(GlobSet.DefaultWebSocketIP);
        message("Bad format of IP: use, e.g., 127.0.0.1", this);
    }
    else
    {
        GlobSet.DefaultWebSocketIP = newIP;
        ui->cbRunWebSocketServer->setChecked(false);
    }
}
void AGlobSetWindow::on_cbRunWebSocketServer_toggled(bool checked)
{
    if (!checked) ui->leWebSocketURL->clear();
}
*/

/*
void AGlobSetWindow::on_cbSaveSimAsText_IncludeNumPhotons_clicked(bool checked)
{
    GlobSet.SimTextSave_IncludeNumPhotons = checked;
}
void AGlobSetWindow::on_cbSaveSimAsText_IncludePositions_clicked(bool checked)
{
    GlobSet.SimTextSave_IncludePositions = checked;
}
*/
