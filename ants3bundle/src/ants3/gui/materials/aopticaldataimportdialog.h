#ifndef AOPTICALDATAIMPORTDIALOG_H
#define AOPTICALDATAIMPORTDIALOG_H

#include <QDialog>
#include <vector>

namespace Ui {
class AOpticalDataImportDialog;
}

class AOpticalDataImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AOpticalDataImportDialog(QWidget *parent = nullptr);
    ~AOpticalDataImportDialog();

    std::vector<std::pair<double,double>> Data;

private slots:
    void on_pbCancel_clicked();

    void on_pbCompute_clicked();

private:
    Ui::AOpticalDataImportDialog *ui;
};

#endif // AOPTICALDATAIMPORTDIALOG_H
