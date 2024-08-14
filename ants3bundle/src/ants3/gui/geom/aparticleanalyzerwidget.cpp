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

    const AParticleAnalyzerRecord & prop = pa.Properties;

    ui->sbEnergyBins->setValue(prop.EnergyBins);
    ui->ledEnergyFrom->setText(QString::number(prop.EnergyFrom));
    ui->ledEnergyTo->setText(QString::number(prop.EnergyTo));
    int index = 1;
        if (prop.EnergyUnits == "MeV") index = 0;
        if (prop.EnergyUnits == "eV")  index = 2;
    ui->cobEnergyUnits->setCurrentIndex(index);

    ui->cbTimeWindow->setChecked(prop.UseTimeWindow);
    ui->ledTimeWindowFrom->setText(QString::number(prop.TimeWindowFrom));
    ui->ledTimeWindowTo->setText(QString::number(prop.TimeWindowTo));

    ui->cbStopTracking->setChecked(prop.StopTracking);

    ui->cbSingleInstance->setChecked(prop.SingleInstanceForAllCopies);
}

void AParticleAnalyzerWidget::updateObject(AGeoParticleAnalyzer & pa) const
{
    AParticleAnalyzerRecord & prop = pa.Properties;

    prop.EnergyBins = ui->sbEnergyBins->value();
    prop.EnergyFrom = ui->ledEnergyFrom->text().toDouble();
    prop.EnergyTo = ui->ledEnergyTo->text().toDouble();
    prop.EnergyUnits = ui->cobEnergyUnits->currentText().toLatin1().data();

    prop.UseTimeWindow = ui->cbTimeWindow->isChecked();
    prop.TimeWindowFrom = ui->ledTimeWindowFrom->text().toDouble();
    prop.TimeWindowTo = ui->ledTimeWindowTo->text().toDouble();

    prop.StopTracking = ui->cbStopTracking->isChecked();

    prop.SingleInstanceForAllCopies = ui->cbSingleInstance->isChecked();
}

QString AParticleAnalyzerWidget::check() const
{
    AGeoParticleAnalyzer pa;
    updateObject(pa);

    const AParticleAnalyzerRecord & prop = pa.Properties;
    if (prop.TimeWindowFrom >= prop.TimeWindowTo) return "Bad time window in particle analyzer:\n'to' should be larger than 'from'";

    return "";
}

void AParticleAnalyzerWidget::on_pbChanged_clicked()
{
    emit contentChanged();
}

