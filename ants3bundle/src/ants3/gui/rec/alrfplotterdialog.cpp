#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "alrfplotter.h"
#include "guitools.h"
#include "QDoubleValidator"
#include "lrmodel.h"

#include <QDebug>

ALrfPlotterDialog::ALrfPlotterDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::ALrfPlotterDialog)
{
    ui->setupUi(this);

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    foreach(QLineEdit *w, list)
        if (w->objectName().startsWith("led")) w->setValidator(dv);

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

    Plotter->FixedVerticalMin = ui->cbVerticalFixMin->isChecked();
    Plotter->VerticalMin = ui->ledVerticalFixMin->text().toDouble();
    Plotter->FixedVerticalMax = ui->cbVerticalFixMax->isChecked();
    Plotter->VerticalMax = ui->ledVerticalFixMax->text().toDouble();
    Plotter->VerticalNumBins = ui->sbVerticalBins->value();

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
    bool plotLrf   = ui->cbRadial_lrf->isChecked();
    bool plotNodes = ui->cbRadial_addNodes->isChecked();
    bool plotData  = ui->cbRadial_data->isChecked();
    bool plotDiff  = ui->cbRadial_diff->isChecked();

    Plotter->drawRadial(iSens, plotLrf, plotNodes, plotData || plotDiff, plotDiff);
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

void ALrfPlotterDialog::on_cbRadial_data_clicked(bool checked)
{
    if (checked && ui->cbRadial_diff->isChecked()) ui->cbRadial_diff->setChecked(false);
    redraw();
}

void ALrfPlotterDialog::on_cbRadial_diff_clicked(bool checked)
{
    if (checked && ui->cbRadial_data->isChecked()) ui->cbRadial_data->setChecked(false);
    redraw();
}

