#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "alrfplotter.h"
#include "guitools.h"
#include "alightresponsehub.h"

#include "lrmodel.h"
#include "lrf.h"
#include "lrfaxial.h"
#include "lrfaxial3d.h"
#include "lrfxyz.h"

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
    foreach(QLineEdit * w, list)
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

    Plotter->NumPointsInRadialGraph = ui->sbLRFpoints->value();
    Plotter->NumPointsInXYGraph = ui->sbLRFpoints->value();

    Plotter->UseFixedVertical = ui->cbVerticalFixMin->isChecked();
    Plotter->VerticalMin      = ui->ledVerticalFixMin->text().toDouble();
    Plotter->VerticalMax      = ui->ledVerticalFixMax->text().toDouble();
    Plotter->VerticalNumBins  = ui->sbVerticalBins->value();

    Plotter->UseFixedRange = ui->cbRangeFixMin->isChecked();
    Plotter->RangeMin      = ui->ledRangeFixMin->text().toDouble();
    Plotter->RangeMax      = ui->ledRangeFixMax->text().toDouble();
    Plotter->XDataBins     = ui->sbDataBins->value();
    Plotter->XDataBins     = ui->sbDataBins->value();

    const int iSens = ui->sbSensor->value();
    const int numSens = Plotter->countSensors();
    if (numSens == 0)
    {
        guitools::message("There are no light sensors in the current configuration");
        return;
    }
    if (iSens < 0 || iSens > numSens)
    {
        ui->sbSensor->setValue(0);
        guitools::message("Invalid sensor index, should be 0.." + QString::number(numSens-1));
        return;
    }

    updateVisibilityAndStatus();

    if (ui->cobPlotType->currentIndex() == 0)
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
    bool plotLrf   = ui->cbLrf->isChecked();
    bool plotNodes = ui->cbRadial_addNodes->isChecked();
    bool plotData  = ui->cbData->isChecked();
    bool plotDiff  = ui->cbDiff->isChecked();

    Plotter->drawRadial(iSens, plotLrf, plotNodes, plotData || plotDiff, plotDiff);
}

void ALrfPlotterDialog::makeXYPlot(int iSens)
{
    bool plotLrf   = ui->cbLrf->isChecked();
    bool plotData  = ui->cbData->isChecked();
    bool plotDiff  = ui->cbDiff->isChecked();

    Plotter->drawXY(iSens, plotLrf, plotData || plotDiff, plotDiff);
}

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

    ui->cbData->setEnabled(HaveData);
    ui->cbDiff->setEnabled(HaveData);
    ui->sbVerticalBins->setEnabled(HaveData);
    ui->frZrange->setEnabled(HaveData);

    if (HaveData)
    {
        bool dataOrDiff = ui->cbData->isChecked() || ui->cbDiff->isChecked();
        ui->sbDataBins->setEnabled(dataOrDiff);
        ui->sbVerticalBins->setEnabled(dataOrDiff);
        ui->ledZrange->setEnabled(dataOrDiff);
    }

    bool showLrf = ui->cbLrf->isChecked();
    ui->cbRadial_addNodes->setEnabled(showLrf);
    ui->sbLRFpoints->setEnabled(showLrf);

    bool showProfiles = false;
    if (ui->cobPlotType->currentIndex() == 0)
        if (!dynamic_cast<LRFaxial*>(lrf))
            showProfiles = true;
    ui->sbNumProfiles->setEnabled(showProfiles);
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

void ALrfPlotterDialog::on_cbData_clicked(bool checked)
{
    if (checked && ui->cbDiff->isChecked()) ui->cbDiff->setChecked(false);
    redraw();
}

void ALrfPlotterDialog::on_cbDiff_clicked(bool checked)
{
    if (checked && ui->cbData->isChecked()) ui->cbData->setChecked(false);
    redraw();
}
