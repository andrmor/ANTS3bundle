#include "aopticaldataimportdialog.h"
#include "ui_aopticaldataimportdialog.h"
#include "vformula.h"
#include "guitools.h"
#include "ajsontools.h"

AOpticalDataImportDialog::AOpticalDataImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AOpticalDataImportDialog)
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

    if (from >= to)
    {
        guitools::message("Error in the wavelength range", this);
        return;
    }

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

    VFormula p;

    std::vector<std::string> names;
    names.push_back("lambda");
    p.setVariableNames(names);

    bool ok = p.parse(formula.toLatin1().data());
    if (!ok)
    {
        guitools::message("Formula parsing error\n" + QString(p.ErrorString.data()));
        return;
    }

    ok = p.validate();
    if (!ok)
    {
        guitools::message("Formula validation error\n" + QString(p.ErrorString.data()));
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

    //qDebug() << Data;
    accept();
}

void AOpticalDataImportDialog::writeToJson(QJsonObject & json) const
{
    {
        QJsonObject js;
            js["From"] = ui->ledWaveFrom->text().toDouble();
            js["To"]   = ui->ledWaveTo->text().toDouble();
            js["Step"] = ui->ledWaveStep->text().toDouble();
        json["WavelengthRange"] = js;
    }

    json["Formula"] = ui->pteFormula->document()->toPlainText();

    QString txt;
    switch (ui->cobUnits->currentIndex())
    {
    case 0: txt = "nm"; break;
    case 1: txt = "um"; break;
    case 2: txt = "A"; break;
    }
    json["FormulaUnits"] = txt;
}

void AOpticalDataImportDialog::readFromJson(const QJsonObject & json)
{
    if (json.empty()) return;

    {
        QJsonObject js;
        jstools::parseJson(json, "WavelengthRange", js);
        double val;
        if (jstools::parseJson(js, "From", val)) ui->ledWaveFrom->setText(QString::number(val));
        if (jstools::parseJson(js, "To",   val)) ui->ledWaveTo->setText(QString::number(val));
        if (jstools::parseJson(js, "Step", val)) ui->ledWaveStep->setText(QString::number(val));
    }

    QString txt;
    if (jstools::parseJson(json, "Formula", txt)) ui->pteFormula->appendPlainText(txt);

    if (jstools::parseJson(json, "FormulaUnits", txt))
    {
        int index = 0;
        if      (txt == "um") index = 1;
        else if (txt == "A")  index = 2;
        ui->cobUnits->setCurrentIndex(index);
    }
}
