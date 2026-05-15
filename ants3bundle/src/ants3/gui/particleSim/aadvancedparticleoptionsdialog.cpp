#include "aadvancedparticleoptionsdialog.h"
#include "ui_aadvancedparticleoptionsdialog.h"
#include "aparticlesimhub.h"

AAdvancedParticleOptionsDialog::AAdvancedParticleOptionsDialog(QWidget * parent) :
    QDialog(parent), ui(new Ui::AAdvancedParticleOptionsDialog), Settings(AParticleSimHub::getInstance().Settings)
{
    ui->setupUi(this);

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

