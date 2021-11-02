#include "ademowindow.h"
#include "ui_ademowindow.h"
#include "ajscripthub.h"
#include "ajscriptmanager.h"
#include "adispatcherinterface.h"
#include "a3config.h"
#include "ademomanager.h"
#include "guitools.h"
#include "afiletools.h"

ADemoWindow::ADemoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ADemoWindow),
    Config(A3Config::getInstance())
{
    ui->setupUi(this);

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    connect(&Dispatcher, &ADispatcherInterface::updateProgress, this, &ADemoWindow::onProgressReceived);

    ADemoManager & DemoMan = ADemoManager::getInstance();
    connect(&DemoMan, &ADemoManager::finished, this, &ADemoWindow::onDemoFinsihed);

    AJScriptManager * SM = &AJScriptHub::manager();
    connect(SM, &AJScriptManager::finished, this, &ADemoWindow::onDemoFinsihed);

    ui->pteData->appendPlainText(Config.lines);
    ui->leFrom->setText(Config.from);
    ui->leTo->setText(Config.to);
}

ADemoWindow::~ADemoWindow()
{
    delete ui;
}

void ADemoWindow::on_pbRun_clicked()
{
    ui->prBar->setValue(0);
    setDisabled(true);
    qApp->processEvents();

    Config.lines = ui->pteData->document()->toPlainText();
    ADemoManager & DemoMan = ADemoManager::getInstance();
    DemoMan.run();
}

void ADemoWindow::onProgressReceived(double progress)
{
    ui->prBar->setValue(progress*100.0);
}

#include "a3global.h"
void ADemoWindow::onDemoFinsihed()
{
    setDisabled(false);

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

void ADemoWindow::on_leFrom_editingFinished()
{
    Config.from = ui->leFrom->text();
}

void ADemoWindow::on_leTo_editingFinished()
{
    Config.to = ui->leTo->text();
}
