#include "a3photsimwin.h"
#include "aphotsimsettings.h"
#include "ui_a3photsimwin.h"
#include "a3config.h"
#include "guitools.h"

#include <QDebug>

A3PhotSimWin::A3PhotSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(APhotSimSettings::getInstance()),
    ui(new Ui::A3PhotSimWin)
{
    ui->setupUi(this);

    connect(&A3Config::getInstance(), &A3Config::requestUpdatePhotSimGui, this, &A3PhotSimWin::updateGui);

    QList<QPushButton*> listDummyButtons = this->findChildren<QPushButton*>();
    for (QPushButton * pb : qAsConst(listDummyButtons))
        if (pb->objectName().startsWith("pbd"))
            pb->setVisible(false);

    updateGui();
}

A3PhotSimWin::~A3PhotSimWin()
{
    delete ui;
}

void A3PhotSimWin::updateGui()
{
    int index;
    switch (SimSet.SimType)
    {
    default:
    case EPhotSimType::PhotonBombs       : index = 0; break;
    case EPhotSimType::FromEnergyDepo    : index = 1; break;
    case EPhotSimType::IndividualPhotons : index = 2; break;
    case EPhotSimType::FromLRFs          : index = 3; break;
    }
    ui->cobSimType->setCurrentIndex(index);

    updatePhotBombGui();

    updateGeneralSettingsGui();
}

void A3PhotSimWin::updatePhotBombGui()
{
    {
        int index;
        switch (SimSet.BombSet.PhotonNumberMode)
        {
        default:
        case EBombPhNumber::Constant : index = 0; break;
        case EBombPhNumber::Poisson  : index = 1; break;
        case EBombPhNumber::Uniform  : index = 2; break;
        case EBombPhNumber::Normal   : index = 3; break;
        case EBombPhNumber::Custom   : index = 4; break;
        }
        ui->cobScanNumPhotonsMode->setCurrentIndex(index);
    }

    {
        int index;
        switch (SimSet.BombSet.GenerationMode)
        {
        default:
        case EBombGen::Single : index = 0; break;
        case EBombGen::Grid   : index = 1; break;
        case EBombGen::Flood  : index = 2; break;
        case EBombGen::File   : index = 3; break;
        case EBombGen::Script : index = 4; break;
        }
        ui->cobNodeGenerationMode->setCurrentIndex(index);
    }

    ui->ledSingleX->setText(QString::number(SimSet.BombSet.Position[0]));
    ui->ledSingleY->setText(QString::number(SimSet.BombSet.Position[1]));
    ui->ledSingleZ->setText(QString::number(SimSet.BombSet.Position[2]));
}

void A3PhotSimWin::updateGeneralSettingsGui()
{
    ui->twGeneralOption->setEnabled(SimSet.SimType != EPhotSimType::FromLRFs);

    ui->cbWaveResolved->setChecked(SimSet.WaveSet.Enabled);
    ui->fWaveOptions->setEnabled(SimSet.WaveSet.Enabled);
    ui->ledWaveFrom->setText(QString::number(SimSet.WaveSet.From));
    ui->ledWaveTo  ->setText(QString::number(SimSet.WaveSet.To));
    ui->ledWaveStep->setText(QString::number(SimSet.WaveSet.Step));
    ui->labWaveNodes->setText(QString::number(SimSet.WaveSet.countNodes()));

    ui->sbMaxNumbPhTransitions->setValue(SimSet.OptSet.MaxPhotonTransitions);
    ui->cbRndCheckBeforeTrack->setChecked(SimSet.OptSet.CheckQeBeforeTracking);
}

void A3PhotSimWin::on_pbdWave_clicked()
{
    if (ui->ledWaveStep->text().toDouble() <= 0)
    {
        updateGeneralSettingsGui();
        guitools::message("Step should be positive!", this);
        return;
    }

    if (ui->ledWaveFrom->text().toDouble() > ui->ledWaveTo  ->text().toDouble())
    {
        updateGeneralSettingsGui();
        guitools::message("'From' should not be larger than 'To'!", this);
        return;
    }

    storeGeneralSettings();
    updateGeneralSettingsGui();
}

void A3PhotSimWin::storeGeneralSettings()
{
    SimSet.WaveSet.Enabled = ui->cbWaveResolved->isChecked();
    SimSet.WaveSet.From    = ui->ledWaveFrom->text().toDouble();
    SimSet.WaveSet.To      = ui->ledWaveTo  ->text().toDouble();
    SimSet.WaveSet.Step    = ui->ledWaveStep->text().toDouble();
}

void A3PhotSimWin::on_sbMaxNumbPhTransitions_editingFinished()
{
    SimSet.OptSet.MaxPhotonTransitions  = ui->sbMaxNumbPhTransitions->value();
}

void A3PhotSimWin::on_cbRndCheckBeforeTrack_clicked()
{
    SimSet.OptSet.CheckQeBeforeTracking = ui->cbRndCheckBeforeTrack->isChecked();
}

void A3PhotSimWin::on_pbQEacceleratorHelp_clicked()
{
    guitools::message("TODO", this);
}

void A3PhotSimWin::on_cobSimType_activated(int index)
{
    switch (index)
    {
    default:
    case 0 : SimSet.SimType = EPhotSimType::PhotonBombs;       break;
    case 1 : SimSet.SimType = EPhotSimType::FromEnergyDepo;    break;
    case 2 : SimSet.SimType = EPhotSimType::IndividualPhotons; break;
    case 3 : SimSet.SimType = EPhotSimType::FromLRFs;          break;
    }
}

void A3PhotSimWin::on_cobScanNumPhotonsMode_activated(int index)
{
    switch (index)
    {
    default:
    case 0: SimSet.BombSet.PhotonNumberMode = EBombPhNumber::Constant; break;
    case 1: SimSet.BombSet.PhotonNumberMode = EBombPhNumber::Poisson;  break;
    case 2: SimSet.BombSet.PhotonNumberMode = EBombPhNumber::Uniform;  break;
    case 3: SimSet.BombSet.PhotonNumberMode = EBombPhNumber::Normal;   break;
    case 4: SimSet.BombSet.PhotonNumberMode = EBombPhNumber::Custom;   break;
    }
}

void A3PhotSimWin::on_cobNodeGenerationMode_activated(int index)
{
    switch (index)
    {
    default:
    case 0 : SimSet.BombSet.GenerationMode = EBombGen::Single; break;
    case 1 : SimSet.BombSet.GenerationMode = EBombGen::Grid;   break;
    case 2 : SimSet.BombSet.GenerationMode = EBombGen::Flood;  break;
    case 3 : SimSet.BombSet.GenerationMode = EBombGen::File;   break;
    case 4 : SimSet.BombSet.GenerationMode = EBombGen::Script; break;
    }
}

void A3PhotSimWin::on_ledSingleX_editingFinished()
{
    SimSet.BombSet.Position[0] = ui->ledSingleX->text().toDouble();
}

void A3PhotSimWin::on_ledSingleY_editingFinished()
{
    SimSet.BombSet.Position[1] = ui->ledSingleY->text().toDouble();
}


void A3PhotSimWin::on_ledSingleZ_editingFinished()
{
    SimSet.BombSet.Position[2] = ui->ledSingleZ->text().toDouble();
}

