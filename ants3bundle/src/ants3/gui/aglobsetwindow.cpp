#include "aglobsetwindow.h"
#include "ui_aglobsetwindow.h"
#include "a3global.h"
#include "ascriptwindow.h"
#include "guitools.h"

#ifdef USE_ROOT_HTML
    #include "aroothttpserver.h"
#endif

#ifdef WEBSOCKETS
    #include "awebsocketserver.h"
#endif

//Qt
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QHostAddress>
#include <QStyleFactory>
#include <QToolTip>

#include "TGeoManager.h"

AGlobSetWindow::AGlobSetWindow(QWidget * parent) :
    AGuiWindow("Glob", parent),
    GlobSet(A3Global::getInstance()),
    ui(new Ui::AGlobSetWindow)
{
    ui->setupUi(this);

    const QString defaultStyleName = QApplication::style()->objectName();
    QStringList styleNames = QStyleFactory::keys();
    for (int i = 1, size = styleNames.size(); i < size; ++i)
    {
        if (defaultStyleName.compare(styleNames.at(i), Qt::CaseInsensitive) == 0)
        {
            styleNames.swapItemsAt(0, i);
            break;
        }
    }
    ui->cobStyle->addItems(styleNames);

    ui->cbUseStyleSystPalette->setChecked(true);
    on_cbUseStyleSystPalette_clicked(true);

    updateGui();

    QToolTip::setPalette(QPalette());

#ifdef USE_ROOT_HTML
    ARootHttpServer & rs = ARootHttpServer::getInstance();
    QObject::connect(&rs, &ARootHttpServer::StatusChanged, this, &AGlobSetWindow::updateNetGui);

    if (rs.Autostart) rs.start();  //does nothing if compilation flag is not set
#endif
}

AGlobSetWindow::~AGlobSetWindow()
{
    delete ui; ui = nullptr;
}

void AGlobSetWindow::updateGui()
{
    ui->leDataExchangeDir->setText(GlobSet.ExchangeDir);

    ui->sbTabInSpaces->setValue(GlobSet.TabInSpaces);
    ui->cbOpenImageExternalEditor->setChecked(GlobSet.OpenImageExternalEditor);

    ui->sbNumBinsHistogramsX->setValue(GlobSet.BinsX);
    ui->sbNumBinsHistogramsY->setValue(GlobSet.BinsY);
    ui->sbNumBinsHistogramsZ->setValue(GlobSet.BinsZ);

    //ui->sbNumPointsFunctionX->setValue(GlobSet.FunctionPointsX);
    //ui->sbNumPointsFunctionY->setValue(GlobSet.FunctionPointsY);

    ui->sbNumSegments->setValue(GlobSet.NumSegmentsTGeo);

    updateNetGui();
}

void AGlobSetWindow::updateNetGui()
{
    ui->leWebSocketIP->setText(GlobSet.DefaultWebSocketIP);
    ui->leWebSocketPort->setText(QString::number(GlobSet.DefaultWebSocketPort));

#ifdef WEBSOCKETS
    AWebSocketServer & WebServer = AWebSocketServer::getInstance();
    bool fWebSocketRunning = WebServer.isRunning();
    ui->cbRunWebSocketServer->setChecked( fWebSocketRunning );
    if (fWebSocketRunning)
    {
        ui->leWebSocketPort->setText(QString::number(WebServer.getPort()));
        ui->leWebSocketURL->setText(WebServer.getUrl());
    }
#else
    ui->cbRunWebSocketServer->setChecked(false);
    ui->cbRunWebSocketServer->setEnabled(false);
#endif

#ifdef USE_ROOT_HTML
    ARootHttpServer & ser = ARootHttpServer::getInstance();
    bool fRootServerRunning = ser.isRunning();
    ui->cbRunRootServer->setChecked(fRootServerRunning);
    ui->cbAutoRunRootServer->setChecked(ser.Autostart);
    QString sPort = QString::number(ser.Port);
    ui->leRootServerPort->setText(sPort);
    QString url = ( ser.isRunning() ? "http://localhost:" + sPort : "" );
    ui->leRootServerURL->setText(url);
#else
    ui->cbRunRootServer->setChecked(false);
    ui->cbRunRootServer->setEnabled(false);
    ui->cbAutoRunRootServer->setEnabled(false);
    ui->leRootServerPort->setEnabled(false);
    ui->leRootServerURL->setEnabled(false);
#endif

#ifdef __USE_ANTS_JSROOT__
    ui->leJSROOT->setText(ser.ExternalJSROOT);
#else
    ui->leJSROOT->setEnabled(false);
#endif
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
    setTab(3);
}

bool AGlobSetWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) updateGui();

    return QMainWindow::event(event);
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
    //QDesktopServices::openUrl(QUrl("file///:"+ui->leDataExchangeDir->text(), QUrl::TolerantMode));
    QDesktopServices::openUrl( QUrl::fromLocalFile(ui->leDataExchangeDir->text()) );
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
}

#ifdef WEBSOCKETS
void AGlobSetWindow::on_cbRunWebSocketServer_clicked(bool checked)
{
    AWebSocketServer & WebServer = AWebSocketServer::getInstance();
    WebServer.stopListen();

    if (checked) WebServer.startListen(QHostAddress(GlobSet.DefaultWebSocketIP), GlobSet.DefaultWebSocketPort);

    updateNetGui();
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
void AGlobSetWindow::on_leWebSocketIP_editingFinished()
{
    QString newIP = ui->leWebSocketIP->text();
    if (newIP == GlobSet.DefaultWebSocketIP) return;

    QHostAddress ip = QHostAddress(newIP);
    if (ip.isNull())
    {
        ui->leWebSocketIP->setText(GlobSet.DefaultWebSocketIP);
        guitools::message("Bad format of IP: use, e.g., 127.0.0.1", this);
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
#endif

#ifdef USE_ROOT_HTML
void AGlobSetWindow::on_cbAutoRunRootServer_clicked()
{
    ARootHttpServer & ser = ARootHttpServer::getInstance();
    ser.Autostart = ui->cbAutoRunRootServer->isChecked();
}
void AGlobSetWindow::on_leRootServerPort_editingFinished()  // !!!*** stop when changed? restart?
{
    ARootHttpServer & ser = ARootHttpServer::getInstance();
    int oldp = ser.Port;
    int newp = ui->leRootServerPort->text().toInt();
    if (oldp == newp) return;
    ser.Port = newp;

    ui->cbRunRootServer->setChecked(false);
}
void AGlobSetWindow::on_cbRunRootServer_clicked(bool checked)
{
    ARootHttpServer & ser = ARootHttpServer::getInstance();

    if (checked)
        ser.start();  //does nothing if compilation flag is not set
    else
        ser.stop();
}
#endif

#ifdef __USE_ANTS_JSROOT__
void AGlobSetWindow::on_leJSROOT_editingFinished()   // !!!*** stop when changed? restart?
{
    ARootHttpServer & ser = ARootHttpServer::getInstance();
    const QString newa = ui->leJSROOT->text();
    if (newa == ser.ExternalJSROOT) return;
    ser.ExternalJSROOT = newa;

    ui->cbRunRootServer->setChecked(false);
}
#endif

/*
#include "aproxystyle.h"
void AGlobSetWindow::on_cobColorPalette_activated(int index)
{
    switch (index)
    {
    case 0:
        {
            QStyle * style = new AProxyStyle();
            QApplication::setPalette(style->standardPalette());

            //QApplication::setStyle(new AProxyStyle());
            //QApplication::setPalette(
            //QApplication::style()->standardPalette() );
                    //QPalette());
        }
        break;
    case 1:
        {
            QStyle * style = QStyleFactory::create("Windows");
            QApplication::setPalette(style->standardPalette());
        }
        break;
    case 2:
        {
            QColor darkGray(53, 53, 53);
            QColor gray(128, 128, 128);
            QColor black(25, 25, 25);
            QColor blue(42, 130, 218);

            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, darkGray);
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, black);
            darkPalette.setColor(QPalette::AlternateBase, darkGray);
            darkPalette.setColor(QPalette::ToolTipBase, blue);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, darkGray);
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::Link, blue);
            darkPalette.setColor(QPalette::Highlight, blue);
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);

            darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
            darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);

            QApplication::setPalette(darkPalette);
            qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

        }
        break;
    }
}
*/

void AGlobSetWindow::on_cobStyle_textActivated(const QString & arg1)
{
    QApplication::setStyle(QStyleFactory::create(arg1));
}

void AGlobSetWindow::on_cbUseStyleSystPalette_clicked(bool checked)
{
    QApplication::setPalette(checked ? QApplication::style()->standardPalette() : QPalette());
}

void AGlobSetWindow::on_sbTabInSpaces_valueChanged(int arg1)
{
    GlobSet.TabInSpaces = arg1;
}
