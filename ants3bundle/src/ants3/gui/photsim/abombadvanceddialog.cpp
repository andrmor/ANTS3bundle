#include "abombadvanceddialog.h"
#include "ui_abombadvanceddialog.h"
#include "aphotonsimhub.h"

ABombAdvancedDialog::ABombAdvancedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ABombAdvancedDialog)
{
    ui->setupUi(this);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    foreach(QLineEdit *w, list)
        if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->fPointSourceWave->setEnabled(false);

    const APhotonAdvancedSettings & s = APhotonSimHub::getConstInstance().Settings.BombSet.AdvancedSettings;

    int index = 0;
    if      (s.DirectionMode == APhotonAdvancedSettings::Fixed) index = 1;
    else if (s.DirectionMode == APhotonAdvancedSettings::Cone)  index = 2;
    ui->cobDirectionMode->setCurrentIndex(index);

    ui->ledDX->setText(QString::number(s.DirDX));
    ui->ledDY->setText(QString::number(s.DirDY));
    ui->ledDZ->setText(QString::number(s.DirDZ));
    ui->ledConeAngle->setText(QString::number(s.ConeAngle));

    ui->cbFixWave->setChecked(s.bFixWave);
    ui->sbFixedWave->setValue(s.WaveIndex);

    ui->cbFixedDecay->setChecked(s.bFixDecay);
    ui->ledDecayTime->setText(QString::number(s.DecayTime));

    ui->cbSkipByVolume->setChecked(s.bOnlyVolume);
    ui->leSkipOutsideVolume->setText(s.Volume);

    ui->cbSkipByMaterial->setChecked(s.bOnlyMaterial);
    ui->leSkipOutsideMaterial->setText(s.Material);

    on_cobDirectionMode_currentIndexChanged(ui->cobDirectionMode->currentIndex());
}

ABombAdvancedDialog::~ABombAdvancedDialog()
{
    delete ui;
}

void ABombAdvancedDialog::on_pbAccept_clicked()
{
    APhotonAdvancedSettings & s = APhotonSimHub::getInstance().Settings.BombSet.AdvancedSettings;

    switch (ui->cobDirectionMode->currentIndex())
    {
    default:
    case 0: s.DirectionMode = APhotonAdvancedSettings::Isotropic; break;
    case 1: s.DirectionMode = APhotonAdvancedSettings::Fixed;     break;
    case 2: s.DirectionMode = APhotonAdvancedSettings::Cone;      break;
    }
    s.DirDX = ui->ledDX->text().toDouble();
    s.DirDY = ui->ledDY->text().toDouble();
    s.DirDZ = ui->ledDZ->text().toDouble();
    s.ConeAngle = ui->ledConeAngle->text().toDouble();

    s.bFixWave = ui->cbFixWave->isChecked();
    s.WaveIndex = ui->sbFixedWave->value();

    s.bFixDecay = ui->cbFixedDecay->isChecked();
    s.DecayTime = ui->ledDecayTime->text().toDouble();

    s.bOnlyVolume = ui->cbSkipByVolume->isChecked();
    s.Volume = ui->leSkipOutsideVolume->text();

    s.bOnlyMaterial = ui->cbSkipByMaterial->isChecked();
    s.Material = ui->leSkipOutsideMaterial->text();

    accept();
}

void ABombAdvancedDialog::on_pbCancel_clicked()
{
    reject();
}

void ABombAdvancedDialog::on_cobDirectionMode_currentIndexChanged(int index)
{
    ui->frNonIsotropic->setEnabled(index != 0);
    ui->fConeForPhotonGen->setEnabled(index == 2);
}

