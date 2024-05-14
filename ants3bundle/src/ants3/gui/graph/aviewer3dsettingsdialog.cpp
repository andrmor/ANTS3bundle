#include "aviewer3dsettingsdialog.h"
#include "ui_aviewer3dsettingsdialog.h"
#include "aviewer3dsettings.h"

#include <QDoubleValidator>

AViewer3DSettingsDialog::AViewer3DSettingsDialog(AViewer3DSettings & settings, QWidget * parent) :
    QDialog(parent), Settings(settings),
    ui(new Ui::AViewer3DSettingsDialog)
{
    ui->setupUi(this);

    for (const auto & p : Settings.DefinedPalettes)
        ui->cobPalette->addItem(p.first);

    QDoubleValidator * dval = new QDoubleValidator(this);
    ui->ledFixedMaximum->setValidator(dval);
    ui->ledColorScaleUnity->setValidator(dval);

    ui->cobMaximumMode->setCurrentIndex((int)Settings.MaximumMode);
    ui->ledFixedMaximum->setText(QString::number(Settings.FixedMaximum));
    ui->sbMaxInFractionFoV->setValue(Settings.PercentFieldOfView);
    ui->ledColorScaleUnity->setText(QString::number(Settings.ScalingFactor));
    ui->cbSuppressZero->setChecked(Settings.SuppressZero);

    ui->cbTitleVisible->setChecked(Settings.TitleVisible);

    oldFoV = Settings.PercentFieldOfView;
}

AViewer3DSettingsDialog::~AViewer3DSettingsDialog()
{
    delete ui;
}

bool AViewer3DSettingsDialog::isRecalcMaxRequired() const
{
    return (oldFoV != Settings.PercentFieldOfView);
}

#include "TStyle.h"
void AViewer3DSettingsDialog::on_pbAccept_clicked()
{
    Settings.MaximumMode = static_cast<AViewer3DSettings::EMaximumMode>(ui->cobMaximumMode->currentIndex());
    Settings.FixedMaximum = ui->ledFixedMaximum->text().toDouble();
    Settings.PercentFieldOfView = ui->sbMaxInFractionFoV->value();
    Settings.ScalingFactor = ui->ledColorScaleUnity->text().toDouble();
    Settings.SuppressZero = ui->cbSuppressZero->isChecked();

    Settings.TitleVisible = ui->cbTitleVisible->isChecked();

    for (const auto & p : Settings.DefinedPalettes)
        if (ui->cobPalette->currentText() == p.first)
        {
            gStyle->SetPalette(p.second);
            break;
        }

    accept();
}

void AViewer3DSettingsDialog::on_pbCancel_clicked()
{
    reject();
}

