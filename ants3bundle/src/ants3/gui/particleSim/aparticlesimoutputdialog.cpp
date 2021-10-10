#include "aparticlesimoutputdialog.h"
#include "ui_aparticlesimoutputdialog.h"
#include "aparticlesimhub.h"

#include <QFileDialog>

AParticleSimOutputDialog::AParticleSimOutputDialog(QWidget *parent) :
    QDialog(parent),
    RunSet(AParticleSimHub::getInstance().Settings.RunSet),
    ui(new Ui::AParticleSimOutputDialog)
{
    ui->setupUi(this);

    ui->leOutputDirectory->setText(RunSet.OutputDirectory.data());

    ui->cobAsciiBinary->setCurrentIndex( RunSet.AsciiOutput ? 0 : 1);
    ui->sbAsciiPrecision->setValue(RunSet.AsciiPrecision);

    ui->cbTrackingData->setChecked(RunSet.SaveTrackingHistory);
    ui->leTracks->setText(RunSet.FileNameTrackingHistory.data());

    ui->cbSaveParticles->setChecked(RunSet.SaveSettings.Enabled);
    ui->leSaveParticles->setText( QString(RunSet.SaveSettings.FileName.data()) );
    ui->leVolume->setText( QString(RunSet.SaveSettings.VolumeName.data()) );
    ui->cbStopTracking->setChecked(RunSet.SaveSettings.StopTrack);
    ui->cbUseTimeWindow->setChecked(RunSet.SaveSettings.TimeWindow);
    ui->ledTimeFrom->setText(QString::number(RunSet.SaveSettings.TimeFrom));
    ui->ledTimeTo->setText(QString::number(RunSet.SaveSettings.TimeTo));

    ui->cbDeposition->setChecked(RunSet.SaveDeposition);
    ui->leDeposition->setText(RunSet.FileNameDeposition.data());

    ui->cbMonitors->setChecked(RunSet.SaveMonitors);
    ui->leMonitors->setText(RunSet.FileNameMonitors.data());
}

AParticleSimOutputDialog::~AParticleSimOutputDialog()
{
    delete ui;
}

void AParticleSimOutputDialog::on_pbAccept_clicked()
{
    RunSet.OutputDirectory = ui->leOutputDirectory->text().toLatin1().data();

    RunSet.AsciiPrecision = ui->sbAsciiPrecision->value();
    RunSet.AsciiOutput = (ui->cobAsciiBinary->currentIndex() == 0);

    RunSet.SaveTrackingHistory = ui->cbTrackingData->isChecked();
    RunSet.FileNameTrackingHistory = ui->leTracks->text().toLatin1().data();

    RunSet.SaveSettings.Enabled = ui->cbSaveParticles->isChecked();
    RunSet.SaveSettings.FileName = ui->leSaveParticles->text().toLatin1().data();
    RunSet.SaveSettings.VolumeName = ui->leVolume->text().toLatin1().data();
    RunSet.SaveSettings.StopTrack = ui->cbStopTracking->isChecked();
    RunSet.SaveSettings.TimeWindow = ui->cbUseTimeWindow->isChecked();
    RunSet.SaveSettings.TimeFrom = ui->ledTimeFrom->text().toDouble();
    RunSet.SaveSettings.TimeTo = ui->ledTimeTo->text().toDouble();

    RunSet.SaveDeposition = ui->cbDeposition->isChecked();
    RunSet.FileNameDeposition = ui->leDeposition->text().toLatin1().data();

    RunSet.SaveMonitors = ui->cbMonitors->isChecked();
    RunSet.FileNameMonitors = ui->leMonitors->text().toLatin1().data();

    accept();
}

void AParticleSimOutputDialog::on_pbChangeDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory",
                                                    RunSet.OutputDirectory.data(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) ui->leOutputDirectory->setText(dir);
}

void AParticleSimOutputDialog::on_cobAsciiBinary_currentIndexChanged(int index)
{
    ui->labAsciiPrecision->setVisible(index == 0);
    ui->sbAsciiPrecision->setVisible(index == 0);
}

void AParticleSimOutputDialog::on_pbChangeTracks_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save tracking data", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leTracks->setText(fileName);
}

void AParticleSimOutputDialog::on_pbChangeParticles_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save particles", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leSaveParticles->setText(fileName);
}

void AParticleSimOutputDialog::on_pbChangeDeposition_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save deposition data", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leDeposition->setText(fileName);
}

void AParticleSimOutputDialog::on_pbChangeMonitors_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file to save monitor data", ui->leOutputDirectory->text());
    if (!fileName.isEmpty()) ui->leMonitors->setText(fileName);
}
