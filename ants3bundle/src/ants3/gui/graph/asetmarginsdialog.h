#ifndef ASETMARGINSDIALOG_H
#define ASETMARGINSDIALOG_H

#include <QDialog>

namespace Ui {
class ASetMarginsDialog;
}

class ASetMarginsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ASetMarginsDialog(QWidget *parent = nullptr);
    ~ASetMarginsDialog();

private slots:
    void on_pbAccept_clicked();

    void on_pbCancel_clicked();

    void on_pbReset_clicked();

private:
    Ui::ASetMarginsDialog *ui;
};

#endif // ASETMARGINSDIALOG_H
