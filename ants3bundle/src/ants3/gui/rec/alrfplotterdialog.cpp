#include "alrfplotterdialog.h"
#include "ui_alrfplotterdialog.h"
#include "lrmodel.h"

ALrfPlotterDialog::ALrfPlotterDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::ALrfPlotterDialog)
{
    ui->setupUi(this);



    //ui->pbClose->setDefault(false);
    ui->pbRedraw->setDefault(true);
}

ALrfPlotterDialog::~ALrfPlotterDialog()
{
    delete ui;
}

void ALrfPlotterDialog::setModel(LRModel * model)
{
    delete Model; Model = new LRModel(*model);
}

void ALrfPlotterDialog::setData(const std::vector<std::vector<double>> & sensSignals, const std::vector<std::array<double, 4>> & xyze)
{
    Signals = sensSignals;
    XYZE = xyze;
}

void ALrfPlotterDialog::on_pbClose_clicked()
{
    accept();
}

void ALrfPlotterDialog::on_pbRedraw_clicked()
{

}

