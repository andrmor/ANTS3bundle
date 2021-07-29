#include "anodesettingsdialog.h"
#include "ui_anodesettingsdialog.h"

ANodeSettingsDialog::ANodeSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ANodeSettingsDialog)
{
    ui->setupUi(this);
}

ANodeSettingsDialog::~ANodeSettingsDialog()
{
    delete ui;
}
