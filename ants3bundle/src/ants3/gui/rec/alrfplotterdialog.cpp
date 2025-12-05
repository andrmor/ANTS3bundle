#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "alrfplotter.h"
#include "guitools.h"
#include "lrmodel.h"

#include <QDebug>

ALrfPlotterDialog::ALrfPlotterDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::ALrfPlotterDialog)
{
    ui->setupUi(this);

    ui->pbRedraw->setDefault(true);
}

ALrfPlotterDialog::~ALrfPlotterDialog()
{
    qDebug() << "..destr for ALrfPlotterDialog";
    delete ui;
}

void ALrfPlotterDialog::redraw()
{
    if (!Plotter)
    {
        qWarning() << "Plotter is not configured yet";
        return;
    }

    const int iSens = ui->sbSensor->value();
    const int numSens = Plotter->countSensors();
    if (numSens == 0)
    {
        guitools::message("There are no light sensors in the current configuration");
        return;
    }
    if (iSens < 0 || iSens > numSens)
    {
        guitools::message("Invalid sensor index, should be 0.." + QString::number(numSens-1));
        return;
    }

    if (ui->tabwPlotType->currentIndex() == 0)
    {
        makeRadialPlot(iSens);
    }
}

void ALrfPlotterDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setAttribute(Qt::WA_Moved); // if this attribute is set, the dialog keeps its prior position when re-opened
}

void ALrfPlotterDialog::on_pbClose_clicked()
{
    setVisible(false);
}

void ALrfPlotterDialog::on_pbRedraw_clicked()
{
    redraw();
}

void ALrfPlotterDialog::makeRadialPlot(int iSens)
{
    if (ui->cbRadial_data->isChecked())
        Plotter->drawRadial_Data(iSens, ui->cbRadial_lrf->isChecked());
    else
        Plotter->drawRadial(iSens, ui->cbRadial_addNodes->isChecked());
}

void ALrfPlotterDialog::on_sbSensor_editingFinished()
{
    redraw();
}

void ALrfPlotterDialog::on_pbPrevious_clicked()
{
    int val = ui->sbSensor->value();
    if (val > 0)
    {
        ui->sbSensor->setValue(val - 1);
        redraw();
    }
}

void ALrfPlotterDialog::on_pbNext_clicked()
{
    int val = ui->sbSensor->value();
    const int numSens = Plotter->countSensors();
    if (val < numSens - 2)
    {
        ui->sbSensor->setValue(val + 1);
        redraw();
    }
}

