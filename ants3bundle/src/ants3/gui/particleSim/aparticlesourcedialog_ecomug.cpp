#include "aparticlesourcedialog_ecomug.h"
#include "ui_aparticlesourcedialog_ecomug.h"
#include "guitools.h"
#include "asourceparticlegenerator.h"
#include "aparticlesimsettings.h"
#include "aparticlesourceplotter.h"
#include "agraphbuilder.h"

#include <QDebug>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QJsonObject>

#include "TH1D.h"
#include "TGraph.h"

AParticleSourceDialog_EcoMug::AParticleSourceDialog_EcoMug(const AParticleSourceRecord_EcoMug & Rec, QWidget * parent) :
    AParticleSourceDialogBase(parent),
    LocalRec(Rec), OriginalRec(Rec),
    ui(new Ui::AParticleSourceDialog_EcoMug)
{
    ui->setupUi(this);

    resize(width(), 200);

    setWindowModality(Qt::WindowModal);
    setWindowTitle("EcoMug particle source configurator");

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->pbUpdateRecord->setDefault(true);
    ui->pbUpdateRecord->setVisible(false);

    ui->leSourceName->setText(Rec.Name.data());
    ui->cobGeneratorShape->setCurrentIndex(Rec.Shape);

    ui->ledSize1->setText(QString::number(Rec.Size1));
    ui->ledSize2->setText(QString::number(Rec.Size2));

    ui->ledX->setText(QString::number(Rec.X0));
    ui->ledY->setText(QString::number(Rec.Y0));
    ui->ledZ->setText(QString::number(Rec.Z0));

    restorePersistentSettings();
}

AParticleSourceDialog_EcoMug::~AParticleSourceDialog_EcoMug()
{
    storePersistentSettings();
    delete ui;
}

void AParticleSourceDialog_EcoMug::storePersistentSettings()
{
    QSettings settings;
    settings.beginGroup("ParticleSourceDialog");
    settings.setValue("ShowStatistics",  ui->cbShowStatistics->isChecked());
    settings.setValue("NumInStatistics", ui->sbGunTestEvents->value());
    settings.endGroup();
}

void AParticleSourceDialog_EcoMug::restorePersistentSettings()
{
    QSettings settings;
    settings.beginGroup("ParticleSourceDialog");
    bool ShowStatistics = settings.value("ShowStatistics", false).toBool();
    ui->cbShowStatistics->setChecked(ShowStatistics);
    int NumInStatistics = settings.value("NumInStatistics", 1000).toInt();
    ui->sbGunTestEvents->setValue(NumInStatistics);
    settings.endGroup();
}

AParticleSourceRecordBase * AParticleSourceDialog_EcoMug::getResult()
{
    AParticleSourceRecord_EcoMug * rec = new AParticleSourceRecord_EcoMug();
    QJsonObject json;
    LocalRec.writeToJson(json);
    rec->readFromJson(json);
    return rec;
}

void AParticleSourceDialog_EcoMug::closeEvent(QCloseEvent *e)
{
    on_pbUpdateRecord_clicked();

    QJsonObject jo, jn;
    LocalRec.writeToJson(jo);
    OriginalRec.writeToJson(jn);
    if (jo != jn)
    {
        QMessageBox msgBox(this);
        msgBox.setText("The source has been modified.");
        msgBox.setInformativeText("Discard all the changes?");
        msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Discard);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
        {
            e->ignore();
            return;
        }
    }
    QDialog::closeEvent(e);
}

void AParticleSourceDialog_EcoMug::on_pbAccept_clicked()
{
    std::string err = LocalRec.check();
    if (err.empty()) accept();
    else guitools::message(err.data(), this);
}

void AParticleSourceDialog_EcoMug::on_pbReject_clicked()
{
    reject();
}

void AParticleSourceDialog_EcoMug::on_pbGunTest_clicked()
{
    AParticleSourcePlotter::clearTracks();
    if (ui->pbShowSource->isChecked()) AParticleSourcePlotter::plotSource(LocalRec);

    ASourceGeneratorSettings settings;
    settings.SourceData.push_back(&LocalRec);
    settings.SourceData.back()->Activity = 1.0;
    ASourceParticleGenerator gun(settings);

    auto abort = [&gun]{gun.AbortRequested = true;};
    QDialog D(this);
    D.setWindowTitle("Particle generator");
    D.setMinimumWidth(250);
    QHBoxLayout * lay = new QHBoxLayout(&D);
    lay->addWidget(new QLabel("Generating..."));
    QPushButton * pb = new QPushButton("Abort");
    lay->addWidget(pb);
    connect(pb, &QPushButton::clicked, &D, &QDialog::reject);
    connect(&D, &QDialog::rejected, &D, abort); // react both to close and button click
    D.setModal(true);
    D.move(mapToGlobal(ui->pbGunTest->pos()));

    this->setDisabled(true);   //-->
    D.setEnabled(true);
    D.show();

    emit requestTestParticleGun(&gun, ui->sbGunTestEvents->value(), ui->cbShowStatistics->isChecked());

    this->setDisabled(false);  // <--
}

void AParticleSourceDialog_EcoMug::on_cobGeneratorShape_currentIndexChanged(int index)
{
    QList<QString> s;
    switch (index)
    {
    case 0: s << "X size:" << "Y size:"; break;
    case 1: s << "Radius:" << "Height:"; break;
    case 2: s << "Radius:" << "";        break;
    default: qWarning() << "Unexpected shape index!";
    }
    ui->labSize1->setText(s[0]);
    ui->labSize2->setText(s[1]);

    bool b1 = !s[1].isEmpty();
    ui->labSize2->setVisible(b1);
    ui->ledSize2->setVisible(b1);
}

void AParticleSourceDialog_EcoMug::on_pbUpdateRecord_clicked()
{
    LocalRec.Name = ui->leSourceName->text().toLatin1().data();

    switch (ui->cobGeneratorShape->currentIndex())
    {
    case 0 : LocalRec.Shape = AParticleSourceRecord_EcoMug::Rectangle;  break;
    case 1 : LocalRec.Shape = AParticleSourceRecord_EcoMug::Cylinder;   break;
    case 2 : LocalRec.Shape = AParticleSourceRecord_EcoMug::HalfSphere; break;
    }

    LocalRec.Size1 = ui->ledSize1->text().toDouble();
    LocalRec.Size2 = ui->ledSize2->text().toDouble();

    LocalRec.X0 = ui->ledX->text().toDouble();
    LocalRec.Y0 = ui->ledY->text().toDouble();
    LocalRec.Z0 = ui->ledZ->text().toDouble();

    if (ui->pbShowSource->isChecked())
    {
        AParticleSourcePlotter::clearTracks();
        AParticleSourcePlotter::plotSource(LocalRec);
        emit requestShowSource();
    }
}

void AParticleSourceDialog_EcoMug::on_pbShowSource_clicked(bool checked)
{
    AParticleSourcePlotter::clearTracks();
    if (checked) AParticleSourcePlotter::plotSource(LocalRec);
    emit requestShowSource();
}
