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
    ui->leSensorSignals->setText(RunSet.FileNameSensorSignals);
    ui->lePhotonTracks->setText(RunSet.FileNameTracks);
    ui->leBombs->setText(RunSet.FileNamePhotonBombs);

    ui->cbSensorSignals->setChecked(RunSet.SaveSensorSignals);
    ui->cbPhotonTracks->setChecked(RunSet.SaveTracks);
    ui->cbBombs->setChecked(RunSet.SavePhotonBombs);

    ui->sbMaxTracks->setValue(RunSet.MaxTracks);
}

APhotonSimOutputDialog::~APhotonSimOutputDialog()
{
    delete ui;
}

void APhotonSimOutputDialog::on_pbAccept_clicked()
{
    APhotSimRunSettings & RunSet = APhotonSimHub::getInstance().Settings.RunSet;

    RunSet.OutputDirectory       = ui->leOutputDirectory->text();
    RunSet.FileNameSensorSignals = ui->leSensorSignals->text();
    RunSet.FileNameTracks        = ui->lePhotonTracks->text();
    RunSet.FileNamePhotonBombs   = ui->leBombs->text();

    RunSet.SaveSensorSignals     = ui->cbSensorSignals->isChecked();
    RunSet.SaveTracks            = ui->cbPhotonTracks->isChecked();
    RunSet.SavePhotonBombs       = ui->cbBombs->isChecked();

    RunSet.MaxTracks             = ui->sbMaxTracks->value();

    accept();
}

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

