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

    void on_cbFixWave_toggled(bool checked);

    void on_cbFixedDecay_toggled(bool checked);

    void on_cbSkipByVolume_toggled(bool checked);

    void on_cbSkipByMaterial_toggled(bool checked);

private:
    Ui::ABombAdvancedDialog *ui;

    QPixmap YellowCircle;
};

#endif // ABOMBADVANCEDDIALOG_H
