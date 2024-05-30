#include "aviewer3dsettingsdialog.h"
#include "ui_aviewer3dsettingsdialog.h"
#include "aviewer3dsettings.h"

#include <QDoubleValidator>

AViewer3DSettingsDialog::AViewer3DSettingsDialog(AViewer3DSettings & settings, QWidget * parent) :
    QDialog(parent), Settings(settings),
    ui(new Ui::AViewer3DSettingsDialog)
{
    ui->setupUi(this);

    QDoubleValidator * dval = new QDoubleValidator(this);
    ui->ledFixedMaximum->setValidator(dval);
    ui->ledColorScaleUnity->setValidator(dval);

    ui->frAdjacent->setEnabled(false);

    for (const auto & p : Settings.DefinedPalettes)
        ui->cobPalette->addItem(p.first);
    ui->cobPalette->setCurrentText(Settings.Palette);

    ui->cobMaximumMode->setCurrentIndex((int)Settings.MaximumMode);
    ui->ledFixedMaximum->setText(QString::number(Settings.FixedMaximum));
    ui->sbMaxInFractionFoV->setValue(Settings.PercentFieldOfView);
    ui->cbApplyScaling->setChecked(Settings.ApplyScaling);
    ui->ledColorScaleUnity->setText(QString::number(Settings.ScalingFactor));
    ui->cbSuppressZero->setChecked(Settings.SuppressZero);

    ui->cbTitleVisible->setChecked(Settings.TitleVisible);
    ui->cbShowPositionLines->setChecked(Settings.ShowPositionLines);

    ui->cbApplyAdjacentAveraging->setChecked(Settings.ApplyAdjacentAveraging);
    ui->sbXleft->setValue(Settings.AdjacentBeforeAfter[0].first);
    ui->sbXright->setValue(Settings.AdjacentBeforeAfter[0].second);
    ui->sbYleft->setValue(Settings.AdjacentBeforeAfter[1].first);
    ui->sbYright->setValue(Settings.AdjacentBeforeAfter[1].second);
    ui->sbZleft->setValue(Settings.AdjacentBeforeAfter[2].first);
    ui->sbZright->setValue(Settings.AdjacentBeforeAfter[2].second);

    oldFoV = Settings.PercentFieldOfView;
}

AViewer3DSettingsDialog::~AViewer3DSettingsDialog()
{
    delete ui;
}

bool AViewer3DSettingsDialog::isRecalcMaxRequired() const
{
    return (Settings.MaximumMode == AViewer3DSettings::FixedMax && Settings.PercentFieldOfView != oldFoV);
}

#include "TStyle.h"
void AViewer3DSettingsDialog::updateSettings()
{
    Settings.MaximumMode = static_cast<AViewer3DSettings::EMaximumMode>(ui->cobMaximumMode->currentIndex());
    Settings.FixedMaximum = ui->ledFixedMaximum->text().toDouble();
    Settings.PercentFieldOfView = ui->sbMaxInFractionFoV->value();
    Settings.ApplyScaling = ui->cbApplyScaling->isChecked();
    Settings.ScalingFactor = ui->ledColorScaleUnity->text().toDouble();
    Settings.SuppressZero = ui->cbSuppressZero->isChecked();

    Settings.TitleVisible = ui->cbTitleVisible->isChecked();
    Settings.ShowPositionLines = ui->cbShowPositionLines->isChecked();

    for (const auto & p : Settings.DefinedPalettes)
        if (ui->cobPalette->currentText() == p.first)
        {
            Settings.Palette = p.first;
            gStyle->SetPalette(p.second);
            break;
        }

    Settings.ApplyAdjacentAveraging = ui->cbApplyAdjacentAveraging->isChecked();
    Settings.AdjacentBeforeAfter[0].first  = ui->sbXleft->value();
    Settings.AdjacentBeforeAfter[0].second = ui->sbXright->value();
    Settings.AdjacentBeforeAfter[1].first  = ui->sbYleft->value();
    Settings.AdjacentBeforeAfter[1].second = ui->sbYright->value();
    Settings.AdjacentBeforeAfter[2].first  = ui->sbZleft->value();
    Settings.AdjacentBeforeAfter[2].second = ui->sbZright->value();
}

void AViewer3DSettingsDialog::on_pbAccept_clicked()
{
    updateSettings();
    accept();
}

void AViewer3DSettingsDialog::on_pbCancel_clicked()
{
    reject();
}

void AViewer3DSettingsDialog::on_pbCopyToScaling_clicked()
{
    ui->ledColorScaleUnity->setText(ui->ledFixedMaximum->text());
}

void AViewer3DSettingsDialog::on_pbApply_clicked()
{
    updateSettings();
    emit requestUpdate(isRecalcMaxRequired());
}

