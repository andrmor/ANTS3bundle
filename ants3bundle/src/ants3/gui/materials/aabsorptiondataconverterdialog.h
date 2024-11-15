#ifndef AABSORPTIONDATACONVERTERDIALOG_H
#define AABSORPTIONDATACONVERTERDIALOG_H

#include <QDialog>

#include <vector>

namespace Ui {
class AAbsorptionDataConverterDialog;
}

class AAbsorptionDataConverterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AAbsorptionDataConverterDialog(QWidget *parent = nullptr);
    ~AAbsorptionDataConverterDialog();

    std::vector<std::pair<double,double>> Data;

private slots:
    void on_pbCancel_clicked();
    void on_pbConvert_clicked();

private:
    Ui::AAbsorptionDataConverterDialog *ui;
};

#endif // AABSORPTIONDATACONVERTERDIALOG_H
