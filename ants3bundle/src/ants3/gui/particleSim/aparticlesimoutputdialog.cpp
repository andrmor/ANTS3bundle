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

    ui->cbSaveConfig->setChecked(RunSet.SaveConfig);

    ui->cobAsciiBinary->setCurrentIndex( RunSet.AsciiOutput ? 0 : 1);
    ui->sbAsciiPrecision->setValue(RunSet.AsciiPrecision);

    ui->cbTrackingData->setChecked(RunSet.SaveTrackingHistory);
    ui->labTracks->setText(RunSet.FileNameTrackingHistory.data());

    ui->cbSaveParticles->setChecked(RunSet.SaveSettings.Enabled);
    ui->labSaveParticles->setText( QString(RunSet.SaveSettings.FileName.data()) );
    ui->leVolume->setText( QString(RunSet.SaveSettings.VolumeName.data()) );
    ui->cbStopTracking->setChecked(RunSet.SaveSettings.StopTrack);
    ui->cbUseTimeWindow->setChecked(RunSet.SaveSettings.TimeWindow);
    ui->ledTimeFrom->setText(QString::number(RunSet.SaveSettings.TimeFrom));
    ui->ledTimeTo->setText(QString::number(RunSet.SaveSettings.TimeTo));

    ui->cbDeposition->setChecked(RunSet.SaveDeposition);
    ui->labDeposition->setText(RunSet.FileNameDeposition.data());
    ui->leDepo->clear();
    QStringList svl;
    for (const auto & s : RunSet.SaveDepositionVolumes) svl << s.data();
    ui->leDepo->setText(svl.join(" "));
    ui->cbDepoIncludeAllScintillators->setChecked(RunSet.SaveDepositionIncludeScintillators);

    ui->cbMonitors->setChecked(RunSet.MonitorSettings.Enabled);
    ui->labMonitors->setText(RunSet.MonitorSettings.FileName.data());

    ui->cbCalorimeters->setChecked(RunSet.CalorimeterSettings.Enabled);
    ui->labCalorimeters->setText(RunSet.CalorimeterSettings.FileName.data());

    ui->cbParticleAnalyzers->setChecked(RunSet.AnalyzerSettings.Enabled);
    ui->labParticleAnalyzers->setText(RunSet.AnalyzerSettings.FileName.data());
}

AParticleSimOutputDialog::~AParticleSimOutputDialog()
{
    delete ui;
}

void AParticleSimOutputDialog::on_pbAccept_clicked()
{
    RunSet.OutputDirectory = ui->leOutputDirectory->text().toLatin1().data();

    RunSet.SaveConfig = ui->cbSaveConfig->isChecked();

    RunSet.AsciiPrecision = ui->sbAsciiPrecision->value();
    RunSet.AsciiOutput = (ui->cobAsciiBinary->currentIndex() == 0);

    RunSet.SaveTrackingHistory = ui->cbTrackingData->isChecked();
    //RunSet.FileNameTrackingHistory = ui->leTracks->text().toLatin1().data();

    RunSet.SaveSettings.Enabled = ui->cbSaveParticles->isChecked();
    //RunSet.SaveSettings.FileName = ui->leSaveParticles->text().toLatin1().data();
    RunSet.SaveSettings.VolumeName = ui->leVolume->text().toLatin1().data();
    RunSet.SaveSettings.StopTrack = ui->cbStopTracking->isChecked();
    RunSet.SaveSettings.TimeWindow = ui->cbUseTimeWindow->isChecked();
    RunSet.SaveSettings.TimeFrom = ui->ledTimeFrom->text().toDouble();
    RunSet.SaveSettings.TimeTo = ui->ledTimeTo->text().toDouble();

    RunSet.SaveDeposition = ui->cbDeposition->isChecked();
    //RunSet.FileNameDeposition = ui->leDeposition->text().toLatin1().data();
    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\n|\\t)"); //separators: ' ' or ',' or '\n' or '\t'
    const QString t = ui->leDepo->text();
    const QStringList sl = t.split(rx, Qt::SkipEmptyParts);
    // !!!*** check for overlaps in names, considering also wildcard
    RunSet.SaveDepositionVolumes.clear();
    for (const QString & s : sl) RunSet.SaveDepositionVolumes.push_back(s.toLatin1().data());
    RunSet.SaveDepositionIncludeScintillators = ui->cbDepoIncludeAllScintillators->isChecked();

    RunSet.MonitorSettings.Enabled = ui->cbMonitors->isChecked();
    //RunSet.MonitorSettings.FileName = ui->leMonitors->text().toLatin1().data();

    RunSet.CalorimeterSettings.Enabled = ui->cbCalorimeters->isChecked();
    //RunSet.CalorimeterSettings.FileName = ui->leCalorimeters->text().toLatin1().data();

    RunSet.AnalyzerSettings.Enabled = ui->cbParticleAnalyzers->isChecked();
    //RunSet.AnalyzerSettings.FileName = ui->labParticleAnalyzers->text().toLatin1().data();

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

#include <QDesktopServices>
void AParticleSimOutputDialog::on_pbChangeDir_customContextMenuRequested(const QPoint &)
{
    const QString dir = ui->leOutputDirectory->text();
    if (dir.isEmpty()) return;

    QDesktopServices::openUrl(QUrl("file:///" + dir, QUrl::TolerantMode));
}

