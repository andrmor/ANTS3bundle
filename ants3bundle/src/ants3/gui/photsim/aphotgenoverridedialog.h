#ifndef APHOTGENOVERRIDEDIALOG_H
#define APHOTGENOVERRIDEDIALOG_H

#include <QDialog>

namespace Ui {
class APhotGenOverrideDialog;
}

class APhotGenOverrideDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APhotGenOverrideDialog(QWidget * parent = nullptr);
    ~APhotGenOverrideDialog();

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();

    void on_cobDirectionMode_currentIndexChanged(int index);

    void on_cbFixWave_toggled(bool checked);
    void on_cbFixedDecay_toggled(bool checked);
    void on_pbFixedWavelengthInfo_clicked();
    void on_ledFixedWavelength_editingFinished();

private:
    Ui::APhotGenOverrideDialog * ui;

    QPixmap YellowCircle;

    void updateFixedWavelengthGui();
};

#endif // APHOTGENOVERRIDEDIALOG_H
