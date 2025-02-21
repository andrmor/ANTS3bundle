#include "arefractiveindeximportdialog.h"
#include "ui_arefractiveindeximportdialog.h"
#include "guitools.h"

ARefractiveIndexImportDialog::ARefractiveIndexImportDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::ARefractiveIndexImportDialog)
{
    ui->setupUi(this);
}

ARefractiveIndexImportDialog::~ARefractiveIndexImportDialog()
{
    delete ui;
}

void ARefractiveIndexImportDialog::on_pbCancel_clicked()
{
    reject();
}

void ARefractiveIndexImportDialog::on_pbConvert_clicked()
{
    QString errorTxt = "The text should conain pairs of wavelength and refractive index\n,separated by space or ',', one pair per line";
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

    Data.clear();
    const QStringList slAll = txt.split("\n", Qt::SkipEmptyParts);
    for (const QString & str : slAll)
    {
        const QString s = str.simplified();
        const QStringList sl = s.split(" ", Qt::SkipEmptyParts);
        if (sl.size() != 2) continue;

        double wave_nm = sl[0].toDouble() * waveFactor;
        double value   = sl[1].toDouble();

        Data.push_back({wave_nm, value});
    }

    if (Data.empty())
    {
        guitools::message("Conversion rejected all the lines!\n" + errorTxt, this);
        return;
    }
    accept();
}
