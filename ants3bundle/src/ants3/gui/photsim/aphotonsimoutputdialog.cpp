#include "aphotonsimoutputdialog.h"
#include "ui_aphotonsimoutputdialog.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "guitools.h"

#include <QFileDialog>

APhotonSimOutputDialog::APhotonSimOutputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::APhotonSimOutputDialog)
{
    ui->setupUi(this);

    const APhotSimRunSettings & RunSet = APhotonSimHub::getConstInstance().Settings.RunSet;

    ui->leOutputDirectory->setText(RunSet.OutputDirectory);
    ui->cobFileFormat->setCurrentIndex(RunSet.BinaryFormat ? 1 : 0);

    ui->cbSensorSignals->setChecked(RunSet.SaveSensorSignals);
    ui->labSensorSignals->setText(RunSet.FileNameSensorSignals);

    ui->cbSensorLog->setChecked(RunSet.SaveSensorLog);
    ui->frSensorLog->setVisible(RunSet.SaveSensorLog);
    ui->labSensorLog->setText(RunSet.FileNameSensorLog);
    ui->cbSensorLogTime->setChecked(RunSet.SensorLogTime);
    ui->cbSensorLogXY->setChecked(RunSet.SensorLogXY);
    ui->cbSensorLogAngle->setChecked(RunSet.SensorLogAngle);
    ui->cbSensorLogWave->setChecked(RunSet.SensorLogWave);

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

    ui->cbPhotonLog->setChecked(RunSet.PhotonLogSet.Save);
    ui->labPhotonLog->setText(RunSet.PhotonLogSet.FileName);
}

APhotonSimOutputDialog::~APhotonSimOutputDialog()
{
    delete ui;
}

void APhotonSimOutputDialog::on_pbAccept_clicked()
{
    if (ui->cbSensorLog->isChecked())
        if (!ui->cbSensorLogTime->isChecked() && !ui->cbSensorLogXY->isChecked() && !ui->cbSensorLogAngle->isChecked() && !ui->cbSensorLogWave->isChecked())
        {
            guitools::message("Check at least one of the checkboxes in photon log configuration", this);
            return;
        }

    APhotSimRunSettings & RunSet = APhotonSimHub::getInstance().Settings.RunSet;

    RunSet.OutputDirectory   = ui->leOutputDirectory->text();
    RunSet.BinaryFormat      = (ui->cobFileFormat->currentIndex() == 1);

    RunSet.SaveSensorSignals = ui->cbSensorSignals->isChecked();

    RunSet.SaveSensorLog     = ui->cbSensorLog->isChecked();
    RunSet.SensorLogTime     = ui->cbSensorLogTime->isChecked();
    RunSet.SensorLogXY       = ui->cbSensorLogXY->isChecked();
    RunSet.SensorLogAngle    = ui->cbSensorLogAngle->isChecked();
    RunSet.SensorLogWave     = ui->cbSensorLogWave->isChecked();

    RunSet.SaveMonitors      = ui->cbMonitors->isChecked();

    RunSet.SaveTracks        = ui->cbPhotonTracks->isChecked();
    RunSet.MaxTracks         = ui->sbMaxTracks->value();

    RunSet.SavePhotonBombs   = ui->cbBombs->isChecked();

    RunSet.SaveStatistics    = ui->cbStatistics->isChecked();
    RunSet.UpperTimeLimit    = ui->ledTimeLimit->text().toDouble();

    RunSet.PhotonLogSet.Save = ui->cbPhotonLog->isChecked();

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
