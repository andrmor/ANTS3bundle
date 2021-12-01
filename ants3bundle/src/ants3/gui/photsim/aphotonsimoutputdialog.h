#ifndef APHOTONSIMOUTPUTDIALOG_H
#define APHOTONSIMOUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class APhotonSimOutputDialog;
}

class APhotonSimOutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APhotonSimOutputDialog(QWidget *parent = nullptr);
    ~APhotonSimOutputDialog();

private slots:
    void on_pbAccept_clicked();
    void on_pbChangeDir_clicked();

private:
    Ui::APhotonSimOutputDialog *ui;
};

#endif // APHOTONSIMOUTPUTDIALOG_H
