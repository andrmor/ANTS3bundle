#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "alrfplotter.h"
#include "lrmodel.h"

ALrfPlotterDialog::ALrfPlotterDialog(ALrfPlotter * plotter, QWidget * parent) :
    QDialog(parent), Plotter(plotter),
    ui(new Ui::ALrfPlotterDialog)
{
    ui->setupUi(this);

    //ui->pbClose->setDefault(false);
    //ui->pbRedraw->setDefault(true);
}

ALrfPlotterDialog::~ALrfPlotterDialog()
{
    qDebug() << "..destr for ALrfPlotterDialog";
    delete ui;
}

void ALrfPlotterDialog::triggerRedraw()
{
    QApplication::processEvents();
    on_pbRedraw_clicked();
    QApplication::processEvents();
}

void ALrfPlotterDialog::on_pbClose_clicked()
{
    reject();
}

void ALrfPlotterDialog::on_pbRedraw_clicked()
{
    if (ui->tabwPlotType->currentIndex() == 0)
    {
        makeRadialPlot();
    }
}

void ALrfPlotterDialog::makeRadialPlot()
{
    const int iSens = ui->sbSensor->value();

    if (ui->cbRadial_data->isChecked())
        Plotter->drawRadial_Data(iSens, ui->cbRadial_lrf->isChecked());
    else
        Plotter->drawRadial(iSens, ui->cbRadial_addNodes->isChecked());
}

void ALrfPlotterDialog::on_sbSensor_editingFinished()
{
    on_pbRedraw_clicked();
}

