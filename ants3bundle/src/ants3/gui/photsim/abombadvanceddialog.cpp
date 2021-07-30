#include "abombadvanceddialog.h"
#include "ui_abombadvanceddialog.h"

ABombAdvancedDialog::ABombAdvancedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ABombAdvancedDialog)
{
    ui->setupUi(this);
}

ABombAdvancedDialog::~ABombAdvancedDialog()
{
    delete ui;
}
