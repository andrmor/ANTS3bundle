#ifndef ABOMBADVANCEDDIALOG_H
#define ABOMBADVANCEDDIALOG_H

#include <QDialog>

namespace Ui {
class ABombAdvancedDialog;
}

class ABombAdvancedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ABombAdvancedDialog(QWidget *parent = nullptr);
    ~ABombAdvancedDialog();

private:
    Ui::ABombAdvancedDialog *ui;
};

#endif // ABOMBADVANCEDDIALOG_H
