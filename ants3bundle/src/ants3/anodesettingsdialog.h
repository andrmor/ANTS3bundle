#ifndef ANODESETTINGSDIALOG_H
#define ANODESETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ANodeSettingsDialog;
}

class ANodeSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ANodeSettingsDialog(QWidget *parent = nullptr);
    ~ANodeSettingsDialog();

private:
    Ui::ANodeSettingsDialog *ui;
};

#endif // ANODESETTINGSDIALOG_H
