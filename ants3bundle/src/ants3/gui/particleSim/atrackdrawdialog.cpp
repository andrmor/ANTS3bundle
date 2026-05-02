#include "atrackdrawdialog.h"
#include "ui_atrackdrawdialog.h"
#include "atrackvisattributes.h"
#include "arootlineconfigurator.h"
#include "atrackvisattributes.h"
#include "guitools.h"
#include "a3global.h"

#include <QMenuBar>

#include "TColor.h"
#include "TROOT.h"

ATrackDrawDialog::ATrackDrawDialog(QWidget * parent) :
    QDialog(parent), settings(A3Global::getInstance().CurrentTrackVisAttributes),
    ui(new Ui::ATrackDrawProperties)
{
    setWindowTitle("Track visuals");
    ui->setupUi(this);
    ui->pbClose->setDefault(true);

    QMenuBar* mb = new QMenuBar(this);
    QMenu * fileMenu = mb->addMenu("File");
    fileMenu->addAction("Save", this, &ATrackDrawDialog::save);
    fileMenu->addAction("Load", this, &ATrackDrawDialog::load);
    QMenu * defMenu = mb->addMenu("Default");
    defMenu->addAction("Load default on this computer", this, &ATrackDrawDialog::loadDefault);
    defMenu->addSeparator();
    defMenu->addAction("Update default on this computer", this, &ATrackDrawDialog::updateDefault);
    defMenu->addAction("Set as default on this computer", this, &ATrackDrawDialog::setDefault);
    defMenu->addSeparator();
    defMenu->addAction("Load Ants3 default", this, &ATrackDrawDialog::resetAnts3Default);
    layout()->setMenuBar(mb);

    updateParticles(0);
    updateParticleAttributes();
}

ATrackDrawDialog::~ATrackDrawDialog()
{
    delete ui;
}

void ATrackDrawDialog::on_pbClose_clicked()
{
    accept();
}

void ATrackDrawDialog::updateParticles(int forceIndex)
{
    int index = (forceIndex == -1 ? ui->cobParticle->currentIndex() : forceIndex);
    ui->cobParticle->clear();
    ui->cobParticle->addItems(settings.getDefinedParticles());
    if (index >= ui->cobParticle->count()) index = 0;
    ui->cobParticle->setCurrentIndex(index);
}

void ATrackDrawDialog::updateParticleAttributes()
{
    const QString & particle = ui->cobParticle->currentText();

    const ATrackAttributes * att = settings.getAttributesForParticle(particle);

    if (att)
    {
        ui->frCustom->setVisible(true);
        TColor * tc = gROOT->GetColor(att-> Color);
        int red = 255;
        int green = 255;
        int blue = 255;
        if (tc)
        {
            red   = 255 * tc->GetRed();
            green = 255 * tc->GetGreen();
            blue  = 255 * tc->GetBlue();
        }
        ui->frCustom->setStyleSheet(  QString("background-color:rgb(%1,%2,%3)").arg(red).arg(green).arg(blue)  );

        return;
    }
    else ui->frCustom->setVisible(false); //undefined
}

void ATrackDrawDialog::on_pbDefaultParticleAtt_clicked()
{
    ARootLineConfigurator * rlc = new ARootLineConfigurator(settings.DefaultAttributes.Color,
                                                            settings.DefaultAttributes.Width,
                                                            settings.DefaultAttributes.Style, this);
    rlc->exec();
}

void ATrackDrawDialog::on_pbEditCustom_clicked()
{
    const QString & particle = ui->cobParticle->currentText();

    ATrackAttributes * s = settings.getAttributesForParticle(particle);
    ARootLineConfigurator * rlc = new ARootLineConfigurator(s->Color,
                                                            s->Width,
                                                            s->Style, this);
    rlc->exec();

    updateParticleAttributes();
}

#include <QFileDialog>
#include "ajsontools.h"
void ATrackDrawDialog::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save track drawing settings", "", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".json")) fileName += ".json";
    QJsonObject json;
    settings.writeToJson_particles(json);
    jstools::saveJsonToFile(json, fileName);
}

void ATrackDrawDialog::load()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load track drawing settings", "", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;
    QJsonObject json;
    jstools::loadJsonFromFile(json, fileName);
    settings.readFromJson_particles(json);

    updateParticles(-1);
    updateParticleAttributes();
}

void ATrackDrawDialog::loadDefault()
{
    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.CurrentTrackVisAttributes.importParticleAttributes(GlobSet.DefaultTrackVisAttributes);

    updateParticles(-1);
    updateParticleAttributes();
}

void ATrackDrawDialog::setDefault()
{
    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.DefaultTrackVisAttributes.importParticleAttributes(GlobSet.CurrentTrackVisAttributes);

    updateParticles(-1);
    updateParticleAttributes();
}

void ATrackDrawDialog::updateDefault()
{
    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.DefaultTrackVisAttributes.importAndMergeParticleAttributes(GlobSet.CurrentTrackVisAttributes);

    updateParticles(-1);
    updateParticleAttributes();
}

void ATrackDrawDialog::resetAnts3Default()
{
    A3Global & GlobSet = A3Global::getInstance();
    GlobSet.CurrentTrackVisAttributes.clearParticleProps();

    updateParticles(-1);
    updateParticleAttributes();
}

void ATrackDrawDialog::on_cobParticle_activated(int)
{
    updateParticleAttributes();
}

void ATrackDrawDialog::on_pbNew_clicked()
{
    QString name;
    guitools::inputString("Enter particle name", name, this);
    if (name.isEmpty()) return;

    ATrackAttributes * s = settings.getAttributesForParticle(name);
    if (s)
    {
        guitools::message("Particle " + name + " already has defined attributes", this);
        ui->cobParticle->setCurrentText(name);
        updateParticleAttributes();
        return;
    }

    settings.defineAttributesForParticle(name, settings.DefaultAttributes);
    updateParticles();
    ui->cobParticle->setCurrentText(name);
    updateParticleAttributes();
}

void ATrackDrawDialog::on_pbRemove_clicked()
{
    const QString name = ui->cobParticle->currentText();
    if (name.isEmpty()) return;

    bool ok = guitools::confirm("Remove attributes for particle " + name, this);
    if (!ok) return;

    settings.removeCustom(name);
    updateParticles();
    updateParticleAttributes();
}

