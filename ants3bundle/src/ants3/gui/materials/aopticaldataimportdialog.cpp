#include "aopticaldataimportdialog.h"
#include "ui_aopticaldataimportdialog.h"
#include "vformula.h"
#include "guitools.h"

AOpticalDataImportDialog::AOpticalDataImportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AOpticalDataImportDialog)
{
    ui->setupUi(this);
}

AOpticalDataImportDialog::~AOpticalDataImportDialog()
{
    delete ui;
}

void AOpticalDataImportDialog::on_pbCancel_clicked()
{
    reject();
}

void AOpticalDataImportDialog::on_pbCompute_clicked()
{
    QString formula = ui->pteFormula->document()->toPlainText();
    if (formula.isEmpty())
    {
        guitools::message("Introduce the formula", this);
        return;
    }

    Data.clear();
    double from = ui->ledWaveFrom->text().toDouble();
    double to   = ui->ledWaveTo->text().toDouble();
    double step = ui->ledWaveStep->text().toDouble();

    double wave = from;
    do
    {
        Data.push_back({wave, 0});
        wave += step;
    }
    while (wave < to);

    double waveFactor = 1.0;
    if      (ui->cobUnits->currentIndex() == 1) waveFactor = 0.001;
    else if (ui->cobUnits->currentIndex() == 2) waveFactor = 0.1;

    VFormula p1;

    std::vector<std::string> names;
    names.push_back("lambda");
    p1.setVariableNames(names);

    bool ok = p1.parse(formula.toLatin1().data());
    if (!ok)
    {
        guitools::message("VFormula parse error!\n" + QString(p1.ErrorString.data()));
        return;
    }

    VFormula p(p1);

    //std::cout << "\n----------Map------------\n";
    //p.printCVMap();
    //std::cout << "\n---------Program---------\n";
    //p.printPrg();

    ok = p.validate();
    if (!ok)
    {
        guitools::message("VFormula validation error!\n" + QString(p.ErrorString.data()));
        return;
    }

    std::vector<double> values(1);
    for (auto & pair : Data)
    {
        values[0] = pair.first * waveFactor;
        double res = p.eval(values);
        if (!p.ErrorString.empty())
        {
            guitools::message("VFormula eval error!\n" + QString(p.ErrorString.data()));
            return;
        }
        pair.second = res;
    }

    qDebug() << Data;
    accept();
}

