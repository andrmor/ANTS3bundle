#include "asetmarginsdialog.h"
#include "ui_asetmarginsdialog.h"
#include "a3global.h"

#include <QDoubleValidator>

ASetMarginsDialog::ASetMarginsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ASetMarginsDialog)
{
    ui->setupUi(this);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    foreach (QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->pbAccept->setDefault(true);

    const A3Global & GlobSet = A3Global::getConstInstance();

    ui->ledTop->      setText( QString::number(GlobSet.MarginTop));
    ui->ledBottom->   setText( QString::number(GlobSet.MarginBottom));
    ui->ledLeft->     setText( QString::number(GlobSet.MarginLeft));
    ui->ledRight->    setText( QString::number(GlobSet.MarginRight));
    ui->ledRightColz->setText( QString::number(GlobSet.MarginRightColz));
}

ASetMarginsDialog::~ASetMarginsDialog()
{
    delete ui;
}

void ASetMarginsDialog::on_pbAccept_clicked()
{
    A3Global & GlobSet = A3Global::getInstance();

    GlobSet.MarginTop       = ui->ledTop->text().toDouble();
    GlobSet.MarginBottom    = ui->ledBottom->text().toDouble();
    GlobSet.MarginLeft      = ui->ledLeft->text().toDouble();
    GlobSet.MarginRight     = ui->ledRight->text().toDouble();
    GlobSet.MarginRightColz = ui->ledRightColz->text().toDouble();

    accept();
}

void ASetMarginsDialog::on_pbCancel_clicked()
{
    reject();
}

void ASetMarginsDialog::on_pbReset_clicked()
{
    ui->ledTop->setText("0.05");
    ui->ledBottom->setText("0.1");
    ui->ledLeft->setText("0.1");
    ui->ledRight->setText("0.1");
    ui->ledRightColz->setText("0.15");
}

