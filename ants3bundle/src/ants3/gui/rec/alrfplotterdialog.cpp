#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "alrfplotter.h"
#include "guitools.h"

#include "lrmodel.h"

#include <QDebug>
#include <QDoubleValidator>

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
    delete ui;
}

void ALrfPlotterDialog::setPlotter(ALrfPlotter * plotter)
{
    Plotter = plotter;
    HaveData = !Plotter->DataSignals.empty();
}

void ALrfPlotterDialog::redraw()
{
    if (!Plotter)
    {
        qWarning() << "ALrfPlotterDialog: Plotter not provided";
        return;
    }

    Plotter->NumPointsInRadialGraph = ui->sbRadial_points->value();
    Plotter->NumPointsInXYGraph = ui->sbXY_points->value();

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

    updateVisibilityAndStatus();

    if (ui->tabwPlotType->currentIndex() == 0)
        makeRadialPlot(iSens);
    else
        makeXYPlot(iSens);
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

void ALrfPlotterDialog::makeXYPlot(int iSens)
{
    bool plotLrf   = ui->cbXY_lrf->isChecked();
    bool plotData  = ui->cbXY_data->isChecked();
    bool plotDiff  = ui->cbXY_diff->isChecked();

    Plotter->drawXY(iSens, plotLrf, plotData || plotDiff, plotDiff);
}

#include "alightresponsehub.h"
#include "lrf.h"
#include "lrfaxial3d.h"
#include "lrfxyz.h"
void ALrfPlotterDialog::updateVisibilityAndStatus()
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    ui->sbSensor->setToolTip( QString("Number of sensors in the response model: %0").arg(model->GetSensorCount()) );

    const int iSens = ui->sbSensor->value();
    LRF * lrf = model->GetLRF(iSens);

    bool haveZ = false;
    if      (dynamic_cast<LRFaxial3d*>(lrf)) haveZ = true;
    else if (dynamic_cast<LRFxyz*>(lrf))     haveZ = true;
    ui->frZ->setVisible(haveZ);

    ui->cbRadial_data->setEnabled(HaveData);
    ui->cbRadial_diff->setEnabled(HaveData);
    ui->cbXY_data->setEnabled(HaveData);
    ui->cbXY_diff->setEnabled(HaveData);
    ui->sbVerticalBins->setEnabled(HaveData);
    ui->frZrange->setEnabled(HaveData);
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

void ALrfPlotterDialog::on_tabwPlotType_currentChanged(int index)
{
    redraw();
}

void ALrfPlotterDialog::on_cbXY_data_clicked(bool checked)
{
    if (checked && ui->cbXY_diff->isChecked()) ui->cbXY_diff->setChecked(false);
    redraw();
}

void ALrfPlotterDialog::on_cbXY_diff_clicked(bool checked)
{
    if (checked && ui->cbXY_data->isChecked()) ui->cbXY_data->setChecked(false);
    redraw();
}

