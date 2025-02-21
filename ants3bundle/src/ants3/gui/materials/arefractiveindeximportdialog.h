#ifndef AREFRACTIVEINDEXIMPORTDIALOG_H
#define AREFRACTIVEINDEXIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ARefractiveIndexImportDialog;
}

class ARefractiveIndexImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ARefractiveIndexImportDialog(QWidget *parent = nullptr);
    ~ARefractiveIndexImportDialog();

    std::vector<std::pair<double,double>> Data;

private slots:
    void on_pbConvert_clicked();
    void on_pbCancel_clicked();

private:
    Ui::ARefractiveIndexImportDialog *ui;
};

#endif // AREFRACTIVEINDEXIMPORTDIALOG_H
