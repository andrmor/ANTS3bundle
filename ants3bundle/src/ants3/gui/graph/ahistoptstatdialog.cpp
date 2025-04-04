#include "ahistoptstatdialog.h"
#include "ui_ahistoptstatdialog.h"

#include <vector>

#include "TStyle.h"

void collect_digits(int num, std::vector<int> & digits)
{
    if (num > 9) collect_digits(num / 10, digits);
    digits.insert(digits.begin(), num % 10);
}

AHistOptStatDialog::AHistOptStatDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::AHistOptStatDialog)
{
    ui->setupUi(this);
    setWindowTitle("Histogram stat box");

    int stat = gStyle->GetOptStat();

    std::vector<int> digits;
    collect_digits(stat, digits);
    qDebug() << stat;
    qDebug() << digits;

    for (int i = 0; i < digits.size(); i++)
    {
        int val = digits[i];
        switch (i)
        {
        case 0: ui->cbName->setChecked(val);       break;
        case 1: ui->cbEntries->setChecked(val);    break;
        case 2: ui->cbMean->setChecked(val);       ui->cbMeanError->setChecked(val > 1); break;
        case 3: ui->cbRMS->setChecked(val);        ui->cbRMSError->setChecked(val > 1); break;
        case 4: ui->cbUnderflows->setChecked(val); break;
        case 5: ui->cbOverflows->setChecked(val);  break;
        case 6: ui->cbIntegral->setChecked(val);   break;
        case 7: ui->cbSkewness->setChecked(val);   ui->cbSkewnessError->setChecked(val > 1); break;
        case 8: ui->cbKurtosis->setChecked(val);   ui->cbKurtosisError->setChecked(val > 1); break;
        default:;
        }
    }
}

AHistOptStatDialog::~AHistOptStatDialog()
{
    delete ui;
}

void AHistOptStatDialog::on_pbConfirm_clicked()
{
    int stat = 0;
    /*
    stat += ui->cbName->isChecked()       * 1;
    stat += ui->cbEntries->isChecked()    * 10;
    stat += ui->cbMean->isChecked()       * 100;
    stat += ui->cbRMS->isChecked()        * 1000;
    stat += ui->cbUnderflows->isChecked() * 10000;
    stat += ui->cbOverflows->isChecked()  * 100000;
    stat += ui->cbIntegral->isChecked()   * 1000000;
    stat += ui->cbSkewness->isChecked()   * 10000000;
    stat += ui->cbKurtosis->isChecked()   * 100000000;
    */

    std::vector<std::pair<bool, bool>> boxes;
    boxes.push_back({ui->cbName->isChecked(), false});
    boxes.push_back({ui->cbEntries->isChecked(), false});
    boxes.push_back({ui->cbMean->isChecked(), ui->cbMeanError->isChecked()});
    boxes.push_back({ui->cbRMS->isChecked(), ui->cbRMSError->isChecked()});
    boxes.push_back({ui->cbUnderflows->isChecked(), false});
    boxes.push_back({ui->cbOverflows->isChecked(), false});
    boxes.push_back({ui->cbIntegral->isChecked(), false});
    boxes.push_back({ui->cbSkewness->isChecked(), ui->cbSkewnessError->isChecked()});
    boxes.push_back({ui->cbKurtosis->isChecked(), ui->cbKurtosisError->isChecked()});
    size_t factor = 1;
    for (size_t i = 0; i < boxes.size(); i++)
    {
        int val = 0;
        if (boxes[i].first) val = 1 + boxes[i].second;
        stat += val * factor;
        factor *= 10;
    }

    gStyle->SetOptStat(stat);
    accept();
}

void AHistOptStatDialog::on_pbCancel_clicked()
{
    reject();
}

