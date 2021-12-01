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
    ui->labSensorSignals->setText(RunSet.FileNameSensorSignals);

    ui->cbMonitors->setChecked(RunSet.SaveMonitors);
    ui->labMonitors->setText(RunSet.FileNameMonitors);

    ui->cbPhotonTracks->setChecked(RunSet.SaveTracks);
    ui->labPhotonTracks->setText(RunSet.FileNameTracks);
    ui->sbMaxTracks->setValue(RunSet.MaxTracks);

    ui->cbBombs->setChecked(RunSet.SavePhotonBombs);
    ui->labBombs->setText(RunSet.FileNamePhotonBombs);

    ui->cbStatistics->setChecked(RunSet.SaveStatistics);
    ui->labStatistics->setText(RunSet.FileNameStatistics);
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
    //RunSet.FileNameSensorSignals = ui->leSensorSignals->text();

    RunSet.SaveMonitors          = ui->cbMonitors->isChecked();
    //RunSet.FileNameMonitors      = ui->leMonitors->text();

    RunSet.SaveTracks            = ui->cbPhotonTracks->isChecked();
    //RunSet.FileNameTracks        = ui->lePhotonTracks->text();
    RunSet.MaxTracks             = ui->sbMaxTracks->value();

    RunSet.SavePhotonBombs       = ui->cbBombs->isChecked();
    //RunSet.FileNamePhotonBombs   = ui->leBombs->text();

    RunSet.SaveStatistics        = ui->cbStatistics->isChecked();
    //RunSet.FileNameStatistics    = ui->leStatistics->text();
    RunSet.UpperTimeLimit        = ui->ledTimeLimit->text().toDouble();

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
