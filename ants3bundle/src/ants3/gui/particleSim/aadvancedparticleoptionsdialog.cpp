#include "aadvancedparticleoptionsdialog.h"
#include "ui_aadvancedparticleoptionsdialog.h"
#include "aparticlesimhub.h"

AAdvancedParticleOptionsDialog::AAdvancedParticleOptionsDialog(QWidget * parent) :
    QDialog(parent), ui(new Ui::AAdvancedParticleOptionsDialog), Settings(AParticleSimHub::getInstance().Settings)
{
    ui->setupUi(this);

    setWindowTitle("Advanced settings");

    ui->cbAcolin->setChecked(Settings.G4Set.SimulateAnnihilAcolinearity);
    ui->cobAcolModel->setCurrentIndex(Settings.G4Set.AcolinearityModel == 0 ? 0 : 1);
    ui->ledAcolFWHM->setText(QString::number(Settings.G4Set.AcolinearityFWHM));
    QString txt;
    for (const std::string & str : Settings.G4Set.AcolinearityVolumes)
        txt += str + ", ";
    if (txt.endsWith(", ")) txt.resize(txt.size() - 2);
    ui->leAcolinVolumes->setText(txt);

    QList<QPushButton*> pbList = this->findChildren<QPushButton*>();
    foreach(QPushButton * pb, pbList) {pb->setDefault(false); pb->setAutoDefault(false);}
}

AAdvancedParticleOptionsDialog::~AAdvancedParticleOptionsDialog()
{
    delete ui;
}

#include <qregularexpression.h>
#include "guitools.h"
#include "ageometryhub.h"
#include "ageoobject.h"
void AAdvancedParticleOptionsDialog::on_pClose_clicked()
{
    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\n|\\t)"); //separators: ' ' or ',' or '\n' or '\t'
    QString AcolVols = ui->leAcolinVolumes->text();
    QStringList acolVolList = AcolVols.split(rx, Qt::SkipEmptyParts);

    // checks
    if (ui->cbAcolin->isChecked())
    {
        if (ui->ledAcolFWHM->text().toDouble() < 0)
        {
            guitools::message("Acolinearity angle FWHM value cannot be negative", this);
            return;
        }
        if (AcolVols.simplified().isEmpty())
        {
            guitools::message("You have to provide at least one volume where to simulate acolinearity", this);
            return;
        }
        QString worldName = AGeometryHub::getInstance().World->Name;
        for (QString & str : acolVolList)
        {
            QString thisOne = str.simplified();
            if (thisOne.endsWith('*'))
            {
                thisOne.chop(1);
                if (worldName.startsWith(thisOne))
                {
                    guitools::message("Cannot simulate acolinearity in the entire world: world objectname cannot be included", this);
                    return;
                }
            }
            else
            {
                if (thisOne == worldName)
                {
                    guitools::message("Cannot simulate acolinearity in the entire world: world objectname cannot be included", this);
                    return;
                }
            }
        }
    }

    Settings.G4Set.SimulateAnnihilAcolinearity = ui->cbAcolin->isChecked();
    Settings.G4Set.AcolinearityModel = ui->cobAcolModel->currentIndex();
    Settings.G4Set.AcolinearityFWHM =  ui->ledAcolFWHM->text().toDouble();

    Settings.G4Set.AcolinearityVolumes.clear();
    for (QString & str : acolVolList)
        Settings.G4Set.AcolinearityVolumes.push_back(str.simplified().toLatin1().data());

    accept();
}

void AAdvancedParticleOptionsDialog::on_pbHelpAcolin_clicked()
{
    QString txt;

    txt += "Introduce acolinearity of annihilation gammas (Geant4 v11 does not do it)\n";
    txt += "\nAvailable models:\n";
    txt += "1) Gaussian distribution of the _magnitude_ of the angle deviation from back-to-back direction\n";
    txt += "2) Gaussian distribution independent for each direction components:";
    txt += " see https://doi.org/10.1088/1361-6560/ad70f1\n";
    txt += "\nFWHM: Gaussian width in degrees\n";
    txt += "\nThe model is applied inside the specified volume(s) and recursive for all daughter volumes\n";
    txt += "The volume names can end with '*', signififying 'starts with'\n";
    txt += "and separated with space or comma.\n";
    txt += "\n";
    txt += "\nThe model is applied at the first tracing step of the affected gamma, ";
    txt += "and in the tracking history the direction change is indicated with 'P' and 'P->'.\n";
    txt += "\nThe model is triggered for a gamma with energy in range of [0.510, 0.512] MeV ";
    txt += "when previously a gamma with such energy was already tracked and:\n";
    txt += "1) both gammas are either primary or have the same parent;\n";
    txt += "2) both gammas have the same timestamp and emission position;\n";
    txt += "3) they have exactly opposite momentum directions.\n";
    txt += "\nAs in Geant4 the order of particle tracking is reveresed (last added tracked first), ";
    txt += "the first gamma of the pair receives the change of direction.\n";
    txt += "\nNote that the Physics List is not modified (FastSimPhysics is used)\n";
    txt += "\nThis model is applicable both for back-to-back gammas generated with an Ants3 particle source ";
    txt += "and for gammas generated in annihilations of positrons (using, e.g., source emitting F18 isotopes).";

    guitools::message(txt, this);
}

