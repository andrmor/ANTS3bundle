#ifndef AHISTOPTSTATDIALOG_H
#define AHISTOPTSTATDIALOG_H

#include <QDialog>

namespace Ui {
class AHistOptStatDialog;
}

class AHistOptStatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AHistOptStatDialog(QWidget *parent = nullptr);
    ~AHistOptStatDialog();

private slots:
    void on_pbConfirm_clicked();

    void on_pbCancel_clicked();

private:
    Ui::AHistOptStatDialog *ui;
};

#endif // AHISTOPTSTATDIALOG_H
