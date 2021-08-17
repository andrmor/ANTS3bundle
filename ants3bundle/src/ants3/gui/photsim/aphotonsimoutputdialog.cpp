#include "aphotonsimoutputdialog.h"
#include "ui_aphotonsimoutputdialog.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"

#include <QFileDialog>

APhotonSimOutputDialog::APhotonSimOutputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::APhotonSimOutputDialog)
{
    ui->setupUi(this);

    const APhotSimRunSettings & RunSet = APhotonSimHub::getConstInstance().Settings.RunSet;

    ui->leOutputDirectory->setText(RunSet.OutputDirectory);

    ui->cbSensorSignals->setChecked(RunSet.SaveSensorSignals);
    ui->leSensorSignals->setText(RunSet.FileNameSensorSignals);

    ui->cbMonitors->setChecked(RunSet.SaveMonitors);
    ui->leMonitors->setText(RunSet.FileNameMonitors);

    ui->cbPhotonTracks->setChecked(RunSet.SaveTracks);
    ui->lePhotonTracks->setText(RunSet.FileNameTracks);
    ui->sbMaxTracks->setValue(RunSet.MaxTracks);

    ui->cbBombs->setChecked(RunSet.SavePhotonBombs);
    ui->leBombs->setText(RunSet.FileNamePhotonBombs);

    ui->cbStatistics->setChecked(RunSet.SaveStatistics);
    ui->leStatistics->setText(RunSet.FileNameStatistics);
    ui->ledTimeLimit->setText(QString::number(RunSet.UpperTimeLimit));
}

APhotonSimOutputDialog::~APhotonSimOutputDialog()
{
    delete ui;
}

void APhotonSimOutputDialog::on_pbAccept_clicked()
{
    APhotSimRunSettings & RunSet = APhotonSimHub::getInstance().Settings.RunSet;

    RunSet.OutputDirectory       = ui->leOutputDirectory->text();

    RunSet.SaveSensorSignals     = ui->cbSensorSignals->isChecked();
    RunSet.FileNameSensorSignals = ui->leSensorSignals->text();

    RunSet.SaveMonitors          = ui->cbMonitors->isChecked();
    RunSet.FileNameMonitors      = ui->leMonitors->text();

    RunSet.SaveTracks            = ui->cbPhotonTracks->isChecked();
    RunSet.FileNameTracks        = ui->lePhotonTracks->text();
    RunSet.MaxTracks             = ui->sbMaxTracks->value();

    RunSet.SavePhotonBombs       = ui->cbBombs->isChecked();
    RunSet.FileNamePhotonBombs   = ui->leBombs->text();

    RunSet.SaveStatistics        = ui->cbStatistics->isChecked();
    RunSet.FileNameStatistics    = ui->leStatistics->text();
    RunSet.UpperTimeLimit        = ui->ledTimeLimit->text().toDouble();

    accept();
}

// !!!*** add to gui tools: select dir

void APhotonSimOutputDialog::on_pbChangeDir_clicked()
{
    const APhotSimRunSettings & RunSet = APhotonSimHub::getConstInstance().Settings.RunSet;
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory",
                                                    RunSet.OutputDirectory,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) ui->leOutputDirectory->setText(dir);
}

void APhotonSimOutputDialog::on_pbChangeSignals_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save sensor output", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leSensorSignals->setText(fileName);
}

void APhotonSimOutputDialog::on_pbChangeTracks_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save photon track", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->lePhotonTracks->setText(fileName);
}

void APhotonSimOutputDialog::on_pbChangeBombs_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save photon bomb data", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leBombs->setText(fileName);
}

void APhotonSimOutputDialog::on_pbChangeStatistics_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save tracing/detection statistics", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leStatistics->setText(fileName);
}

void APhotonSimOutputDialog::on_pbChangeMonitors_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save monitor data", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leMonitors->setText(fileName);
}
