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

#include "TH1D.h"
#include "TGraph.h"

// save to make persistent?
static bool ShowStatistics = false;
static int  NumInStatistics = 1000;

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
    ui->cobGunSourceType->setCurrentIndex(Rec.Shape);

    ui->ledGun1DSize->setText(QString::number(2.0 * Rec.Size1));
    ui->ledGun2DSize->setText(QString::number(2.0 * Rec.Size2));
    ui->ledGun3DSize->setText(QString::number(2.0 * Rec.Size3));

    ui->ledGunOriginX->setText(QString::number(Rec.X0));
    ui->ledGunOriginY->setText(QString::number(Rec.Y0));
    ui->ledGunOriginZ->setText(QString::number(Rec.Z0));

    ui->ledGunPhi->setText(QString::number(Rec.Phi));
    ui->ledGunTheta->setText(QString::number(Rec.Theta));
    ui->ledGunPsi->setText(QString::number(Rec.Psi));

    int index = 0;
    switch (Rec.AngularMode)
    {
    case AParticleSourceRecord::UniformAngular  : index = 0; break;
    case AParticleSourceRecord::FixedDirection  : index = 1; break;
    case AParticleSourceRecord::GaussDispersion : index = 2; break;
    case AParticleSourceRecord::CustomAngular   : index = 3; break;
    default:
        guitools::message("Unknown angular mode, setting to Uniform!", this);
    }
    ui->cobAngularMode->setCurrentIndex(index);
    on_cobAngularMode_currentIndexChanged(ui->cobAngularMode->currentIndex());
    ui->ledAngularSigma->setText(QString::number(Rec.DispersionSigma));
    updateCustomAngularButtons();
    ui->ledGunCollPhi->setText(QString::number(Rec.DirectionPhi));
    ui->ledGunCollTheta->setText(QString::number(Rec.DirectionTheta));
    ui->cbAngularCutoff->setChecked(Rec.UseCutOff);
    ui->ledAngularCutoff->setText(QString::number(Rec.CutOff));

    ui->cbSourceLimitmat->setChecked(Rec.MaterialLimited);
    ui->leSourceLimitMaterial->setText(Rec.LimtedToMatName.data());

    ui->cobTimeAverageMode->setCurrentIndex(Rec.TimeAverageMode);
    ui->ledTimeAverageFixed->setText( QString::number(Rec.TimeAverage) );
    ui->ledTimeAverageStart->setText( QString::number(Rec.TimeAverageStart) );
    ui->ledTimeAveragePeriod->setText( QString::number(Rec.TimeAveragePeriod) );
    ui->cobTimeSpreadMode->setCurrentIndex(Rec.TimeSpreadMode);
    ui->ledTimeSpreadSigma->setText( QString::number(Rec.TimeSpreadSigma) );
    ui->ledTimeSpreadWidth->setText( QString::number(Rec.TimeSpreadWidth) );

    updateListWidget();
    updateColorLimitingMat();
    if ( !Rec.Particles.empty() )
    {
        ui->lwGunParticles->setCurrentRow(0);
        updateParticleInfo();
    }

    on_cobGunSourceType_currentIndexChanged(ui->cobGunSourceType->currentIndex());

    ui->pbAbort->setVisible(false);

    ui->cbShowStatistics->setChecked(ShowStatistics);
    ui->sbGunTestEvents->setValue(NumInStatistics);

    // !!!*** export / import source
//    QMenuBar* mb = new QMenuBar(this);
//    QMenu* fileMenu = mb->addMenu("&File");
//    fileMenu->addAction("Load source", this, &AParticleSourceDialog::loadSource);
//    fileMenu->addAction("Save source", this, &AParticleSourceDialog::saveSource);
//    layout()->setMenuBar(mb);
}

AParticleSourceDialog::~AParticleSourceDialog()
{
    ShowStatistics = ui->cbShowStatistics->isChecked();
    NumInStatistics = ui->sbGunTestEvents->value();

    delete ui;
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
    else guitools::message(QString(err.data()), this);
}

void AParticleSourceDialog::on_pbReject_clicked()
{
    reject();
}

void AParticleSourceDialog::on_pbGunTest_clicked()
{
    ui->pbGunTest->setEnabled(false); //-->

    AParticleSourcePlotter::clearTracks();
    if (ui->pbShowSource->isChecked()) AParticleSourcePlotter::plotSource(LocalRec);

    ASourceGeneratorSettings settings;
    settings.SourceData.push_back(LocalRec);
    ASourceParticleGenerator gun(settings);

    ui->pbAbort->setVisible(true);

    emit requestTestParticleGun(&gun, ui->sbGunTestEvents->value(), ui->cbShowStatistics->isChecked());

    ui->pbAbort->setVisible(false);

    ui->pbGunTest->setEnabled(true);  // <--
}

void AParticleSourceDialog::on_cobGunSourceType_currentIndexChanged(int index)
{
    QVector<QString> s;
    switch (index)
    {
      case 0: s <<""<<""<<"";
        break;
      case 1: s <<"Length:"<<""<<"";
        break;
      case 2: s <<"SizeX:"<<"SizeY:"<<"";
        break;
      case 3: s <<"Diameter:"<<""<<"";
        break;
      case 4: s <<"SizeX:"<<"SizeY:"<<"SizeZ:";
        break;
      case 5: s <<"Diameter:"<<""<<"Height:";
        break;
      default:
        qWarning() << "Unknown source type!";
        s <<""<<""<<"";
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
        bool Independent = (gps.ParticleType == AGunParticle::Independent);

        QString str, str1;
        if (Independent) str = "";
        else str = ">";
        str1.setNum(counter++);
        str += str1 + "> ";
        str += QString(gps.Particle.data());
        if (gps.UseFixedEnergy)
             str += QString(" E=%1").arg(gps.FixedEnergy);
        else str += " E=spec";

        if (Independent)
        {
            str += " W=";
            str1.setNum(gps.StatWeight);
            str += str1;
        }
        else
        {
            if (gps.ParticleType == AGunParticle::Linked_IfGenerated) str += " Link:";
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

        int iPrefUnits = ui->cobUnits->findText(gRec.PreferredUnits.data());
        double energy = gRec.FixedEnergy;
        if (iPrefUnits > -1)
        {
            ui->cobUnits->setCurrentIndex(iPrefUnits);
            if      (gRec.PreferredUnits == "MeV") energy *= 1.0e-3;
            else if (gRec.PreferredUnits == "keV") ;
            else if (gRec.PreferredUnits == "eV") energy *= 1.0e3;
            else if (gRec.PreferredUnits == "meV") energy *= 1.0e6;
        }
        else ui->cobUnits->setCurrentText("keV");
        ui->ledGunEnergy->setText( QString::number(energy) );

        ui->frIndependent->setVisible(gRec.ParticleType == AGunParticle::Independent);
        ui->frLinked->setVisible(gRec.ParticleType != AGunParticle::Independent);
        ui->sbLinkedTo->setValue(gRec.LinkedTo);
        str.setNum(gRec.LinkedProb);
        ui->ledLinkingProbability->setText(str);
        ui->cbLinkingOpposite->setChecked(gRec.BtBPair);

        bool bFix = gRec.UseFixedEnergy;
        ui->cobEnergy->setCurrentIndex(bFix ? 0 : 1);
        ui->swEnergy->setCurrentIndex(bFix ? 0 : 1);
        ui->pbGunShowSpectrum->setEnabled(!gRec.EnergySpectrum.empty());
        ui->cbRangeBaseEnergyData->setVisible(!bFix);
        ui->cbRangeBaseEnergyData->setChecked(gRec.RangeBasedEnergies);
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
    int iPart = ui->lwGunParticles->currentRow();
    if (iPart == -1) return;
    LocalRec.Particles[iPart].PreferredUnits = ui->cobUnits->currentText().toLatin1().data();
    updateParticleInfo();
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

    LocalRec.Phi = ui->ledGunPhi->text().toDouble();
    LocalRec.Theta = ui->ledGunTheta->text().toDouble();
    LocalRec.Psi = ui->ledGunPsi->text().toDouble();

    switch (ui->cobAngularMode->currentIndex())
    {
    case 0 : LocalRec.AngularMode = AParticleSourceRecord::UniformAngular;  break;
    case 1 : LocalRec.AngularMode = AParticleSourceRecord::FixedDirection;  break;
    case 2 : LocalRec.AngularMode = AParticleSourceRecord::GaussDispersion; break;
    case 3 : LocalRec.AngularMode = AParticleSourceRecord::CustomAngular;   break;
    default:
        qWarning() << "Unknown angular mode!";
        LocalRec.AngularMode = AParticleSourceRecord::UniformAngular;
    }
    LocalRec.DispersionSigma = ui->ledAngularSigma->text().toDouble();
    LocalRec.DirectionPhi = ui->ledGunCollPhi->text().toDouble();
    LocalRec.DirectionTheta = ui->ledGunCollTheta->text().toDouble();
    LocalRec.UseCutOff = ui->cbAngularCutoff->isChecked();
    LocalRec.CutOff = ui->ledAngularCutoff->text().toDouble();

    LocalRec.TimeAverageMode = ui->cobTimeAverageMode->currentIndex();
    LocalRec.TimeAverage = ui->ledTimeAverageFixed->text().toDouble();
    LocalRec.TimeAverageStart = ui->ledTimeAverageStart->text().toDouble();
    LocalRec.TimeAveragePeriod = ui->ledTimeAveragePeriod->text().toDouble();
    LocalRec.TimeSpreadMode = ui->cobTimeSpreadMode->currentIndex();
    LocalRec.TimeSpreadSigma = ui->ledTimeSpreadSigma->text().toDouble();
    LocalRec.TimeSpreadWidth = ui->ledTimeSpreadWidth->text().toDouble();

    int iPart = ui->lwGunParticles->currentRow();
    if (iPart >= 0)
    {
        AGunParticle & p = LocalRec.Particles[iPart];

        p.Particle = ui->leGunParticle->text().toLatin1().data();
        p.StatWeight = ui->ledGunParticleWeight->text().toDouble();
        p.UseFixedEnergy = (ui->cobEnergy->currentIndex() == 0);
        p.PreferredUnits = ui->cobUnits->currentText().toLatin1().data();
        double energy = ui->ledGunEnergy->text().toDouble();
        if      (p.PreferredUnits == "MeV") energy *= 1.0e3;
        else if (p.PreferredUnits == "keV") ;
        else if (p.PreferredUnits == "eV") energy *= 1.0e-3;
        else if (p.PreferredUnits == "meV") energy *= 1.0e-6;
        p.FixedEnergy = energy;
        p.RangeBasedEnergies = ui->cbRangeBaseEnergyData->isChecked();
// !!!***        p.Individual = !ui->cbLinkedParticle->isChecked();
        p.LinkedTo = ui->sbLinkedTo->value();
        p.LinkedProb = ui->ledLinkingProbability->text().toDouble();
        p.BtBPair = ui->cbLinkingOpposite->isChecked();
    }

    int curRow = ui->lwGunParticles->currentRow();
    updateListWidget();
    if ( curRow < 0 && curRow >= ui->lwGunParticles->count() )
        curRow = 0;
    ui->lwGunParticles->setCurrentRow(curRow);
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
    QString fileName = guitools::dialogLoadFile(this, "Load energy spectrum", "");
    if (fileName.isEmpty()) return;

    int iPart = ui->lwGunParticles->currentRow();
    QString err = ftools::loadPairs(fileName, LocalRec.Particles[iPart].EnergySpectrum, true);
    if (!err.isEmpty())
    {
        LocalRec.Particles[iPart].EnergySpectrum.clear();
        LocalRec.Particles[iPart].UseFixedEnergy = true;

        guitools::message(err, this);
    }

    bool ok = LocalRec.Particles[iPart].configureEnergySampler();
    if (!ok)
    {
        LocalRec.Particles[iPart].EnergySpectrum.clear();
        LocalRec.Particles[iPart].UseFixedEnergy = true;

        guitools::message("bad spectrum", this); // !!!*** use a generic check! (todo)
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

void AParticleSourceDialog::on_leGunParticle_editingFinished()
{
    on_pbUpdateRecord_clicked();
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
                       "  e-, proton, neutron, gamma, He3, etc.\n\n"
                       "For combined particle/optical simulations, the name can also be \"-\", \n"
                       "  which indicates direct energy deposition (a particle is NOT generated).\n"
                       "  Note that the direct energy deposition is only saved if the position is inside\n"
                       "  one of the sensitive volumes (See \"Settings\" tab)", "Particle name help", this);
}

void AParticleSourceDialog::on_cbRangeBaseEnergyData_clicked()
{
    int iPart = ui->lwGunParticles->currentRow();
    LocalRec.Particles[iPart].RangeBasedEnergies = ui->cbRangeBaseEnergyData->isChecked();
    LocalRec.Particles[iPart]._EnergySampler.configure(LocalRec.Particles[iPart].EnergySpectrum, LocalRec.Particles[iPart].RangeBasedEnergies);
    updateParticleInfo();
}

void AParticleSourceDialog::on_pbShowAngular_clicked()
{
    TGraph * gr = AGraphBuilder::graph(LocalRec.AngularDistribution);
    AGraphBuilder::configure(gr, "Angular distribution", "Angle, degrees", "");
    emit requestDraw(gr, "APL", true, true);
}

void AParticleSourceDialog::on_pbLoadAngular_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load angular distribution", "");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, LocalRec.AngularDistribution, true);
    if (!err.isEmpty())
    {
        LocalRec.AngularDistribution.clear();
        guitools::message(err, this);
    }

    bool ok = LocalRec.configureAngularSampler();
    if (!ok)
    {
        LocalRec.AngularDistribution.clear();
        guitools::message("bad angular", this); // !!!*** use a generic check! (todo)
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

void AParticleSourceDialog::on_cobGenerationType_currentIndexChanged(int index)
{
    ui->frIndependent->setVisible(index == 0);
    ui->frLinked->setVisible(index != 0);
}

