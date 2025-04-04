#ifndef ATEXTPAVEDIALOG_H
#define ATEXTPAVEDIALOG_H

#include <QDialog>

namespace Ui {
class ATextPaveDialog;
}

class TPaveText;

class ATextPaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ATextPaveDialog(TPaveText & Pave, QWidget *parent = 0);
    ~ATextPaveDialog();

    void updateGui();

    QString CurrentText;

private slots:
    void on_pbDummy_clicked();
    void on_pbConfirm_clicked();
    void on_pbTextAttributes_clicked();
    void on_pbConfigureFrame_clicked();

private:
    TPaveText & Pave;
    Ui::ATextPaveDialog *ui;

private slots:
    void updatePave();

    void on_ledX0_editingFinished();

    void on_ledX1_editingFinished();

    void on_ledY0_editingFinished();

    void on_ledY1_editingFinished();

    void on_pbExport_clicked();

    void on_pbImport_clicked();

    void on_pte_textChanged();

signals:
    void requestRedraw();
    void textChanged(QString txt);
};

#endif // ATEXTPAVEDIALOG_H
