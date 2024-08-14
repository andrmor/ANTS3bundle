#include "aparticleanalyzerwidget.h"
#include "ui_aparticleanalyzerwidget.h"
#include "ageospecial.h"

#include <QDoubleValidator>

AParticleAnalyzerWidget::AParticleAnalyzerWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::AParticleAnalyzerWidget)
{
    ui->setupUi(this);
    ui->pbChanged->setVisible(false);

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);
}

AParticleAnalyzerWidget::~AParticleAnalyzerWidget()
{
    delete ui;
}

void AParticleAnalyzerWidget::updateGui(const AGeoParticleAnalyzer & pa)
{
    const QSignalBlocker blocker(this);

    ui->sbEnergyBins->setValue(pa.EnergyBins);
    ui->ledEnergyFrom->setText(QString::number(pa.EnergyFrom));
    ui->ledEnergyTo->setText(QString::number(pa.EnergyTo));
    int index = 1;
        if (pa.EnergyUnits == "MeV") index = 0;
        if (pa.EnergyUnits == "eV")  index = 2;
    ui->cobEnergyUnits->setCurrentIndex(index);

    ui->cbTimeWindow->setChecked(pa.UseTimeWindow);
    ui->ledTimeWindowFrom->setText(QString::number(pa.TimeWindowFrom));
    ui->ledTimeWindowTo->setText(QString::number(pa.TimeWindowTo));

    ui->cbStopTracking->setChecked(pa.StopTracking);

    ui->cbSingleInstance->setChecked(pa.SingleInstanceForAllCopies);
}

void AParticleAnalyzerWidget::updateObject(AGeoParticleAnalyzer & pa) const
{
    pa.EnergyBins = ui->sbEnergyBins->value();
    pa.EnergyFrom = ui->ledEnergyFrom->text().toDouble();
    pa.EnergyTo = ui->ledEnergyTo->text().toDouble();
    pa.EnergyUnits = ui->cobEnergyUnits->currentText();

    pa.UseTimeWindow = ui->cbTimeWindow->isChecked();
    pa.TimeWindowFrom = ui->ledTimeWindowFrom->text().toDouble();
    pa.TimeWindowTo = ui->ledTimeWindowTo->text().toDouble();

    pa.StopTracking = ui->cbStopTracking->isChecked();

    pa.SingleInstanceForAllCopies = ui->cbSingleInstance->isChecked();
}

QString AParticleAnalyzerWidget::check() const
{
    AGeoParticleAnalyzer pa;
    updateObject(pa);

    if (pa.TimeWindowFrom >= pa.TimeWindowTo) return "Bad time window in particle analyzer:\n'to' should be larger than 'from'";

    return "";
}

void AParticleAnalyzerWidget::on_pbChanged_clicked()
{
    emit contentChanged();
}

