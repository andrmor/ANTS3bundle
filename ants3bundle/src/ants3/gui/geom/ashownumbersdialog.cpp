#include "ashownumbersdialog.h"
#include "ui_ashownumbersdialog.h"
#include "ageometrywindow.h"

#include <QDoubleValidator>

AShowNumbersDialog::AShowNumbersDialog(AGeometryWindow & gw) :
    QDialog(&gw), GW(gw),
    ui(new Ui::AShowNumbersDialog)
{
    ui->setupUi(this);

    setWindowTitle("Selector");

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    foreach (QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);
}

AShowNumbersDialog::~AShowNumbersDialog()
{
    delete ui;
}

int AShowNumbersDialog::exec()
{
    move(GW.x(), GW.y() + 100);
    return QDialog::exec();
}

void AShowNumbersDialog::on_pbSensorIndex_clicked()
{
    GW.showSensorIndexes();
    accept();
}

void AShowNumbersDialog::on_pbSensorModel_clicked()
{
    GW.showSensorModelIndexes();
    accept();
}

void AShowNumbersDialog::on_pbOptMonIndex_clicked()
{
    GW.showPhotonMonIndexes();
    accept();
}

void AShowNumbersDialog::on_pbPartMonIndex_clicked()
{
    GW.showParticleMonIndexes();
    accept();
}

void AShowNumbersDialog::on_pbOptMonHits_clicked()
{

}

void AShowNumbersDialog::on_pbPartMonHits_clicked()
{

}

void AShowNumbersDialog::on_pbCalIndex_clicked()
{
    GW.showCalorimeterIndexes();
    accept();
}

void AShowNumbersDialog::on_pbCalTotals_clicked()
{

}

void AShowNumbersDialog::on_ledSizeSensor_editingFinished()
{
    const double size = ui->ledSizeSensor->text().toDouble();
    GW.GeoWriter.setSizeForSensors(size);
}

void AShowNumbersDialog::on_ledSizeMonitor_editingFinished()
{
    const double size = ui->ledSizeMonitor->text().toDouble();
    GW.GeoWriter.setSizeForMonitors(size);
}

void AShowNumbersDialog::on_ledSizeCalorimeter_editingFinished()
{
    const double size = ui->ledSizeCalorimeter->text().toDouble();
    GW.GeoWriter.setSizeForCalorimeters(size);
}
