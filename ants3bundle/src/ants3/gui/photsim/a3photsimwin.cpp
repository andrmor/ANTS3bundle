#include "a3photsimwin.h"
#include "asimsettingshub.h"
#include "ui_a3photsimwin.h"

A3PhotSimWin::A3PhotSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(ASimSettingsHub::getInstance().PhotSimSet),
    ui(new Ui::A3PhotSimWin)
{
    ui->setupUi(this);

    updateGui();
}

A3PhotSimWin::~A3PhotSimWin()
{
    delete ui;
}

void A3PhotSimWin::updateGui()
{
    int iTab = static_cast<int>(SimSet.SimType);
    ui->swSimType->setCurrentIndex(iTab);

    updatePhotBombGui();

    updateGeneralOptions();
}

void A3PhotSimWin::updatePhotBombGui()
{

}

void A3PhotSimWin::updateGeneralOptions()
{
    ui->twGeneralOption->setEnabled(SimSet.SimType != APhotSinTypeEnum::FromLRFs);

    ui->cbWaveResolved->setChecked(SimSet.WaveSet.Enabled);
    ui->fWaveOptions->setEnabled(SimSet.WaveSet.Enabled);
    ui->ledWaveFrom->setText(QString::number(SimSet.WaveSet.From));
    ui->ledWaveTo  ->setText(QString::number(SimSet.WaveSet.To));
    ui->ledWaveStep->setText(QString::number(SimSet.WaveSet.Step));
    ui->labWaveNodes->setText(QString::number(SimSet.WaveSet.countNodes()));

}
