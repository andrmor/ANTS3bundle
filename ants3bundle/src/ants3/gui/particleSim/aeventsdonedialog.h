#ifndef AEVENTSDONEDIALOG_H
#define AEVENTSDONEDIALOG_H

#include <QDialog>

namespace Ui {
class AEventsDoneDialog;
}

class AEventsDoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AEventsDoneDialog(QWidget *parent = nullptr);
    ~AEventsDoneDialog();

public slots:
    void onProgressReported(int numEventsDone);

private slots:
    void on_pbAbort_clicked();

private:
    Ui::AEventsDoneDialog *ui;
};

#endif // AEVENTSDONEDIALOG_H
