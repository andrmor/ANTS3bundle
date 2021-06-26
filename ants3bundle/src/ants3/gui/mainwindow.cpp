#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "a3scriptmanager.h"
#include "a3scriptres.h"
#include "a3config.h"
#include "guitools.h"
#include "afiletools.h"
#include "a3particlesimmanager.h"

#include <QDebug>

MainWindow::MainWindow(A3ScriptManager & SM, A3ScriptRes & ScrRes, QWidget * parent) :
    QMainWindow(parent),
    Config(A3Config::getInstance()), ScriptManager(SM), ScrRes(ScrRes),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&SM, &A3ScriptManager::finished, this, &MainWindow::onScriptEvaluationFinished);
    connect(ScrRes.ParticleSim, &A3ParticleSimManager::simFinished, this, &MainWindow::onParticleSimulationFinsihed);

    ui->pteData->appendPlainText(Config.lines);
    ui->leFrom->setText(Config.from);
    ui->leTo->setText(Config.to);
}

MainWindow::~MainWindow()
{
    delete ui;
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
