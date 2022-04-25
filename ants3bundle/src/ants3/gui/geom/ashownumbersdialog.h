#ifndef ASHOWNUMBERSDIALOG_H
#define ASHOWNUMBERSDIALOG_H

#include <QDialog>

namespace Ui {
class AShowNumbersDialog;
}

class AGeometryWindow;

class AShowNumbersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AShowNumbersDialog(AGeometryWindow & gw);
    ~AShowNumbersDialog();

public slots:
    int exec() override;

private slots:
    void on_pbSensorIndex_clicked();
    void on_pbSensorModel_clicked();
    void on_ledSizeSensor_editingFinished();

    void on_pbOptMonIndex_clicked();
    void on_pbPartMonIndex_clicked();
    void on_pbOptMonHits_clicked();
    void on_pbPartMonHits_clicked();
    void on_ledSizeMonitor_editingFinished();

    void on_pbCalIndex_clicked();
    void on_pbCalTotals_clicked();
    void on_ledSizeCalorimeter_editingFinished();

private:
    AGeometryWindow & GW;
    Ui::AShowNumbersDialog *ui;
};

#endif // ASHOWNUMBERSDIALOG_H
