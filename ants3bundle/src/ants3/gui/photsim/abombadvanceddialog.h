#ifndef ABOMBADVANCEDDIALOG_H
#define ABOMBADVANCEDDIALOG_H

#include <QDialog>

namespace Ui {
class ABombAdvancedDialog;
}

// !!!***
// check volume * mat exist; check wave index
// direction vector normalization!
// wavelength indication!

class ABombAdvancedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ABombAdvancedDialog(QWidget *parent = nullptr);
    ~ABombAdvancedDialog();

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();

    void on_cobDirectionMode_currentIndexChanged(int index);

private:
    Ui::ABombAdvancedDialog *ui;
};

#endif // ABOMBADVANCEDDIALOG_H
