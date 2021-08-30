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

    ui->leOutputDirectory->setText(RunSet.OutputDirectory);

    ui->cobAsciiBinary->setCurrentIndex( RunSet.AsciiOutput ? 0 : 1);
    ui->sbAsciiPrecision->setValue(RunSet.AsciiPrecision);
}

AParticleSimOutputDialog::~AParticleSimOutputDialog()
{
    delete ui;
}

void AParticleSimOutputDialog::on_pbAccept_clicked()
{
    RunSet.OutputDirectory = ui->leOutputDirectory->text();

    RunSet.AsciiPrecision = ui->sbAsciiPrecision->value();
    RunSet.AsciiOutput = (ui->cobAsciiBinary->currentIndex() == 0);

    RunSet.SaveTrackingData = ui->cbTrackingData->isChecked();
    RunSet.FileNameTrackingData = ui->leTracks->text();

    accept();
}

void AParticleSimOutputDialog::on_pbChangeDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory",
                                                    RunSet.OutputDirectory,
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

