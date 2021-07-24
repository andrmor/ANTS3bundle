#include "a3photsimwin.h"
#include "asimsettingshub.h"
#include "ui_a3photsimwin.h"
#include "guitools.h"

#include <QDebug>

A3PhotSimWin::A3PhotSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(ASimSettingsHub::getInstance().PhotSimSet),
    ui(new Ui::A3PhotSimWin)
{
    ui->setupUi(this);

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
    int iTab = static_cast<int>(SimSet.SimType);
    ui->swSimType->setCurrentIndex(iTab);

    updatePhotBombGui();

    updateGeneralGui();
}

void A3PhotSimWin::updatePhotBombGui()
{

}

void A3PhotSimWin::updateGeneralGui()
{
    ui->twGeneralOption->setEnabled(SimSet.SimType != APhotSinTypeEnum::FromLRFs);

    ui->cbWaveResolved->setChecked(SimSet.WaveSet.Enabled);
    ui->fWaveOptions->setEnabled(SimSet.WaveSet.Enabled);
    ui->ledWaveFrom->setText(QString::number(SimSet.WaveSet.From));
    ui->ledWaveTo  ->setText(QString::number(SimSet.WaveSet.To));
    ui->ledWaveStep->setText(QString::number(SimSet.WaveSet.Step));
    ui->labWaveNodes->setText(QString::number(SimSet.WaveSet.countNodes()));

}

void A3PhotSimWin::on_pbdWave_clicked()
{
    if (ui->ledWaveStep->text().toDouble() <= 0)
    {
        updateGeneralGui();
        guitools::message("Step should be positive!", this);
        return;
    }

    if (ui->ledWaveFrom->text().toDouble() > ui->ledWaveTo  ->text().toDouble())
    {
        updateGeneralGui();
        guitools::message("'From' should not be larger than 'To'!", this);
        return;
    }

    storeGeneral();
    updateGeneralGui();  // temprary! ***!!!
}

void A3PhotSimWin::storeGeneral()
{
    SimSet.WaveSet.Enabled = ui->cbWaveResolved->isChecked();
    SimSet.WaveSet.From    = ui->ledWaveFrom->text().toDouble();
    SimSet.WaveSet.To      = ui->ledWaveTo  ->text().toDouble();
    SimSet.WaveSet.Step    = ui->ledWaveStep->text().toDouble();
}
