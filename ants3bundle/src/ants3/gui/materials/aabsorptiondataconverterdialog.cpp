#include "aabsorptiondataconverterdialog.h"
#include "ui_aabsorptiondataconverterdialog.h"
#include "guitools.h"

AAbsorptionDataConverterDialog::AAbsorptionDataConverterDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::AAbsorptionDataConverterDialog)
{
    ui->setupUi(this);
}

AAbsorptionDataConverterDialog::~AAbsorptionDataConverterDialog()
{
    delete ui;
}

void AAbsorptionDataConverterDialog::on_pbCancel_clicked()
{
    reject();
}

void AAbsorptionDataConverterDialog::on_pbConvert_clicked()
{
    QString errorTxt = "The text should conain pairs of wavelength and absorption or extinction coefficients\n,separated by space or ',', one pair per line";
    QString txt = ui->pteTable->document()->toPlainText();
    if (txt.isEmpty())
    {
        guitools::message(errorTxt, this);
        return;
    }
    txt.replace(","," ");

    double waveFactor = 1.0;
    if      (ui->cobWavelengthUnits->currentIndex() == 1) waveFactor = 1000;
    else if (ui->cobWavelengthUnits->currentIndex() == 2) waveFactor = 0.1;

    int valueType = ui->cobWhat->currentIndex(); // 0 - abs mm-1, 1 - abs cm-1, 2 - extinction

    Data.clear();
    const QStringList slAll = txt.split("\n", Qt::SkipEmptyParts);
    for (const QString & str : slAll)
    {
        const QString s = str.simplified();
        const QStringList sl = s.split(" ", Qt::SkipEmptyParts);
        if (sl.size() != 2) continue;

        double wave_nm = sl[0].toDouble() * waveFactor;
        double value   = sl[1].toDouble();
        double abs;
        switch (valueType)
        {
        case 0: abs = value; break;
        case 1: abs = 0.1 * value; break;
        case 2: abs = 4.0 * 3.1415926 * value / (wave_nm * 1e-9) * 0.001; // 4 Pi k / lambda, [mm-1]
        }

        Data.push_back({wave_nm, abs});
    }

    if (Data.empty())
    {
        guitools::message("Conversion rejected all the lines!\n" + errorTxt, this);
        return;
    }
    accept();
}

