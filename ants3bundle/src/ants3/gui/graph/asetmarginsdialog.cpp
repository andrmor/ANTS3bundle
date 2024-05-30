#include "asetmarginsdialog.h"
#include "ui_asetmarginsdialog.h"

#include <QDoubleValidator>

ASetMarginsDialog::ASetMarginsDialog(const ADrawMarginsRecord & rec, const ADrawMarginsRecord & defaultRec, QWidget * parent) :
    QDialog(parent), DefaultRec(defaultRec), ui(new Ui::ASetMarginsDialog)
{
    ui->setupUi(this);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    foreach (QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->pbAccept->setDefault(true);

    updateGui(rec);
}

ASetMarginsDialog::~ASetMarginsDialog()
{
    delete ui;
}

void ASetMarginsDialog::updateGui(const ADrawMarginsRecord & rec)
{
    ui->ledTop->      setText( QString::number(rec.Top) );
    ui->ledBottom->   setText( QString::number(rec.Bottom) );
    ui->ledLeft->     setText( QString::number(rec.Left) );
    ui->ledRight->    setText( QString::number(rec.Right) );
    ui->ledRightColz->setText( QString::number(rec.RightForZ) );

    ui->labCustomMargins->setVisible(rec.Override);
}

ADrawMarginsRecord ASetMarginsDialog::getResult() const
{
    ADrawMarginsRecord rec;

    rec.Top       = ui->ledTop->text().toDouble();
    rec.Bottom    = ui->ledBottom->text().toDouble();
    rec.Left      = ui->ledLeft->text().toDouble();
    rec.Right     = ui->ledRight->text().toDouble();
    rec.RightForZ = ui->ledRightColz->text().toDouble();

    return rec;
}

void ASetMarginsDialog::on_pbAccept_clicked()
{
    accept();
}

void ASetMarginsDialog::on_pbCancel_clicked()
{
    reject();
}

void ASetMarginsDialog::on_pbReset_clicked()
{
    updateGui(DefaultRec);
    UseDefault = true;
    accept();
}

