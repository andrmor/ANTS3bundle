#ifndef APHOTONSIMOUTPUTDIALOG_H
#define APHOTONSIMOUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class APhotonSimOutputDialog;
}

class APhotonLogSettingsForm;

class APhotonSimOutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APhotonSimOutputDialog(QWidget * parent = nullptr);
    ~APhotonSimOutputDialog();

private slots:
    void on_pbAccept_clicked();
    void on_pbChangeDir_clicked();

    void on_cbPhotonLog_toggled(bool checked);

    void on_pbChangeDir_customContextMenuRequested(const QPoint &pos);

private:
    Ui::APhotonSimOutputDialog * ui;
    APhotonLogSettingsForm * PhotonLog = nullptr;
};

#endif // APHOTONSIMOUTPUTDIALOG_H
