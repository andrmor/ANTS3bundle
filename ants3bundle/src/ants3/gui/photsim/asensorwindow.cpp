#include "asensorwindow.h"
#include "ui_asensorwindow.h"
#include "asensorhub.h"
#include "asensormodel.h"
#include "guitools.h"
#include "afiletools.h"
#include "agraphbuilder.h"

#include <QDoubleValidator>

#include "TGraph.h"

ASensorWindow::ASensorWindow(QWidget *parent) :
    AGuiWindow("Sens", parent),
    SensHub(ASensorHub::getInstance()),
    ui(new Ui::ASensorWindow)
{
    ui->setupUi(this);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit *>();
    foreach(QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    updateGui();
}

ASensorWindow::~ASensorWindow()
{
    delete ui;
}

void ASensorWindow::updateGui()
{
    updateHeader();

    int iModel = ui->cobModel->currentIndex();

    ui->cobModel->clear();
    ui->cobModel->addItems(SensHub.getListOfModelNames());

    if (iModel < 0 || iModel >= SensHub.countModels()) iModel = 0;
    ui->cobModel->setCurrentIndex(iModel);
    on_cobModel_activated(iModel);

    ui->cobAssignmentMode->setCurrentIndex(SensHub.isPersistentModelAssignment() ? 1 : 0);

    updatePdeButtons();
}

void ASensorWindow::on_cobModel_activated(int index)
{
    if (index == -1) return;

    ui->sbModelIndex->setValue(index);
    onModelIndexChanged();
}

void ASensorWindow::on_sbModelIndex_editingFinished()
{
    const int index = ui->sbModelIndex->value();
    if (index >= ui->cobModel->count())
        guitools::message("Mismatch on number of models!", this);
    else ui->cobModel->setCurrentIndex(index);

    onModelIndexChanged();
}

void ASensorWindow::onModelIndexChanged()
{
    const int index = ui->sbModelIndex->value();
    ASensorModel * mod = SensHub.model(index);
    if (!mod)
    {
        guitools::message("This sensor model does not exist!", this);
        return;
    }

    const int numInUse = SensHub.countSensorsOfModel(index);
    ui->labNumSensorsThisModel->setText(QString::number(numInUse));
    ui->pbRemoveModel->setEnabled( numInUse == 0 );

    ui->leModelName->setText(mod->Name);

    ui->ledEffectivePDE->setText( QString::number(mod->PDE_effective) );

    updatePdeButtons();

    ui->cobSensorType->setCurrentIndex(mod->SiPM ? 1 : 0);
    ui->sbPixelsX->setValue(mod->PixelsX);
    ui->sbPixelsY->setValue(mod->PixelsY);
    updateNumPixels();
}

void ASensorWindow::updateHeader()
{
    ui->labNumSensors->setText( QString::number(SensHub.countSensors()) );
    ui->labNumModels->setText( QString::number(SensHub.countModels()) );
}

void ASensorWindow::on_cobSensorType_currentIndexChanged(int index)
{
    ui->frSiPM->setEnabled(index == 1);
}

void ASensorWindow::on_pbAddNewModel_clicked()
{
    SensHub.addNewModel();
    updateGui();
    int index = SensHub.countModels() - 1;
    ui->cobModel->setCurrentIndex(index);
    on_cobModel_activated(index);
}

void ASensorWindow::on_pbCloneModel_clicked()
{
    SensHub.cloneModel(ui->cobModel->currentIndex());
    updateGui();
    int index = SensHub.countModels() - 1;
    ui->cobModel->setCurrentIndex(index);
    on_cobModel_activated(index);
}

void ASensorWindow::on_pbRemoveModel_clicked()
{
    int iModel = ui->cobModel->currentIndex();

    bool ok = guitools::confirm("Remove this sensor model?", this);
    if (!ok) return;

    QString err = SensHub.removeModel(iModel);
    if (err.isEmpty()) updateGui();
    else guitools::message(err, this);
}

void ASensorWindow::on_leModelName_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->Name = ui->leModelName->text();
}

void ASensorWindow::on_ledEffectivePDE_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    double pde = ui->ledEffectivePDE->text().toDouble();
    if (pde < 0 || pde > 1.0)
    {
        guitools::message("PDE should be in the range from 0 to 1.0", this);
        ui->ledEffectivePDE->setText(QString::number(mod->PDE_effective));
        return;
    }
    mod->PDE_effective = pde;
}

void ASensorWindow::on_cobSensorType_activated(int index)
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->SiPM = (index == 1);
}

void ASensorWindow::on_sbPixelsX_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PixelsX = ui->sbPixelsX->value();
    updateNumPixels();
}
void ASensorWindow::on_sbPixelsY_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PixelsY = ui->sbPixelsY->value();
    updateNumPixels();
}

void ASensorWindow::updateNumPixels()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    int num = mod->PixelsX * mod->PixelsY;
    ui->labNumPixels->setText(QString::number(num));
}

#include "aconfig.h"
void ASensorWindow::on_cobAssignmentMode_activated(int index)
{
    if (index == 1)
    {
        guitools::message("The mode will change to \"Custom\" automatically\nas soon as any sensor assignment\nis modified by script!", this);
        ui->cobAssignmentMode->setCurrentIndex(0);
    }
    else
    {
        SensHub.exitPersistentMode();

        AConfig & Config = AConfig::getInstance();
        Config.updateJSONfromConfig();
        Config.updateConfigFromJSON();
    }
}

void ASensorWindow::on_pbShowSensorsOfThisModel_clicked()
{
    emit requestShowSensorModels(ui->sbModelIndex->value());
}

void ASensorWindow::updatePdeButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->PDE_spectral.empty();

    ui->pbShowPDE->setEnabled(enabled);
    ui->pbRemovePDE->setEnabled(enabled);
    ui->pbShowBinnedPDE->setEnabled(enabled);
}

void ASensorWindow::on_pbLoadPDE_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    QString fname =guitools::dialogLoadFile(this, "Load spectral PDE", "");
    if (fname.isEmpty()) return;

    std::vector<std::pair<double,double>> data;
    QString err = ftools::loadPairs(fname, data, true);
    if (err.isEmpty())
    {
        mod->PDE_spectral = data;
        updatePdeButtons();
    }
    else guitools::message(err, this);
}

void ASensorWindow::on_pbRemovePDE_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PDE_spectral.clear();
    updatePdeButtons();
}

void ASensorWindow::on_pbShowPDE_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->PDE_spectral.empty()) return;

    TGraph * gr = AGraphBuilder::graph(mod->PDE_spectral);
    AGraphBuilder::configure(gr, QString("PDE for model%0").arg(iModel), "Wavelength, nm", "");
    emit requestDraw(gr, "APL", true, true);
}

#include "aphotonsimhub.h"
void ASensorWindow::on_pbShowBinnedPDE_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->PDE_spectral.empty()) return;

    mod->updateRuntimeProperties();

    const APhotonSimSettings SimSet = APhotonSimHub::getConstInstance().Settings;
    std::vector<double> wave;
    SimSet.WaveSet.getWavelengthBins(wave);

    TGraph * gr = AGraphBuilder::graph(wave, mod->PDEbinned);
    AGraphBuilder::configure(gr, QString("Binned PDE, model%0").arg(iModel), "Wavelength, nm", "PDE", 4, 20, 1, 4);
    emit requestDraw(gr, "APL", true, true);
}

