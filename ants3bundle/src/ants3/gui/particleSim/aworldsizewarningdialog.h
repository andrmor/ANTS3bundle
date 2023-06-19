#ifndef AWORLDSIZEWARNINGDIALOG_H
#define AWORLDSIZEWARNINGDIALOG_H

#include <QDialog>

namespace Ui {
class AWorldSizeWarningDialog;
}

class AWorldSizeWarningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AWorldSizeWarningDialog(double xy, double z, QWidget *parent = nullptr);
    ~AWorldSizeWarningDialog();

    enum EResult{OK, Goto, DontBother};
    EResult Result = OK;

private slots:
    void on_pbOK_clicked();
    void on_pbGoto_clicked();
    void on_pbDont_clicked();

private:
    Ui::AWorldSizeWarningDialog *ui;
};

#endif // AWORLDSIZEWARNINGDIALOG_H
