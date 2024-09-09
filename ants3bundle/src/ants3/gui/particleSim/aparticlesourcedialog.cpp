#include "aparticlesourcedialog.h"
#include "ui_aparticlesourcedialog.h"
#include "ajsontools.h"
#include "guitools.h"
#include "afiletools.h"
#include "asourceparticlegenerator.h"
#include "aparticlesimsettings.h"
#include "aparticlesourceplotter.h"
#include "agraphbuilder.h"
#include "amaterialhub.h"

#include <QDebug>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>

#include "TH1D.h"
#include "TGraph.h"

AParticleSourceDialog::AParticleSourceDialog(const AParticleSourceRecord & Rec, QWidget * parent) :
    QDialog(parent),
    LocalRec(Rec), OriginalRec(Rec),
    ui(new Ui::AParticleSourceDialog)
{
    ui->setupUi(this);

    resize(width(), 700);

    setWindowModality(Qt::WindowModal);
    setWindowTitle("Particle source configurator");

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->pbUpdateRecord->setDefault(true);
    ui->pbUpdateRecord->setVisible(false);

    ui->leSourceName->setText(Rec.Name.data());
    ui->cobGunSourceType->setCurrentIndex(Rec.Shape);  // !!!***

    ui->ledGun1DSize->setText(QString::number(2.0 * Rec.Size1));
    ui->ledGun2DSize->setText(QString::number(2.0 * Rec.Size2));
    ui->ledGun3DSize->setText(QString::number(2.0 * Rec.Size3));

    ui->ledGunOriginX->setText(QString::number(Rec.X0));
    ui->ledGunOriginY->setText(QString::number(Rec.Y0));
    ui->ledGunOriginZ->setText(QString::number(Rec.Z0));

    ui->cbAxialDistribution->setChecked(Rec.UseAxialDistribution);
    int index = 0;
    switch (Rec.AxialDistributionType)
    {
    case AParticleSourceRecord::GaussAxial  : index = 0; break;
    case AParticleSourceRecord::CustomAxial : index = 1; break;
    default: guitools::message("Unknown axial distribution type, setting to Gauss!", this);
    }
    ui->cobAxialDistributionType->setCurrentIndex(index);
    ui->ledAxialDistributionSigma->setText(QString::number(Rec.AxialDistributionSigma));
    updateAxialButtons();

    ui->ledGunPhi->setText(QString::number(Rec.Phi));
    ui->ledGunTheta->setText(QString::number(Rec.Theta));
    ui->ledGunPsi->setText(QString::number(Rec.Psi));

    index = 0;
    switch (Rec.AngularMode)
    {
    case AParticleSourceRecord::Isotropic  : index = 0; break;
    case AParticleSourceRecord::FixedDirection  : index = 1; break;
    case AParticleSourceRecord::GaussDispersion : index = 2; break;
    case AParticleSourceRecord::CustomAngular   : index = 3; break;
    default : guitools::message("Unknown angular mode, setting to Uniform!", this);
    }
    ui->cobAngularMode->setCurrentIndex(index);
    on_cobAngularMode_currentIndexChanged(ui->cobAngularMode->currentIndex());
    ui->ledAngularSigma->setText(QString::number(Rec.DispersionSigma));
    updateCustomAngularButtons();
    ui->cobAngularSphericalOrVector->setCurrentIndex(Rec.DirectionBySphericalAngles ? 0 : 1);
    ui->ledDirectionX->setText(QString::number(Rec.DirectionVectorX));
    ui->ledDirectionY->setText(QString::number(Rec.DirectionVectorY));
    ui->ledDirectionZ->setText(QString::number(Rec.DirectionVectorZ));
    ui->ledGunCollPhi->setText(QString::number(Rec.DirectionPhi));
    ui->ledGunCollTheta->setText(QString::number(Rec.DirectionTheta));
    ui->cbAngularCutoff->setChecked(Rec.UseCutOff);
    ui->ledAngularCutoff->setText(QString::number(Rec.CutOff));

    ui->cbSourceLimitmat->setChecked(Rec.MaterialLimited);
    ui->leSourceLimitMaterial->setText(Rec.LimtedToMatName.data());

    index = 0;
    switch (Rec.TimeOffsetMode)
    {
    case AParticleSourceRecord::FixedOffset              : index = 0; break;
    case AParticleSourceRecord::ByEventIndexOffset       : index = 1; break;
    case AParticleSourceRecord::CustomDistributionOffset : index = 2; break;
    default : guitools::message("Unknown TimeOffsetMode, setting to FixedOffset!", this);
    }
    ui->cobTimeAverageMode->setCurrentIndex(index);
    ui->ledTimeAverageFixed->setText( QString::number(Rec.TimeFixedOffset) );
    ui->ledTimeAverageStart->setText( QString::number(Rec.TimeByEventStart) );
    ui->ledTimeAveragePeriod->setText( QString::number(Rec.TimeByEventPeriod) );
    updateTimeButtons();
    index = 0;
    switch (Rec.TimeSpreadMode)
    {
    case AParticleSourceRecord::NoSpread          : index = 0; break;
    case AParticleSourceRecord::GaussianSpread    : index = 1; break;
    case AParticleSourceRecord::UniformSpread     : index = 2; break;
    case AParticleSourceRecord::ExponentialSpread : index = 3; break;
    default : guitools::message("Unknown TimeSpreadMode, setting to NoSpread!", this);
    }
    ui->cobTimeSpreadMode->setCurrentIndex(index);
    ui->ledTimeSpreadSigma->setText( QString::number(Rec.TimeSpreadSigma) );
    ui->ledTimeSpreadWidth->setText( QString::number(Rec.TimeSpreadWidth) );
    updateTimeWithUnitsIndication(LocalRec.TimeSpreadHalfLife, LocalRec.TimeHalfLifePrefUnit, ui->ledTimeSpreadHalfLife, ui->cobPreferedHalfLifeUnits);

    updateListWidget();
    updateColorLimitingMat();

    if ( !Rec.Particles.empty() )
    {
        ui->lwGunParticles->setCurrentRow(0);
        updateParticleInfo();
    }

    on_cobGunSourceType_currentIndexChanged(ui->cobGunSourceType->currentIndex());

    restorePersistentSettings();
}

AParticleSourceDialog::~AParticleSourceDialog()
{
    storePersistentSettings();
    delete ui;
}

void AParticleSourceDialog::storePersistentSettings()
{
    QSettings settings;
    settings.beginGroup("ParticleSourceDialog");
    settings.setValue("ShowStatistics",  ui->cbShowStatistics->isChecked());
    settings.setValue("NumInStatistics", ui->sbGunTestEvents->value());
    settings.endGroup();
}

void AParticleSourceDialog::restorePersistentSettings()
{
    QSettings settings;
    settings.beginGroup("ParticleSourceDialog");
    bool ShowStatistics = settings.value("ShowStatistics", false).toBool();
    ui->cbShowStatistics->setChecked(ShowStatistics);
    int NumInStatistics = settings.value("NumInStatistics", 1000).toInt();
    ui->sbGunTestEvents->setValue(NumInStatistics);
    settings.endGroup();
}

AParticleSourceRecord & AParticleSourceDialog::getResult()
{
    return LocalRec;
}

void AParticleSourceDialog::closeEvent(QCloseEvent *e)
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

void AParticleSourceDialog::on_pbAccept_clicked()
{
    std::string err = LocalRec.check();
    if (err.empty()) accept();
    else guitools::message(err.data(), this);
}

void AParticleSourceDialog::on_pbReject_clicked()
{
    reject();
}

void AParticleSourceDialog::on_pbGunTest_clicked()
{
    AParticleSourcePlotter::clearTracks();
    if (ui->pbShowSource->isChecked()) AParticleSourcePlotter::plotSource(LocalRec);

    ASourceGeneratorSettings settings;
    settings.SourceData.push_back(LocalRec);
    settings.SourceData.back().Activity = 1.0;
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

void AParticleSourceDialog::on_cobGunSourceType_currentIndexChanged(int index)
{
    QVector<QString> s;
    switch (index)
    {
    default: qWarning() << "Unknown source type!";
            s << ""          << ""       << "";        break;
    case 0: s << ""          << ""       << "";        break;
    case 1: s << "Length:"   << ""       << "";        break;
    case 2: s << "SizeX:"    << "SizeY:" << "";        break;
    case 3: s << "Diameter:" << ""       << "";        break;
    case 4: s << "SizeX:"    << "SizeY:" << "SizeZ:";  break;
    case 5: s << "Diameter:" << ""       << "Height:"; break;
    }
    ui->lGun1DSize->setText(s[0]);
    ui->lGun2DSize->setText(s[1]);
    ui->lGun3DSize->setText(s[2]);

    bool bPoint = (index == 0);
    ui->frDimensions->setVisible(!bPoint);

    bool b1 = !s[0].isEmpty();
    ui->lGun1DSize->setVisible(b1);
    ui->ledGun1DSize->setVisible(b1);

    bool b2 = !s[1].isEmpty();
    ui->lGun2DSize->setVisible(b2);
    ui->ledGun2DSize->setVisible(b2);

    bool b3 = !s[2].isEmpty();
    ui->lGun3DSize->setVisible(b3);
    ui->ledGun3DSize->setVisible(b3);

    ui->ledGunPhi->setEnabled(index != 0);
    ui->ledGunTheta->setEnabled(index != 0);
    ui->ledGunPsi->setEnabled(index > 1);

    if (index == 1)
    {
        ui->labGunPhi->setText("Phi:");
        ui->labGunTheta->setText("Theta:");
    }
    else
    {
        ui->labGunPhi->setText("Around X:");
        ui->labGunTheta->setText("Around Y:");
    }

    ui->frLimitToMat->setVisible(index != 0);
    ui->frAxialDistribution->setVisible(index == 3);
}

void AParticleSourceDialog::on_pbGunAddNew_clicked()
{
    AGunParticle tmp;
    tmp.Particle = ui->leGunParticle->text().toLatin1().data();
    tmp.StatWeight = ui->ledGunParticleWeight->text().toDouble();
    tmp.FixedEnergy = ui->ledGunEnergy->text().toDouble();
    LocalRec.Particles.push_back(tmp);

    updateListWidget();
    ui->lwGunParticles->setCurrentRow( LocalRec.Particles.size()-1 );
    updateParticleInfo();
}

void AParticleSourceDialog::on_pbGunRemove_clicked()
{
    if (LocalRec.Particles.size() < 2)
    {
        guitools::message("There should be at least one particle!", this);
        return;
    }

    int iparticle = ui->lwGunParticles->currentRow();
    if (iparticle == -1)
    {
        guitools::message("Select a particle to remove!", this);
        return;
    }

    LocalRec.Particles.erase(LocalRec.Particles.begin() + iparticle);

    updateListWidget();
    ui->lwGunParticles->setCurrentRow( LocalRec.Particles.size()-1 );
    updateParticleInfo();
}

void AParticleSourceDialog::updateListWidget()
{
    bUpdateInProgress = true; // >>>---

    ui->lwGunParticles->clear();
    int counter = 0;
    for (const AGunParticle & gps : LocalRec.Particles)
    {
        bool Independent = (gps.GenerationType == AGunParticle::Independent);

        QString str, str1;
        if (Independent) str = "";
        else str = ">";
        str1.setNum(counter++);
        str += str1 + "> ";
        str += QString(gps.Particle.data());
        if (gps.UseFixedEnergy)
             str += QString(" E=%1keV").arg(gps.FixedEnergy);
        else str += " E=spec";

        if (Independent)
        {
            str += " W=";
            str1.setNum(gps.StatWeight);
            str += str1;
        }
        else
        {
            if (gps.GenerationType == AGunParticle::Linked_IfGenerated) str += " Link:";
            else str += " IfNotLink:";
            str += QString::number(gps.LinkedTo);
            str += " P=";
            str += QString::number(gps.LinkedProb);
            if (gps.BtBPair) str += " BtB pair";
        }
        ui->lwGunParticles->addItem(str);
    }

    bUpdateInProgress = false; // <<<---
}

void AParticleSourceDialog::updateParticleInfo()
{
    int row = ui->lwGunParticles->currentRow();

    int DefinedSourceParticles = LocalRec.Particles.size();
    if (DefinedSourceParticles > 0 && row > -1)
    {
        ui->fGunParticle->setEnabled(true);
        QString part = LocalRec.Particles.at(row).Particle.data();
        ui->leGunParticle->setText(part);

        const AGunParticle & gRec = LocalRec.Particles.at(row);

        QString str;
        str.setNum(gRec.StatWeight);
        ui->ledGunParticleWeight->setText(str);

        updateFixedEnergyIndication(gRec);

        int index = 0;
        if      (gRec.GenerationType == AGunParticle::Independent)           index = 0;
        else if (gRec.GenerationType == AGunParticle::Linked_IfGenerated)    index = 1;
        else if (gRec.GenerationType == AGunParticle::Linked_IfNotGenerated) index = 2;
        else qWarning() << "Not implemented particle generation type";
        ui->cobGenerationType->setCurrentIndex(index);
        ui->frIndependent->setVisible(gRec.GenerationType == AGunParticle::Independent);
        ui->frLinked->setVisible(gRec.GenerationType != AGunParticle::Independent);
        ui->sbLinkedTo->setValue(gRec.LinkedTo);
        str.setNum(gRec.LinkedProb);
        ui->ledLinkingProbability->setText(str);
        ui->cbLinkingOpposite->setChecked(gRec.BtBPair);

        bool bFix = gRec.UseFixedEnergy;
        ui->cobEnergy->setCurrentIndex(bFix ? 0 : 1);
        ui->swEnergy->setCurrentIndex(bFix ? 0 : 1);
        ui->pbGunShowSpectrum->setDisabled(gRec.EnergySpectrum.empty());
        ui->pbDeleteSpectrum->setDisabled(gRec.EnergySpectrum.empty());
    }
    else ui->fGunParticle->setEnabled(false);
}

void AParticleSourceDialog::on_lwGunParticles_currentRowChanged(int)
{
    if (bUpdateInProgress) return;

    updateParticleInfo();
}

void AParticleSourceDialog::on_cobUnits_activated(int)
{
    updateFixedEnergy();
    updateParticleInfo();
    //updateListWidget();
    on_pbUpdateRecord_clicked();
}

void AParticleSourceDialog::on_cobEnergySigmaUnits_activated(int)
{
    updateFixedEnergy();
    updateParticleInfo();
    updateListWidget();
    on_pbUpdateRecord_clicked();
}

void AParticleSourceDialog::on_pbUpdateRecord_clicked()
{
    LocalRec.Name = ui->leSourceName->text().toLatin1().data();

    switch (ui->cobGunSourceType->currentIndex())
    {
    case 0 : LocalRec.Shape = AParticleSourceRecord::Point;     break;
    case 1 : LocalRec.Shape = AParticleSourceRecord::Line;      break;
    case 2 : LocalRec.Shape = AParticleSourceRecord::Rectangle; break;
    case 3 : LocalRec.Shape = AParticleSourceRecord::Round;     break;
    case 4 : LocalRec.Shape = AParticleSourceRecord::Box;       break;
    case 5 : LocalRec.Shape = AParticleSourceRecord::Cylinder;  break;
    }

    LocalRec.Size1 = 0.5 * ui->ledGun1DSize->text().toDouble();
    LocalRec.Size2 = 0.5 * ui->ledGun2DSize->text().toDouble();
    LocalRec.Size3 = 0.5 * ui->ledGun3DSize->text().toDouble();

    LocalRec.MaterialLimited = ui->cbSourceLimitmat->isChecked();
    LocalRec.LimtedToMatName = ui->leSourceLimitMaterial->text().toLatin1().data();

    LocalRec.X0 = ui->ledGunOriginX->text().toDouble();
    LocalRec.Y0 = ui->ledGunOriginY->text().toDouble();
    LocalRec.Z0 = ui->ledGunOriginZ->text().toDouble();

    LocalRec.UseAxialDistribution = ui->cbAxialDistribution->isChecked();
    switch (ui->cobAxialDistributionType->currentIndex())
    {
    case 0 : LocalRec.AxialDistributionType = AParticleSourceRecord::GaussAxial;  break;
    case 1 : LocalRec.AxialDistributionType = AParticleSourceRecord::CustomAxial; break;
    default: qWarning() << "Not implemented AxialDistributionType";
    }
    LocalRec.AxialDistributionSigma = ui->ledAxialDistributionSigma->text().toDouble();
    updateAxialButtons();   // need here?

    LocalRec.Phi = ui->ledGunPhi->text().toDouble();
    LocalRec.Theta = ui->ledGunTheta->text().toDouble();
    LocalRec.Psi = ui->ledGunPsi->text().toDouble();

    switch (ui->cobAngularMode->currentIndex())
    {
    case 0 : LocalRec.AngularMode = AParticleSourceRecord::Isotropic;  break;
    case 1 : LocalRec.AngularMode = AParticleSourceRecord::FixedDirection;  break;
    case 2 : LocalRec.AngularMode = AParticleSourceRecord::GaussDispersion; break;
    case 3 : LocalRec.AngularMode = AParticleSourceRecord::CustomAngular;   break;
    default:
        qWarning() << "Unknown angular mode!";
        LocalRec.AngularMode = AParticleSourceRecord::Isotropic;
    }
    LocalRec.DispersionSigma = ui->ledAngularSigma->text().toDouble();
    LocalRec.DirectionBySphericalAngles = (ui->cobAngularSphericalOrVector->currentIndex() == 0);
    LocalRec.DirectionVectorX = ui->ledDirectionX->text().toDouble();
    LocalRec.DirectionVectorY = ui->ledDirectionY->text().toDouble();
    LocalRec.DirectionVectorZ = ui->ledDirectionZ->text().toDouble();
    LocalRec.DirectionPhi = ui->ledGunCollPhi->text().toDouble();
    LocalRec.DirectionTheta = ui->ledGunCollTheta->text().toDouble();
    LocalRec.UseCutOff = ui->cbAngularCutoff->isChecked();
    LocalRec.CutOff = ui->ledAngularCutoff->text().toDouble();

    switch (ui->cobTimeAverageMode->currentIndex())
    {
    case 0  : LocalRec.TimeOffsetMode = AParticleSourceRecord::FixedOffset;              break;
    case 1  : LocalRec.TimeOffsetMode = AParticleSourceRecord::ByEventIndexOffset;       break;
    case 2  : LocalRec.TimeOffsetMode = AParticleSourceRecord::CustomDistributionOffset; break;
    default :
        qWarning() << "Unknown time offset mode!";
        LocalRec.TimeOffsetMode = AParticleSourceRecord::FixedOffset;
    }
    LocalRec.TimeFixedOffset = ui->ledTimeAverageFixed->text().toDouble();
    LocalRec.TimeByEventStart = ui->ledTimeAverageStart->text().toDouble();
    LocalRec.TimeByEventPeriod = ui->ledTimeAveragePeriod->text().toDouble();
    switch (ui->cobTimeSpreadMode->currentIndex())
    {
    case 0  : LocalRec.TimeSpreadMode = AParticleSourceRecord::NoSpread;          break;
    case 1  : LocalRec.TimeSpreadMode = AParticleSourceRecord::GaussianSpread;    break;
    case 2  : LocalRec.TimeSpreadMode = AParticleSourceRecord::UniformSpread;     break;
    case 3  : LocalRec.TimeSpreadMode = AParticleSourceRecord::ExponentialSpread; break;
    default :
        qWarning() << "Unknown time spread mode!";
        LocalRec.TimeSpreadMode = AParticleSourceRecord::NoSpread;
    }
    LocalRec.TimeSpreadSigma = ui->ledTimeSpreadSigma->text().toDouble();
    LocalRec.TimeSpreadWidth = ui->ledTimeSpreadWidth->text().toDouble();
    readTimeWithUnits(ui->ledTimeSpreadHalfLife, ui->cobPreferedHalfLifeUnits, LocalRec.TimeSpreadHalfLife, LocalRec.TimeHalfLifePrefUnit);

    LocalRec.configureAngularSampler();
    LocalRec.configureTimeSampler();
    LocalRec.configureAxialSampler();

    int iPart = ui->lwGunParticles->currentRow();
    if (iPart >= 0)
    {
        AGunParticle & p = LocalRec.Particles[iPart];

        p.Particle = ui->leGunParticle->text().toLatin1().data();
        p.StatWeight = ui->ledGunParticleWeight->text().toDouble();
        p.UseFixedEnergy = (ui->cobEnergy->currentIndex() == 0);
        updateFixedEnergy();
        switch (ui->cobGenerationType->currentIndex())
        {
        case 0 : p.GenerationType = AGunParticle::Independent; break;
        case 1 : p.GenerationType = AGunParticle::Linked_IfGenerated; break;
        case 2 : p.GenerationType = AGunParticle::Linked_IfNotGenerated; break;
        default :
            qWarning() << "Non-implemented generation type for the particle!";
            p.GenerationType = AGunParticle::Independent;
        }
        p.LinkedTo = ui->sbLinkedTo->value();
        p.LinkedProb = ui->ledLinkingProbability->text().toDouble();
        p.BtBPair = ui->cbLinkingOpposite->isChecked();

        p.configureEnergySampler();
    }

    updateListWidget();

    if (iPart < 0 || iPart >= ui->lwGunParticles->count()) iPart = 0;
    ui->lwGunParticles->setCurrentRow(iPart);
    updateParticleInfo();
    updateColorLimitingMat();

    if (ui->pbShowSource->isChecked())
    {
        AParticleSourcePlotter::clearTracks();
        AParticleSourcePlotter::plotSource(LocalRec);
        emit requestShowSource();
    }
}

void AParticleSourceDialog::on_sbLinkedTo_editingFinished()
{
    int index = ui->sbLinkedTo->value();
    if (index >= ui->lwGunParticles->currentRow())
    {
        ui->sbLinkedTo->setValue(0);
        guitools::message("Bad index of the particle linked to! Setting to 0", this);
    }
    on_pbUpdateRecord_clicked();
}

void AParticleSourceDialog::on_ledLinkingProbability_editingFinished()
{
    double val = ui->ledLinkingProbability->text().toDouble();
    if ((val < 0) || (val > 1.0))
    {
        ui->ledLinkingProbability->setText(0);
        guitools::message("Linking probability has to be within [0, 1] range. Setting to 0", this);
    }
    on_pbUpdateRecord_clicked();
}

void AParticleSourceDialog::on_pbGunShowSpectrum_clicked()
{
    int particle = ui->lwGunParticles->currentRow();
    TGraph * gr = AGraphBuilder::graph(LocalRec.Particles[particle].EnergySpectrum);
    AGraphBuilder::configure(gr, "Energy distribution", "Energy, keV", "");
    emit requestDraw(gr, "APL", true, true);
}

void AParticleSourceDialog::on_pbGunLoadSpectrum_clicked()
{
    const QString fileName = guitools::dialogLoadFile(this, "Load energy spectrum", "");
    if (fileName.isEmpty()) return;

    const int iParticle = ui->lwGunParticles->currentRow();
    AGunParticle & Particle = LocalRec.Particles[iParticle];
    QString err = ftools::loadPairs(fileName, Particle.EnergySpectrum, true);
    if (err.isEmpty())
        err = QString(Particle.configureEnergySampler().data());
    if (!err.isEmpty())
    {
        Particle.EnergySpectrum.clear();
        guitools::message(err, this);
    }
    updateParticleInfo();
}

void AParticleSourceDialog::on_pbDeleteSpectrum_clicked()
{
    int iPart = ui->lwGunParticles->currentRow();
    LocalRec.Particles[iPart].EnergySpectrum.clear();
    LocalRec.Particles[iPart].UseFixedEnergy = true;

    updateParticleInfo();
}

void AParticleSourceDialog::updateColorLimitingMat()
{
    if (!ui->cbSourceLimitmat->isChecked()) return;

    const QStringList mats = AMaterialHub::getInstance().getListOfMaterialNames();
    const QString name = ui->leSourceLimitMaterial->text();
    bool fFound = false;
    for (const QString & mn : mats)
        if (name == mn)
        {
            fFound = true;
            break;
        }

    QPalette palette = ui->leSourceLimitMaterial->palette();
    palette.setColor(QPalette::Text, (fFound ? Qt::black : Qt::red) );
    ui->leSourceLimitMaterial->setPalette(palette);
}

void AParticleSourceDialog::updateCustomAngularButtons()
{
    const bool distrLoaded = !LocalRec.AngularDistribution.empty();
    ui->pbShowAngular->setEnabled(distrLoaded);
    ui->pbDeleteAngular->setEnabled(distrLoaded);
}

void AParticleSourceDialog::updateAxialButtons()
{
    const bool distrLoaded = !LocalRec.AxialDistribution.empty();
    ui->pbAxialDistributionShow->setEnabled(distrLoaded);
    ui->pbAxialDistributionRemove->setEnabled(distrLoaded);
}

void AParticleSourceDialog::updateTimeButtons()
{
    const bool distrLoaded = !LocalRec.TimeDistribution.empty();
    ui->pbTimeCustomShow->setEnabled(distrLoaded);
    ui->pbTimeCustomDelete->setEnabled(distrLoaded);
}

void AParticleSourceDialog::on_pbShowSource_clicked(bool checked)
{
    AParticleSourcePlotter::clearTracks();
    if (checked) AParticleSourcePlotter::plotSource(LocalRec);
    emit requestShowSource();
}

void AParticleSourceDialog::on_pbHelpParticle_clicked()
{
    guitools::message1("For particle simulations, the particle name should be one of those defined in Geant4, e.g.\n"
                       "   e-, proton, neutron, gamma, He3, etc."
                       "\n\n"
                       "For combined particle/optical simulations, the name can also be \"-\", \n"
                       "   which indicates direct energy deposition (a particle is NOT generated).\n"
                       "   Note that the direct energy deposition is only saved if the position is inside\n"
                       "   one of the sensitive volumes (See \"Settings\" tab)."
                       "\n\n"
                       "As an experimental feature, we are testing \"custom\" particle generation\n"
                       "   Advanced users can add their generation code to ants3 (search for \"_oPs\" string)\n"
                       "   The names of such particles must start from \"_\" character.\n"
                       "   Currently implemented:\n"
                       "   _oPs   --> Ortho-Positronium decaying into 3 gammas (energy settings are ignored)", "Particle name help", this);
}

void AParticleSourceDialog::on_pbShowAngular_clicked()
{
    TGraph * gr = AGraphBuilder::graph(LocalRec.AngularDistribution);
    AGraphBuilder::configure(gr, "Angular distribution", "Angle, degrees", "");
    emit requestDraw(gr, "APL", true, true);
}

void AParticleSourceDialog::on_pbLoadAngular_clicked()
{
    const QString fileName = guitools::dialogLoadFile(this, "Load angular distribution", "");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, LocalRec.AngularDistribution, true);
    if (err.isEmpty())
        err = QString(LocalRec.configureAngularSampler().data());
    if (!err.isEmpty())
    {
        LocalRec.AngularDistribution.clear();
        guitools::message(err, this);
    }
    updateCustomAngularButtons();
}

void AParticleSourceDialog::on_pbDeleteAngular_clicked()
{
    LocalRec.AngularDistribution.clear();
    updateCustomAngularButtons();
}

void AParticleSourceDialog::on_cobAngularMode_currentIndexChanged(int index)
{
    ui->swAngular->setCurrentIndex(index);
    updateDirectionVisibility();
}

void AParticleSourceDialog::updateDirectionVisibility()
{
    ui->frDirection->setVisible(ui->swAngular->currentIndex() != 0 || ui->cbAngularCutoff->isChecked());
}
void AParticleSourceDialog::on_cbAngularCutoff_toggled(bool)
{
    updateDirectionVisibility();
}

void AParticleSourceDialog::on_leSourceLimitMaterial_textEdited(const QString &)
{
    updateColorLimitingMat();
}

void AParticleSourceDialog::updateTimeWithUnitsIndication(double time_ns, AParticleSourceRecord::ETimeUnits prefUnits, QLineEdit * led, QComboBox * cob)
{
    int index = 0;
    double factor = 1.0;
    switch (prefUnits)
    {
    case AParticleSourceRecord::ns  : index = 0; factor = 1.0;    break;
    case AParticleSourceRecord::us  : index = 1; factor = 1e3;    break;
    case AParticleSourceRecord::ms  : index = 2; factor = 1e6;    break;
    case AParticleSourceRecord::s   : index = 3; factor = 1e9;    break;
    case AParticleSourceRecord::min : index = 4; factor = 60e9;   break;
    case AParticleSourceRecord::h   : index = 5; factor = 3600e9; break;
    default :
        qWarning() << "Not impelmented ETimeUnits enum value in updateTimeWithUnitsIndication";
        index = 0; factor = 1.0;
    }

    led->setText(QString::number(time_ns / factor));
    cob->setCurrentIndex(index);
}

void AParticleSourceDialog::readTimeWithUnits(QLineEdit * led, QComboBox * cob, double & time_ns, AParticleSourceRecord::ETimeUnits & prefUnits)
{
    AParticleSourceRecord::ETimeUnits e = AParticleSourceRecord::ns;
    double factor = 1.0;
    switch (cob->currentIndex())
    {
    case 0 : e = AParticleSourceRecord::ns;  factor = 1.0;    break;
    case 1 : e = AParticleSourceRecord::us;  factor = 1e3;    break;
    case 2 : e = AParticleSourceRecord::ms;  factor = 1e6;    break;
    case 3 : e = AParticleSourceRecord::s;   factor = 1e9;    break;
    case 4 : e = AParticleSourceRecord::min; factor = 60e9;   break;
    case 5 : e = AParticleSourceRecord::h;   factor = 3600e9; break;
    default :
        qWarning() << "Not implemented ETimeUnits enum value in readTimeWithUnits";
        e = AParticleSourceRecord::ns; factor = 1.0;
    }
    time_ns = led->text().toDouble() * factor;
    prefUnits = e;
}

double neutronEnergy_keV_ToWavelength_A(double energy_keV)
{
    // lambda = h / sqrt(2meV)
    return 6.62607015e-34 / sqrt(2 * 1.67492749804e-27 * 1.60217663e-19 * energy_keV * 1000) * 1e10;
}

double neutronWavelength_A_ToEnergy_keV(double wavelength_A)
{
    // lambda² = h² / (2meV) --> V = h² / lambda²/ (2me)
    double tmp = 6.62607015e-34 / (wavelength_A * 1e-10);
    tmp *= tmp;
    tmp /= (2 * 1.67492749804e-27 * 1.60217663e-19);
    return tmp * 0.001; // keV
}

void AParticleSourceDialog::updateFixedEnergyIndication(const AGunParticle & gRec)
{
    QStringList units{"MeV", "keV", "eV", "meV"};
    bool bNeutron = (gRec.Particle == "neutron");
    if (bNeutron) units << "A";
    ui->cobUnits->clear(); ui->cobUnits->addItems(units);
    ui->cobEnergySigmaUnits->clear(); ui->cobEnergySigmaUnits->addItems(units);

    int index = 0;
    double factor = 1.0;
    switch (gRec.PreferredUnits)
    {
    case AGunParticle::MeV      : index = 0; factor = 1e-3; break;
    case AGunParticle::keV      : index = 1; factor = 1.0;  break;
    case AGunParticle::eV       : index = 2; factor = 1e3;  break;
    case AGunParticle::meV      : index = 3; factor = 1e6;  break;
    case AGunParticle::Angstrom : index = 4; factor = 0;    break;
    default :
        qWarning() << "Not implemented EEneryUnits of PreferredUnits -> assuming keV";
        index = 1; factor = 1.0; break;
    }

    if (!bNeutron && index == 4)
    {
        qWarning() << "Attempt to use Angstrom energy units for not neutron -> changing to keV";
        index = 1; factor = 1.0;
    }

    ui->cobUnits->setCurrentIndex(index);
    double value;
    if (index == 4) value = neutronEnergy_keV_ToWavelength_A(gRec.FixedEnergy);
    else            value = gRec.FixedEnergy * factor;
    ui->ledGunEnergy->setText( QString::number(value) );

    ui->cbEnergyGaussBlur->setChecked(gRec.UseGaussBlur);
    index = 0;
    factor = 1.0;
    switch (gRec.PreferredSigmaUnits)
    {
    case AGunParticle::MeV      : index = 0; factor = 1e-3; break;
    case AGunParticle::keV      : index = 1; factor = 1.0;  break;
    case AGunParticle::eV       : index = 2; factor = 1e3;  break;
    case AGunParticle::meV      : index = 3; factor = 1e6;  break;
    case AGunParticle::Angstrom : index = 4; factor = 0;    break;
    default :
        qWarning() << "Not implemented EEneryUnits of PreferredSigmaUnits -> assuming keV";
        index = 1; factor = 1.0; break;
    }

    if (!bNeutron && index == 4)
    {
        qWarning() << "Attempt to use Angstrom energy units for not neutron -> changing to keV";
        index = 1; factor = 1.0;
    }

    ui->cobEnergySigmaUnits->setCurrentIndex(index);
    if (index == 4) value = neutronEnergy_keV_ToWavelength_A(gRec.EnergySigma);
    else            value = gRec.EnergySigma * factor;
    ui->ledEnergySigma->setText( QString::number(value) );
}

void AParticleSourceDialog::on_leGunParticle_editingFinished()
{
    const int row = ui->lwGunParticles->currentRow();
    const AGunParticle & gRec = LocalRec.Particles.at(row);
    if (ui->leGunParticle->text() != QString(gRec.Particle.data()))
        if (gRec.Particle == "neutron" || ui->leGunParticle->text() == "neutron")
            updateFixedEnergyIndication(gRec); // hide / show Angstrom units option

    on_pbUpdateRecord_clicked();
}

void AParticleSourceDialog::updateFixedEnergy()
{
    int iPart = ui->lwGunParticles->currentRow();
    if (iPart == -1) return;
    AGunParticle & p = LocalRec.Particles[iPart];

    double factor = 1.0;
    switch (ui->cobUnits->currentIndex())
    {
    case 0 : p.PreferredUnits = AGunParticle::MeV;      factor = 1e-3; break;
    case 1 : p.PreferredUnits = AGunParticle::keV;      factor = 1.0;  break;
    case 2 : p.PreferredUnits = AGunParticle::eV;       factor = 1e3;  break;
    case 3 : p.PreferredUnits = AGunParticle::meV;      factor = 1e6;  break;
    case 4 : p.PreferredUnits = AGunParticle::Angstrom; factor = 0;    break;
    default:
        qWarning() << "Not implemented EEneryUnits of PreferredUnits in updateFixedEnergy()";
        p.PreferredUnits = AGunParticle::keV;
    }

    double val = ui->ledGunEnergy->text().toDouble();
    if (ui->cobUnits->currentIndex() == 4) p.FixedEnergy = neutronWavelength_A_ToEnergy_keV(val);
    else                                   p.FixedEnergy = val / factor;

    p.UseGaussBlur = ui->cbEnergyGaussBlur->isChecked();
    factor = 1.0;
    switch (ui->cobEnergySigmaUnits->currentIndex())
    {
    case 0 : p.PreferredSigmaUnits = AGunParticle::MeV;      factor = 1e-3; break;
    case 1 : p.PreferredSigmaUnits = AGunParticle::keV;      factor = 1.0;  break;
    case 2 : p.PreferredSigmaUnits = AGunParticle::eV;       factor = 1e3;  break;
    case 3 : p.PreferredSigmaUnits = AGunParticle::meV;      factor = 1e6;  break;
    case 4 : p.PreferredSigmaUnits = AGunParticle::Angstrom; factor = 0;    break;
    default:
        qWarning() << "Not implemented EEneryUnits of PreferredSigmaUnits in updateFixedEnergy()";
        p.PreferredSigmaUnits = AGunParticle::keV;
    }

    val = ui->ledEnergySigma->text().toDouble();
    if (ui->cobEnergySigmaUnits->currentIndex() == 4) p.EnergySigma = neutronWavelength_A_ToEnergy_keV(val);
    else                                              p.EnergySigma = val / factor;
}

void AParticleSourceDialog::on_pbTimeCustomShow_clicked()
{
    TGraph * gr = AGraphBuilder::graph(LocalRec.TimeDistribution);
    AGraphBuilder::configure(gr, "Time offset distribution", "Time, ns", "");
    emit requestDraw(gr, "APL", true, true);
}

void AParticleSourceDialog::on_pbTimeCustomLoad_clicked()
{
    const QString fileName = guitools::dialogLoadFile(this, "Load custom distribution of time offsets", "");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, LocalRec.TimeDistribution, true);
    if (err.isEmpty())
        err = QString(LocalRec.configureTimeSampler().data());
    if (!err.isEmpty())
    {
        LocalRec.TimeDistribution.clear();
        guitools::message(err, this);
    }
    updateTimeButtons();
}

void AParticleSourceDialog::on_pbTimeCustomDelete_clicked()
{
    LocalRec.TimeDistribution.clear();
    updateTimeButtons();
}

void AParticleSourceDialog::on_pbAxialDistributionShow_clicked()
{
    TGraph * gr = AGraphBuilder::graph(LocalRec.AxialDistribution);
    AGraphBuilder::configure(gr, "Axial distribution", "Distance from axis, mm", "");
    emit requestDraw(gr, "APL", true, true);
}

void AParticleSourceDialog::on_pbAxialDistributionLoad_clicked()
{
    const QString fileName = guitools::dialogLoadFile(this, "Load custom axial distribution", "");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, LocalRec.AxialDistribution, true);
    if (err.isEmpty())
        err = QString(LocalRec.configureAxialSampler().data());
    if (!err.isEmpty())
    {
        LocalRec.AxialDistribution.clear();
        guitools::message(err, this);
    }
    updateAxialButtons();
}

void AParticleSourceDialog::on_pbAxialDistributionRemove_clicked()
{
    LocalRec.AxialDistribution.clear();
    updateAxialButtons();
}

void AParticleSourceDialog::on_cobEnergy_currentIndexChanged(int index)
{
    ui->frEnergyBlur->setVisible(index == 0);
}

void AParticleSourceDialog::on_cbEnergyGaussBlur_toggled(bool checked)
{
    ui->ledEnergySigma->setEnabled(checked);
    ui->cobEnergySigmaUnits->setEnabled(checked);
}

