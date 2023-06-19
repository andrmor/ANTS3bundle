#include "aworldsizewarningdialog.h"
#include "ui_aworldsizewarningdialog.h"

AWorldSizeWarningDialog::AWorldSizeWarningDialog(double xy, double z, QWidget * parent) :
    QDialog(parent),
    ui(new Ui::AWorldSizeWarningDialog)
{
    ui->setupUi(this);
    setWindowTitle("Warning");
    ui->labXY->setText(QString::number(2.0 * xy, 'g', 4));
    ui->labZ->setText( QString::number(2.0 * z,  'g', 4));

}

AWorldSizeWarningDialog::~AWorldSizeWarningDialog()
{
    delete ui;
}

void AWorldSizeWarningDialog::on_pbOK_clicked()
{
    Result = OK;
    accept();
}

void AWorldSizeWarningDialog::on_pbGoto_clicked()
{
    Result = Goto;
    accept();
}

void AWorldSizeWarningDialog::on_pbDont_clicked()
{
    Result = DontBother;
    accept();
}

