#include "abombadvanceddialog.h"
#include "ui_abombadvanceddialog.h"
#include "aphotonsimhub.h"
#include "guitools.h"

ABombAdvancedDialog::ABombAdvancedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ABombAdvancedDialog)
{
    ui->setupUi(this);

    YellowCircle = guitools::createColorCirclePixmap({15,15}, Qt::yellow);

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

    ui->twAdvSimOpt->setTabIcon(0, (index == 0 ? QIcon() : YellowCircle));
}

void ABombAdvancedDialog::on_cbFixWave_toggled(bool checked)
{
    ui->twAdvSimOpt->setTabIcon(1, (checked ? YellowCircle : QIcon()));
}

void ABombAdvancedDialog::on_cbFixedDecay_toggled(bool checked)
{
    ui->twAdvSimOpt->setTabIcon(2, (checked ? YellowCircle : QIcon()));
}

void ABombAdvancedDialog::on_cbSkipByVolume_toggled(bool)
{
    bool flag = ui->cbSkipByVolume->isChecked() || ui->cbSkipByMaterial->isChecked();
    ui->twAdvSimOpt->setTabIcon(3, (flag ? YellowCircle : QIcon()));
}

void ABombAdvancedDialog::on_cbSkipByMaterial_toggled(bool)
{
    bool flag = ui->cbSkipByVolume->isChecked() || ui->cbSkipByMaterial->isChecked();
    ui->twAdvSimOpt->setTabIcon(3, (flag ? YellowCircle : QIcon()));
}

