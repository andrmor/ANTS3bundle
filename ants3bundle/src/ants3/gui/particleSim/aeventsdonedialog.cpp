#include "aeventsdonedialog.h"
#include "ui_aeventsdonedialog.h"

AEventsDoneDialog::AEventsDoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AEventsDoneDialog)
{
    ui->setupUi(this);
    setWindowTitle(" ");
}

AEventsDoneDialog::~AEventsDoneDialog()
{
    delete ui;
}

void AEventsDoneDialog::onProgressReported(int numEventsDone)
{
    ui->leEvents->setText( QString::number(numEventsDone) );
}

void AEventsDoneDialog::on_pbAbort_clicked()
{
    emit rejected();
}

