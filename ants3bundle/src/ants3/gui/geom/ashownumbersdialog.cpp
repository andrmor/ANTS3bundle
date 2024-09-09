#include "ashownumbersdialog.h"
#include "ui_ashownumbersdialog.h"
#include "ageometrywindow.h"

#include <QDoubleValidator>

AShowNumbersDialog::AShowNumbersDialog(AGeometryWindow & gw) :
    QDialog(&gw), GW(gw),
    ui(new Ui::AShowNumbersDialog)
{
    ui->setupUi(this);

    setWindowTitle("Show:");

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    //QList<QLineEdit*> list = findChildren<QLineEdit*>();
    //foreach (QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);
    ui->ledSize->setValidator(dv);

    ui->cobObjectType->setCurrentIndex(0);
    on_cobObjectType_currentIndexChanged(0);
}

AShowNumbersDialog::~AShowNumbersDialog()
{
    delete ui;
}

void AShowNumbersDialog::on_cobObjectType_currentIndexChanged(int index)
{
    ui->swMain->setCurrentIndex(index);

    double size = 10;
    switch (index)
    {
    case 0: size = GW.GeoWriter.SizeForScints;       break; // Scint
    case 1: size = GW.GeoWriter.SizeForSensors;      break; // Opt sens
    case 2: size = GW.GeoWriter.SizeForMonitors;     break; // Part mon
    case 3: size = GW.GeoWriter.SizeForMonitors;     break; // Opt mon
    case 4: size = GW.GeoWriter.SizeForCalorimeters; break; // Calorimeters
    case 5: size = GW.GeoWriter.SizeForAnalyzers;    break; // Part ana
    case 6: size = GW.GeoWriter.SizeForPhotFuncts;   break; // Phot funct
    };

    ui->ledSize->setText(QString::number(size));
}

int AShowNumbersDialog::exec()
{
    move(GW.x(), GW.y() + 250);
    return QDialog::exec();
}

void AShowNumbersDialog::on_ledSize_editingFinished()
{
    int index = ui->cobObjectType->currentIndex();
    double size = ui->ledSize->text().toDouble();

    switch (index)
    {
    case 0: GW.GeoWriter.SizeForScints       = size; break; // Scint
    case 1: GW.GeoWriter.SizeForSensors      = size; break; // Opt sens
    case 2: GW.GeoWriter.SizeForMonitors     = size; break; // Part mon
    case 3: GW.GeoWriter.SizeForMonitors     = size; break; // Opt mon
    case 4: GW.GeoWriter.SizeForCalorimeters = size; break; // Calorimeters
    case 5: GW.GeoWriter.SizeForAnalyzers    = size; break; // Part ana
    case 6: GW.GeoWriter.SizeForPhotFuncts   = size; break; // Phot funct
    }
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

void AShowNumbersDialog::on_pbPhFunIndex_clicked()
{
    GW.showPhotonFunctionalIndexes();
    accept();
}

void AShowNumbersDialog::on_pbPhFunLinks_clicked()
{
    GW.onRequestShowAllConnections();
    accept();
}

void AShowNumbersDialog::on_pbAnIndex_clicked()
{
    GW.showAnalyzerIndexes();
    accept();
}

void AShowNumbersDialog::on_pbScintillatorIndex_clicked()
{
    GW.showScintillatorIndexes();
    accept();
}

