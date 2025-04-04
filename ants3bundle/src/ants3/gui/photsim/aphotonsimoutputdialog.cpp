#include "aphotonsimoutputdialog.h"
#include "ui_aphotonsimoutputdialog.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "aphotonlogsettingsform.h"
#include "guitools.h"

#include <QFileDialog>

APhotonSimOutputDialog::APhotonSimOutputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::APhotonSimOutputDialog)
{
    ui->setupUi(this);

    PhotonLog = new APhotonLogSettingsForm(this);
    QHBoxLayout * layLog = new QHBoxLayout();
    layLog->setContentsMargins(130,0,0,0);
    layLog->addWidget(PhotonLog);
    ui->verticalLayout->insertLayout(18, layLog);

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

    ui->cbPhotonLog->setChecked(RunSet.PhotonLogSet.Enabled);
    ui->labPhotonLog->setText(RunSet.PhotonLogSet.FileName);

    ui->cbSaveConfig->setChecked(RunSet.SaveConfig);

    PhotonLog->updateGui(RunSet.PhotonLogSet);
    PhotonLog->setEnabled(RunSet.PhotonLogSet.Enabled);
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

    QString err = PhotonLog->updateSettings(RunSet.PhotonLogSet);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

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

    RunSet.PhotonLogSet.Enabled = ui->cbPhotonLog->isChecked();

    RunSet.SaveConfig        = ui->cbSaveConfig->isChecked();

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

void APhotonSimOutputDialog::on_cbPhotonLog_toggled(bool checked)
{
    PhotonLog->setEnabled(checked);
}

#include <QDesktopServices>
void APhotonSimOutputDialog::on_pbChangeDir_customContextMenuRequested(const QPoint &)
{
    QString txt = ui->leOutputDirectory->text();
    if (txt.isEmpty()) return;
    QDesktopServices::openUrl( QUrl::fromLocalFile(txt) );
}

