#include "abombadvanceddialog.h"
#include "ui_abombadvanceddialog.h"
#include "aphotonsimhub.h"
#include "guitools.h"
#include "aphotonsimhub.h"

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
    ui->ledFixedWavelength->setText(QString::number(s.FixedWavelength));
    updateFixedWavelengthGui();

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
    s.FixedWavelength = ui->ledFixedWavelength->text().toDouble();

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

void ABombAdvancedDialog::on_pbFixedWavelengthInfo_clicked()
{
    guitools::message("If not checked, the wavelength of the generated photons is defined by the emission spectrum of the material at the emission position.\n"
                      "\nIn case the emission spectrum is not defined, the photons are generated with waveindex of -1, and non-wavelength resolved properties of all materials is used in tracking of this photon\n"
                      "\nIf the slected wavelength is outside of the configured wavelength range, the generated photons will have waveindex of -1", this);
}

void ABombAdvancedDialog::updateFixedWavelengthGui()
{
    const APhotonSimHub & SimSet = APhotonSimHub::getConstInstance();
    const AWaveResSettings & WaveSet = SimSet.Settings.WaveSet;

    ui->labNotWavelengthResolved->setVisible(!WaveSet.Enabled);
    ui->frFixedWavelength->setEnabled(WaveSet.Enabled);

    const double wave = ui->ledFixedWavelength->text().toDouble();
    const int iwave = WaveSet.toIndex(wave);
    ui->labFixedWaveIndex->setText(QString::number(iwave));
}

void ABombAdvancedDialog::on_ledFixedWavelength_editingFinished()
{
    updateFixedWavelengthGui();
}
