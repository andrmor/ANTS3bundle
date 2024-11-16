#ifndef ABOMBADVANCEDDIALOG_H
#define ABOMBADVANCEDDIALOG_H

#include <QDialog>

namespace Ui {
class ABombAdvancedDialog;
}

// !!!***
// check volume * mat exist
// direction vector normalization!

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
    void on_cbFixWave_toggled(bool checked);
    void on_cbFixedDecay_toggled(bool checked);
    void on_cbSkipByVolume_toggled(bool checked);
    void on_cbSkipByMaterial_toggled(bool checked);
    void on_pbFixedWavelengthInfo_clicked();

    void on_ledFixedWavelength_editingFinished();

private:
    Ui::ABombAdvancedDialog *ui;

    QPixmap YellowCircle;

    void updateFixedWavelengthGui();
};

#endif // ABOMBADVANCEDDIALOG_H
