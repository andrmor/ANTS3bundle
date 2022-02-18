#include "asensorwindow.h"
#include "ui_asensorwindow.h"
#include "asensorhub.h"
#include "asensormodel.h"
#include "guitools.h"

#include <QDoubleValidator>

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
    int iModel = ui->cobModel->currentIndex();

    ui->cobModel->clear();
    ui->cobModel->addItems(SensHub.getListOfModelNames());

    if (iModel < 0 || iModel >= SensHub.countSensorModels()) iModel = 0;
    ui->cobModel->setCurrentIndex(iModel);
    on_cobModel_activated(iModel);
}

void ASensorWindow::on_cobModel_activated(int index)
{
    if (index == -1) return;
    ASensorModel * mod = SensHub.model(index);
    if (!mod)
    {
        guitools::message("This sensor model does not exist!", this);
        return;
    }
    ui->labModelIndex->setText(QString::number(index));
    ui->labNumSensors->setText(QString::number(SensHub.countSensorsOfModel(index)));

    ui->leModelName->setText(mod->Name);

    ui->ledEffectivePDE->setText( QString::number(mod->PDE_effective) );

    bool bHaveSpectral = (mod->PDE_spectral.size() > 0);
    ui->pbShowPDE->setEnabled(bHaveSpectral);
    ui->pbRemovePDE->setEnabled(bHaveSpectral);
    ui->pbShowBinnedPDE->setEnabled(bHaveSpectral);

    ui->cobSensorType->setCurrentIndex(mod->SiPM ? 1 : 0);
    ui->sbPixelsX->setValue(mod->PixelsX);
    ui->sbPixelsY->setValue(mod->PixelsY);
    updateNumPixels();
}

void ASensorWindow::on_cobSensorType_currentIndexChanged(int index)
{
    ui->frSiPM->setEnabled(index == 1);
}

void ASensorWindow::on_pbAddNewModel_clicked()
{
    SensHub.addNewModel();
    updateGui();
    int index = SensHub.countSensorModels() - 1;
    ui->cobModel->setCurrentIndex(index);
    on_cobModel_activated(index);
}

void ASensorWindow::on_pbCloneModel_clicked()
{
    SensHub.cloneModel(ui->cobModel->currentIndex());
    updateGui();
    int index = SensHub.countSensorModels() - 1;
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

