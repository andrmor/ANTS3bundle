#include "asensorwindow.h"
#include "ui_asensorwindow.h"
#include "asensorhub.h"
#include "asensormodel.h"
#include "guitools.h"
#include "afiletools.h"
#include "agraphbuilder.h"
#include "aphotonsimhub.h"
#include "amaterialhub.h"

#include <QDoubleValidator>
#include <QDebug>

#include "TGraph.h"
#include "TH2D.h"

ASensorWindow::ASensorWindow(QWidget *parent) :
    AGuiWindow("Sens", parent),
    SensHub(ASensorHub::getInstance()),
    ui(new Ui::ASensorWindow)
{
    ui->setupUi(this);

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit *>();
    foreach(QLineEdit * w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    QDoubleValidator * dvp = new QDoubleValidator(this);
    dvp->setNotation(QDoubleValidator::ScientificNotation);
    dvp->setBottom(0);
    foreach(QLineEdit * w, list) if (w->objectName().startsWith("lep")) w->setValidator(dvp);

    ui->cbGains_ShowTable->setChecked(false);

    ui->frSiPM->setVisible(ui->cobSensorType->currentIndex() == 1);

    on_cobPDEmodel_currentIndexChanged(ui->cobPDEmodel->currentIndex());

    CellValidator = new QDoubleValidator(this);
    CellValidator->setBottom(0);

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

    updateModelGui();

    ui->cobAssignmentMode->setCurrentIndex(SensHub.isPersistentModelAssignment() ? 1 : 0);

    updateGains();
}

void ASensorWindow::updateModelGui()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel); // can be nullptr

    if (mod)
    {
        ui->leModelName->setText(mod->Name);

        ui->cobSensorType->setCurrentIndex(mod->SiPM ? 1 : 0);
        ui->sbPixelsX->setValue(mod->PixelsX);
        ui->sbPixelsY->setValue(mod->PixelsY);
        ui->lepPixelSizeX->setText(QString::number(mod->PixelSizeX));
        ui->lepPixelSizeY->setText(QString::number(mod->PixelSizeY));
        ui->lepPixelSpacingX->setText(QString::number(mod->PixelSpacingX));
        ui->lepPixelSpacingY->setText(QString::number(mod->PixelSpacingY));
        updateNumPixels();

        int modIndex = mod->PDE_model;
        if (modIndex < 0 || modIndex >= ui->cobPDEmodel->count())
        {
            qWarning() << "Unknown PDE model index" << modIndex;
            modIndex = 0;
        }
        ui->cobPDEmodel->setCurrentIndex(modIndex);

        ui->ledEffectivePDE->setText( QString::number(mod->PDE_effective) );

        ui->ledAngularWave->setText( QString::number(mod->Angular_Wavelength) );

        ui->lepAreaStepX->setText(QString::number(mod->StepX));
        ui->lepAreaStepY->setText(QString::number(mod->StepY));

        ui->lepDarkRate->setText(QString::number(mod->DarkCountRate));
        ui->lepIntegrationTime->setText(QString::number(mod->IntegrationTime));

        ui->lepElNoiseSigma->setText(QString::number(mod->ElectronicNoiseSigma));

        int index = 0;
        switch (mod->PhElToSignalModel)
        {
        case ASensorModel::Constant : index = 0; break;
        case ASensorModel::Normal   : index = 1; break;
        case ASensorModel::Gamma    : index = 2; break;
        case ASensorModel::Custom   : index = 3; break;
        default: qWarning() << "Unknown ph.e- to signal model!\nSwitching to \"Constant\"";
        }
        ui->cobSignalModel->setCurrentIndex(index);
        on_cobSignalModel_currentIndexChanged(index);
        //ui->lepElGainFactor->setText(QString::number(mod->ElectronicGainFactor));
        ui->lepAverageSignalPerPhE->setText(QString::number(mod->AverageSignalPerPhEl));
        ui->lepNormalSigma->setText(QString::number(mod->NormalSigma));
        ui->lepGammaShape->setText(QString::number(mod->GammaShape));
    }

    updatePdeButtons();
    updateAngularButtons();
    updateAreaButtons();

    updatePhElToSigButtons();
}

void ASensorWindow::on_cobModel_activated(int)
{
    onModelIndexChanged();
}

void ASensorWindow::onModelIndexChanged()
{
    int index = ui->cobModel->currentIndex();
    if (index == -1) return;

    int numInUse = SensHub.countSensorsOfModel(index);
    ui->labNumSensorsThisModel->setText(QString::number(numInUse));
    ui->pbRemoveModel->setEnabled( numInUse == 0 );

    updateModelGui();
}

void ASensorWindow::updateHeader()
{
    ui->labNumSensors->setText( QString::number(SensHub.countSensors()) );
    ui->labNumModels->setText( QString::number(SensHub.countModels()) );

    ui->labDefinedGains->setVisible(SensHub.UseSensorGains);
    ui->labNumGains->setVisible(SensHub.UseSensorGains);
    if (SensHub.UseSensorGains)
    {
        ui->labNumGains->setText( QString::number(SensHub.SensorGains.size()) );
        ui->labGainMissmatch->setVisible( SensHub.countSensors() != SensHub.SensorGains.size() );
        ui->tabWidget->setTabIcon(1, SensHub.countSensors() == SensHub.SensorGains.size() ? QIcon() : guitools::createColorCircleIcon(ui->tabWidget->iconSize(), Qt::red) );
    }
    else
    {
        ui->tabWidget->setTabIcon(1, QIcon());
        ui->labGainMissmatch->setVisible(false);
    }

}

void ASensorWindow::on_cobSensorType_currentIndexChanged(int index)
{
    ui->frSiPM->setVisible(index == 1);
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
void ASensorWindow::on_lepPixelSizeX_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    double val = ui->lepPixelSizeX->text().toDouble();
    if (val <= 0)
    {
        ui->lepPixelSizeX->setText(QString::number(mod->PixelSizeX));
        guitools::message("Value should be positive", this);
    }
    else mod->PixelSizeX = val;
}
void ASensorWindow::on_lepPixelSizeY_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    double val = ui->lepPixelSizeY->text().toDouble();
    if (val <= 0)
    {
        ui->lepPixelSizeY->setText(QString::number(mod->PixelSizeX));
        guitools::message("Value should be positive", this);
    }
    else mod->PixelSizeY = val;
}
void ASensorWindow::on_lepPixelSpacingX_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PixelSpacingX = ui->lepPixelSpacingX->text().toDouble();
}
void ASensorWindow::on_lepPixelSpacingY_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PixelSpacingY = ui->lepPixelSpacingY->text().toDouble();
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
        Config.updateConfigFromJSON(true);
    }
}

void ASensorWindow::on_pbShowSensorsOfThisModel_clicked()
{
    emit requestShowSensorModels(ui->cobModel->currentIndex());
}

void ASensorWindow::updatePdeButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->PDE_spectral.empty();

    ui->pbShowPDE->setEnabled(enabled);
    ui->pbRemovePDE->setEnabled(enabled);
}

void ASensorWindow::updateAngularButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->AngularFactors.empty();

    ui->pbShowAngular->setEnabled(enabled);
    ui->pbRemoveAngular->setEnabled(enabled);
    ui->frAngularWave->setVisible(enabled && ui->cobPDEmodel->currentIndex() == 1);
}

void ASensorWindow::updateAreaButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->AreaFactors.empty();

    ui->pbShowArea->setEnabled(enabled);
    ui->pbRemoveArea->setEnabled(enabled);
    ui->frAreaSteps->setVisible(enabled);
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
        for (const auto & pair : data)
            if (pair.second > 1.0)
            {
                guitools::message("Photon detection efficinecy should be in the range from 0 to 1", this);
                return;
            }

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
    AGraphBuilder::configure(gr, QString("PDE for model%0").arg(iModel), "Wavelength, nm", "PDE");
    gr->SetMinimum(0);
    emit requestDraw(gr, "APL", true, true);
}

void ASensorWindow::on_pbShowAngular_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->AngularFactors.empty()) return;

    TGraph * gr = AGraphBuilder::graph(mod->AngularFactors);
    AGraphBuilder::configure(gr, QString("Angular sensitivity for model%0").arg(iModel), "Incidence angle, deg", "");
    emit requestDraw(gr, "APL", true, true);
}
void ASensorWindow::on_pbLoadAngular_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    QString fname = guitools::dialogLoadFile(this, "Load sensitivity vs angle of incidence [deg].\nShould start from 0 and end with 90 degrees!", "");
    if (fname.isEmpty()) return;

    std::vector<std::pair<double,double>> data;
    QString err = ftools::loadPairs(fname, data, true);
    if (err.isEmpty())
    {
        if (data.front().first != 0)    err = "Data should start from zero degrees";
        if (data.back(). first != 90.0) err = "Data should end with 90 degrees";
        else
        {
            mod->AngularFactors = data;
            updateAngularButtons();
            return;
        }
    }
    guitools::message(err, this);
}
void ASensorWindow::on_pbRemoveAngular_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->AngularFactors.clear();
    updateAngularButtons();
}

void ASensorWindow::on_pbShowArea_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->AreaFactors.empty()) return;

    const size_t xNum = mod->AreaFactors.front().size();
    const size_t yNum = mod->AreaFactors.size();
    const double & xStep = mod->StepX;
    const double & yStep = mod->StepY;

    TH2D * hist2D = new TH2D("", "AreaFactors", xNum, -0.5 * xNum * xStep, 0.5 * xNum * xStep,     yNum, -0.5 * yNum * yStep, 0.5 * yNum * yStep);
    for (size_t iY = 0; iY < yNum; iY++)
        for (size_t iX = 0; iX < xNum; iX++)
            hist2D->Fill(-0.5 * xNum * xStep + (0.5 + iX) * xStep, -0.5 * yNum * yStep + (0.5 + iY) * yStep, mod->AreaFactors[yNum-1 - iY][iX]);

    hist2D->SetXTitle("X, mm");
    hist2D->SetYTitle("Y, mm");
    emit requestDraw(hist2D, "colz", true, true);
}
void ASensorWindow::on_pbLoadArea_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    QString fname = guitools::dialogLoadFile(this, "Load matrix of area factors", "");
    if (fname.isEmpty()) return;

    std::vector<std::vector<double>> data;
    QString err = ftools::loadMatrix(fname, data);
    if (err.isEmpty()) mod->AreaFactors = data;
    else guitools::message(err, this);
    // note that the min index of y is on top of the matrix!
    updateAreaButtons();
}
void ASensorWindow::on_pbRemoveArea_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->AreaFactors.clear();
    updateAreaButtons();
}
void ASensorWindow::on_lepAreaStepX_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->StepX = ui->lepAreaStepX->text().toDouble();
}
void ASensorWindow::on_lepAreaStepY_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->StepY = ui->lepAreaStepY->text().toDouble();
}

void ASensorWindow::on_pbShowPixelMap_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

     double dotsPerMin = ui->sbShowResolution->value();

     double & PX = mod->PixelSizeX;
     double & PY = mod->PixelSizeY;
     double & dX = mod->PixelSpacingX;
     double & dY = mod->PixelSpacingY;

     double minX = std::min(PX, dX); if (minX == 0) minX = PX;
     double minY = std::min(PY, dY); if (minY == 0) minY = PY;

     double scaleX = minX / dotsPerMin; // mm per dot
     double scaleY = minY / dotsPerMin; // mm per dot

     double fullSizeX = PX * mod->PixelsX + dX * (mod->PixelsX-1);
     double fullSizeY = PY * mod->PixelsY + dY * (mod->PixelsY-1);

     int numX = fullSizeX/scaleX + 1;
     int numY = fullSizeY/scaleY + 1;

     //qDebug() << std::fmod(6.3, 2); shows 0.3

     TH2D * h = new TH2D("", "", numX, -0.5*fullSizeX, 0.5*fullSizeX,   numY, -0.5*fullSizeY, 0.5*fullSizeY);

     mod->updateRuntimeProperties(std::vector<int>()); // to enable mod->getPixelHit()

     size_t binX = -1;
     size_t binY = -1;
     size_t oldBinX = -2;
     size_t oldBinY = -2;
     bool bFlag = true;
     bool bFlagOnLineStart = true;
     for (int iy = 0; iy < numY; iy++)
     {
         const double y = iy * scaleY - 0.5*fullSizeY;
         for (int ix = 0; ix < numX; ix++)
         {
             const double x = ix * scaleX - 0.5*fullSizeX;
             const bool isPixel = mod->getPixelHit(x, y, binX, binY);
             //qDebug() << "binY, binX:" << binY << binX;
             if (!isPixel) continue;

             if (binX != oldBinX)
             {
                 bFlag = !bFlag;
                 oldBinX = binX;
             }

             if (binX == 0)
             {
                 if (binY != oldBinY) // new pixel in the line start!
                 {
                     if (bFlag == bFlagOnLineStart) bFlag = !bFlag;
                     bFlagOnLineStart = bFlag;
                     oldBinY = binY;
                 }
                 else bFlag = bFlagOnLineStart;
             }

             h->Fill(x, y, (bFlag ? 5 : 1) );
         }
     }

     h->SetXTitle("Local X, mm");
     h->SetYTitle("Local Y, mm");
     emit requestDraw(h, "col", true, true);
}

void ASensorWindow::on_lepDarkRate_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->DarkCountRate = ui->lepDarkRate->text().toDouble();
}

void ASensorWindow::on_lepIntegrationTime_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->IntegrationTime = ui->lepIntegrationTime->text().toDouble();
}

void ASensorWindow::on_lepElNoiseSigma_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->ElectronicNoiseSigma = ui->lepElNoiseSigma->text().toDouble();
}

void ASensorWindow::on_cobSignalModel_currentIndexChanged(int index)
{
    ui->swSignalModel->setCurrentIndex(index);
    ui->frAverageSignalPerPhEl->setHidden(index == 3);
    ui->swSignalModel->setHidden(index == 0);
}

void ASensorWindow::on_cobSignalModel_activated(int index)
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    switch (index)
    {
    default: qWarning() << "Non-implemented value for PhElToSignalModel, assumong \"Constant\""; // fall-through
    case 0 : mod->PhElToSignalModel = ASensorModel::Constant; break;
    case 1 : mod->PhElToSignalModel = ASensorModel::Normal;   break;
    case 2 : mod->PhElToSignalModel = ASensorModel::Gamma;    break;
    case 3 : mod->PhElToSignalModel = ASensorModel::Custom;   break;
    }
}

/*
void ASensorWindow::on_lepElGainFactor_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->ElectronicGainFactor = ui->lepElGainFactor->text().toDouble();
}
*/

void ASensorWindow::on_lepAverageSignalPerPhE_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->AverageSignalPerPhEl = ui->lepAverageSignalPerPhE->text().toDouble();
}

void ASensorWindow::on_lepNormalSigma_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->NormalSigma = ui->lepNormalSigma->text().toDouble();
}

void ASensorWindow::on_lepGammaShape_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->GammaShape = ui->lepGammaShape->text().toDouble();
}

void ASensorWindow::on_pbLoadCustomPhElSig_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    QString fname = guitools::dialogLoadFile(this, "Load custom pulse height distribution for single ph.e-", "");
    if (fname.isEmpty()) return;

    std::vector<std::pair<double,double>> data;
    QString err = ftools::loadPairs(fname, data, true);
    if (err.isEmpty())
    {
        ASensorModel dummy;
        dummy.SinglePhElPHS = data;
        err = dummy.checkPhElToSignals();
        if (err.isEmpty())
        {
            mod->SinglePhElPHS = data;
            updatePhElToSigButtons();
            return;
        }
    }
    guitools::message(err, this);
}

void ASensorWindow::on_pbShowCustomPhElSig_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->SinglePhElPHS.empty()) return;

    TGraph * gr = AGraphBuilder::graph(mod->SinglePhElPHS);
    AGraphBuilder::configure(gr, QString("Single ph.e- PHS for model%0").arg(iModel), "Sensor signal", "");
    emit requestDraw(gr, "APL", true, true);
}

void ASensorWindow::on_pbRemoveCustomPhElSig_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->SinglePhElPHS.clear();
    updatePhElToSigButtons();
}

void ASensorWindow::updatePhElToSigButtons()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    ui->pbShowCustomPhElSig->setDisabled(mod->SinglePhElPHS.empty());
    ui->pbRemoveCustomPhElSig->setDisabled(mod->SinglePhElPHS.empty());
}

void ASensorWindow::updateGains()
{
    ui->cbGains->setChecked(SensHub.UseSensorGains);

    if (ui->cbGains_ShowTable->isChecked()) showTableWithGains();
}

#include <QDialog>
#include <QSpinBox>
#include <QDoubleValidator>
static int    lastSelectedTimes = 100000;
static double lastSelectedPhEl = 1.0;
static double lastSelectedGain = 1.0;
void ASensorWindow::on_pbTestPhElSignal_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    QString err = mod->updateRuntimeProperties(std::vector<int>());
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    QDialog * dialog = new QDialog(this);
    dialog->setWindowTitle("Ph.e-  to signal convertion tester");
    QVBoxLayout * vlay = new QVBoxLayout(dialog);
    QPushButton * pb = new QPushButton("Generate");
    vlay->addWidget(pb);
    QHBoxLayout * lay = new QHBoxLayout();
        lay->addWidget(new QLabel("signal for"));
        QLineEdit * lePE = new QLineEdit(QString::number(lastSelectedPhEl)); lePE->setMaximumWidth(80);
        QDoubleValidator * dv = new QDoubleValidator(dialog); dv->setBottom(0); lePE->setValidator(dv);
        lay->addWidget(lePE);
        lay->addWidget(new QLabel("ph.e-"));
        QSpinBox * sbTimes = new QSpinBox(); sbTimes->setMaximum(1e9); sbTimes->setMinimum(1); sbTimes->setValue(lastSelectedTimes);
        lay->addWidget(sbTimes);
        lay->addWidget(new QLabel("times"));
        lay->addStretch();
    vlay->addLayout(lay);
    QHBoxLayout * layGain = new QHBoxLayout();
        layGain->addWidget(new QLabel("assume sensor gain of"));
        QLineEdit * ledGain = new QLineEdit(QString::number(lastSelectedGain)); ledGain->setMaximumWidth(80);
            ledGain->setValidator(dv);
        layGain->addWidget(ledGain);
        layGain->addStretch();
    vlay->addLayout(layGain);

    auto click = [this, mod, dialog, lePE, sbTimes, ledGain]()
    {
        lastSelectedPhEl = lePE->text().toDouble();
        lastSelectedTimes = sbTimes->value();
        lastSelectedGain = ledGain->text().toDouble();
        auto hist1D = new TH1D("", "Signal distribution", 100, 0, 0);
        for (int i = 0; i < lastSelectedTimes; i++)
            hist1D->Fill(mod->convertHitsToSignal(lastSelectedPhEl) * lastSelectedGain);
        hist1D->SetXTitle("Sensor signal");
        emit requestDraw(hist1D, "hist", true, true);

        dialog->accept();
    };
    connect(pb, &QPushButton::clicked, this, click);

    dialog->setMinimumWidth(width());
    dialog->show();
    dialog->move(dialog->x(), QCursor::pos().y() - 15);
    dialog->exec();
}

#include "amaterialhub.h"
void ASensorWindow::on_pbCompteEffectivePDE_clicked()
{
    int iSensorModel = ui->cobModel->currentIndex();
    if (iSensorModel < 0 || iSensorModel >= SensHub.countModels())
    {
        guitools::message("Sensor model does not exist!", this);
        return;
    }
    if (SensHub.model(iSensorModel)->PDE_spectral.empty())
    {
        guitools::message("Wavelength-resolved PDE is not defined for this sensor model", this);
        return;
    }

    QDialog dia(this);
    dia.setWindowTitle("Estimate effective PDE");
    QVBoxLayout * mainLay = new QVBoxLayout(&dia);
        QHBoxLayout * lay = new QHBoxLayout();
            lay->addWidget(new QLabel("Primary scintillation from"));
            QComboBox * cobMats = new QComboBox();
            cobMats->addItems(AMaterialHub::getInstance().getListOfMaterialNames());
            lay->addWidget(cobMats);
        mainLay->addLayout(lay);
            QPushButton * pbEst = new QPushButton("Compute mean PDE");
            connect(pbEst, &QPushButton::clicked, &dia, &QDialog::accept);
        mainLay->addWidget(pbEst);

    int res = dia.exec();
    if (res == QDialog::Rejected) return;

    int iMat = cobMats->currentIndex();
    AMaterialHub & MatHub = AMaterialHub::getInstance();
    if (iMat < 0 || iMat >= MatHub.countMaterials() )
    {
        guitools::message("Material does not exist!", this);
        return;
    }
    if (MatHub[iMat]->PrimarySpectrum.empty())
    {
        guitools::message(QString("Primary scintillation emission spectrum is not defined for the selected material"), this);
        return;
    }

    //qDebug() << "Converting data to standart wavelength: From To Nodes"<<WaveFrom<<WaveTo<<WaveNodes;
    const AWaveResSettings & WaveSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    std::vector<double> spec;
    WaveSet.toStandardBins(MatHub[iMat]->PrimarySpectrum, spec, AWaveResSettings::ExpandWithZero);
    std::vector<double> pde;
    WaveSet.toStandardBins(SensHub.model(iSensorModel)->PDE_spectral, pde, AWaveResSettings::ExpandWithZero);

    double weightedSum = 0;
    double weights = 0;
    for(size_t i = 0; i < spec.size(); i++)
    {
        //qDebug() << i << "sp:" << spec[i] << "pde:" << pde[i];
        weightedSum += spec[i] * pde[i];
        weights     += spec[i];
    }

    if (weights == 0)
    {
        guitools::message("Error: the overlap is zero!", this);
        return;
    }

    ui->ledEffectivePDE->setText(QString::number(weightedSum/weights, 'g', 4));
    on_ledEffectivePDE_editingFinished();
}


void ASensorWindow::on_cbGains_clicked(bool checked)
{
    SensHub.UseSensorGains = checked;

    if (checked)
    {
        if (ui->cbGains_ShowTable->isChecked())
            showTableWithGains();
    }
    else ui->cbGains_ShowTable->setChecked(false);

    updateHeader();
}

void ASensorWindow::on_pbGains_Clear_clicked()
{
    SensHub.SensorGains.resize(SensHub.countSensors());
    std::fill(SensHub.SensorGains.begin(), SensHub.SensorGains.end(), 1.0);

    if (ui->cbGains_ShowTable->isChecked()) showTableWithGains();
    updateHeader();
}

#include "arandomhub.h"
void ASensorWindow::on_pbGains_Randomize_clicked()
{
    double mean  = ui->ledGains_Mean->text().toDouble();
    double sigma = ui->ledGains_Sigma->text().toDouble();

    SensHub.SensorGains.resize(SensHub.countSensors());
    for (size_t i = 0; i < SensHub.SensorGains.size(); i++)
    {
        double val = ARandomHub::getInstance().gauss(mean, sigma);
        int rounded = val * 1000;
        SensHub.SensorGains[i] = 0.001 * rounded;
    }

    if (ui->cbGains_ShowTable->isChecked()) showTableWithGains();
    updateHeader();
}

void ASensorWindow::showTableWithGains()
{
    ui->twGains->clearContents();

    const size_t num = SensHub.SensorGains.size();
    ui->twGains->setRowCount(num);

    QStringList headerLabels;
    for (size_t iSens = 0; iSens < num; iSens++)
    {
        QLineEdit * te = new QLineEdit("", 0);
        te->setValidator(CellValidator);
        te->setAlignment(Qt::AlignHCenter);
        te->setFrame(false);
        QObject::connect(te, &QLineEdit::editingFinished, this, &ASensorWindow::onGainCellEditingFinished);
        ui->twGains->setCellWidget(iSens, 0, te);

        te->setText( QString::number(SensHub.SensorGains[iSens]) );

        ui->twGains->setRowHeight(iSens, RowHeight);
        headerLabels << QString::number(iSens);
    }

    ui->twGains->setVerticalHeaderLabels(headerLabels);
}

void ASensorWindow::on_cbGains_ShowTable_toggled(bool checked)
{
    ui->twGains->setVisible(checked);
    if (checked) showTableWithGains();

    ui->spGainLower->changeSize(10, 10, QSizePolicy::Minimum,
                                checked? QSizePolicy::Ignored : QSizePolicy::Expanding);
}

void ASensorWindow::onGainCellEditingFinished()
{
    int row = ui->twGains->currentRow();
    if (row >= 0 && row < SensHub.SensorGains.size())
    {
        QWidget * widget = ui->twGains->cellWidget(row, 0);
        QLineEdit * le = dynamic_cast<QLineEdit*>(widget);
        if (le) SensHub.SensorGains[row] = le->text().toDouble();
        else guitools::message("Cell widget not found!", this);
    }
    else guitools::message("Cell not found!", this);
}

#include <QFileInfo>
void ASensorWindow::on_pbGains_Load_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load gains from a text file", "*.*");
    if (fileName.isEmpty()) return;

    if (!QFileInfo::exists(fileName))
    {
        guitools::message("File does not exist: " + fileName);
        return;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        guitools::message("Cannot open file: "+fileName);
        return;
    }

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    std::vector<double> vec;

    while (!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.startsWith('#') || line.startsWith("//")) continue; // it is a comment

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.isEmpty()) continue;

        bool bOK;
        double first = fields.first().toDouble(&bOK);
        if (!bOK)
        {
            guitools::message("Bad format of the file: numeric values are expected");
            return;
        }

        if      (fields.size() == 1)
            vec.push_back(first);
        else if (fields.size() == 2)
        {
            double second = fields[1].toDouble(&bOK);
            if (!bOK)
            {
                guitools::message("Bad format of the file: numeric values are expected");
                return;
            }
            vec.push_back(second);
        }
    }
    file.close();

    SensHub.SensorGains = vec;

    updateHeader();
    if (ui->cbGains_ShowTable->isChecked()) showTableWithGains();
}

void ASensorWindow::on_pbGains_Save_clicked()
{
    QString fn = guitools::dialogSaveFile(this, "Save gains to text file", "*.*");
    if (fn.isEmpty()) return;

    QString err = ftools::saveArrayOfDoublesToFile(fn, SensHub.SensorGains);
    if (!err.isEmpty()) guitools::message(err, this);
}

void ASensorWindow::on_pbGains_Save_customContextMenuRequested(const QPoint &)
{
    QString fn = guitools::dialogSaveFile(this, "Save gains to text file (with sensor index)", "*.*");
    if (fn.isEmpty()) return;

    const size_t size = SensHub.SensorGains.size();
    std::vector<std::pair<double, double>> vec(size);
    for (size_t i = 0; i < size; i++)
        vec[i] = {i, SensHub.SensorGains[i]};

    QString err = ftools::saveArrayOfDoublePairsToFile(fn, vec);
    if (!err.isEmpty()) guitools::message(err, this);
}

#include "ajsontools.h"
void ASensorWindow::on_actionSave_sensor_triggered()
{
    int iModel = ui->cobModel->currentIndex();
    if (iModel < 0 || iModel >= SensHub.countModels()) return;
    ASensorModel * mod = SensHub.model(iModel);

    QString fn = guitools::dialogSaveFile(this, "Save this sensor model to file", "*.json");
    if (fn.isEmpty()) return;
    if (!fn.endsWith(".json")) fn += ".json";

    QJsonObject json;
    mod->writeToJson(json);
    bool ok = jstools::saveJsonToFile(json, fn);
    if (!ok) guitools::message("Cannot open file for writing!", this);
}

void ASensorWindow::on_actionLoad_sensor_triggered()
{
    QString fn = guitools::dialogLoadFile(this, "Append new sensor model from file", "*.json");
    if (fn.isEmpty()) return;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fn);
    if (!ok) guitools::message("Cannot open file for reading!", this);

    ASensorModel model;
    QString err = model.readFromJson(json);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    int index = SensHub.addModel(model);
    updateGui();

    ui->cobModel->setCurrentIndex(index);
    onModelIndexChanged();
}

void ASensorWindow::on_pbShowPDE_customContextMenuRequested(const QPoint &)
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->PDE_spectral.empty()) return;

    //mod->updateRuntimeProperties(std::vector<int>());
    AMaterialHub::getInstance().updateRuntimeProperties();
    ASensorHub::getInstance().updateRuntimeProperties();

    const APhotonSimSettings SimSet = APhotonSimHub::getConstInstance().Settings;
    std::vector<double> wave;
    SimSet.WaveSet.getWavelengthBins(wave);

    if (ui->cobPDEmodel->currentIndex() == 0)
    {
        TGraph * gr = AGraphBuilder::graph(wave, mod->PDEbinned);
        AGraphBuilder::configure(gr, QString("BinnedPDE_mod%0").arg(iModel), "Wavelength, nm", "PDE", 4, 20, 1, 4);
        gr->SetMinimum(0);
        emit requestDraw(gr, "APL", true, true);
    }
    else
    {
        for (size_t i = 0; i < mod->_InterfaceAwarePDE.size(); i++)
        {
            int iMat = mod->_InterfaceAwarePDE[i].first;

            TGraph * gr = new TGraph();
            for (size_t iWave = 0; iWave < wave.size(); iWave++)
            {
                double pde = mod->PDEbinned[iWave] * mod->_InterfaceAwarePDE[i].second.PdeBinnedFactor[iWave];
                gr->AddPoint(wave[iWave], pde);
            }
            AGraphBuilder::configure(gr, QString("BinnedPDE_%0").arg(AMaterialHub::getConstInstance().getMaterialName(iMat)), "Wavelength, nm", "PDE", 4, 20, 1, 4);
            gr->SetMinimum(0);
            emit requestDraw(gr, (i == 0 ? "APL" : "PLsame"), true, true);
        }
    }
}

void ASensorWindow::on_pbShowAngular_customContextMenuRequested(const QPoint &)
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->AngularFactors.empty()) return;

    //mod->updateRuntimeProperties(std::vector<int>());
    AMaterialHub::getInstance().updateRuntimeProperties();
    ASensorHub::getInstance().updateRuntimeProperties();

    std::vector<double> angles;
    size_t bins = mod->AngularBinned.size();
    for (size_t i = 0; i < mod->AngularBinned.size(); i++) angles.push_back(i * 90.0 / (bins - 1));

    if ( ui->cobPDEmodel->currentIndex() == 0)
    {
        TGraph * gr = AGraphBuilder::graph(angles, mod->AngularBinned);
        AGraphBuilder::configure(gr, QString("Binned angular sensitivity, model%0").arg(iModel), "Incidence angle, deg", "Sensitivity factor", 4, 20, 1, 4);
        gr->SetMinimum(0);
        emit requestDraw(gr, "APL", true, true);
    }
    else
    {
        for (size_t i = 0; i < mod->_InterfaceAwarePDE.size(); i++)
        {
            int iMat = mod->_InterfaceAwarePDE[i].first;
            TGraph * gr = AGraphBuilder::graph(angles, mod->_InterfaceAwarePDE[i].second.AngularBinned);
            AGraphBuilder::configure(gr, QString("BinSens_mat%1_mod%0").arg(iModel).arg(iMat), "Refracted angle, deg", "Sensitivity factor", 4, 20, 1, 4);
            gr->SetMinimum(0);
            emit requestDraw(gr, (i == 0 ? "APL" : "PLsame"), true, true);

            gr = AGraphBuilder::graph(mod->_InterfaceAwarePDE[i].second.AngularRefracted);
            AGraphBuilder::configure(gr, QString("Sens_mat%1_mod%0").arg(iModel).arg(iMat), "Refracted angle, deg", "Sensitivity factor", 2, 31, 1, 2);
            gr->SetMinimum(0);
            emit requestDraw(gr, "Psame", true, true);
        }
    }
}

void ASensorWindow::on_pbHelpPDEmodeling_clicked()
{
    QString txt = "Photon detection check is triggered when photon _enters_ a sensor.\n"
                  "Thus the photon first has to pass the interface:\n"
                  "taking into account the defined custom interface rules, and,\n"
                  "if the materials of the sensor and of the surrounding medium have different refractive indexes, "
                  "pass the reflection test based on Fresnel equations.\n"
                  "\n"
        "The PDE is computer as a multiplication of three factors (all except the 'Effective PDE' are optional):\n"
        "1) Base PDE, which is the Effective PDE for photons with waveindex of -1, "
        "or computed from the spectral PDE data (in case the waveindex is not -1 and the spectral data are provided by the user).\n"
        "2) Angular factor (if provided by the user), typically of unity at the normal incidence, and describing the sensor n"
        "response as a function of the _refracted_ angle.\n"
        "3) Area factor, descriping the spatial response (over the sensor active area, also if provided by the user)\n"
        "\n"
        "The user can also select either the 'Simplistic' or 'Account for the interface' PDE model.\n"

        "\n'Simplistic' model does not apply any corrections and the data are used exactly as provided by the user.\n"
        "One of the cases where this is an adequate model is when the sensor material is set to be the same "
        "as that of the surrounding medium. In this case the angles of incidence and refraction are the same, and if the material "
        "is air, the measured angular response can be directly used.\n"
        "Be careful in the case when the reflected light from the sensor is important, as in this case there will be none!\n"
        "\n'Account for the interface model' is intended for the case when the medium in front of the sensor is not the same as that of the sensor, "
        "and, expecially, when the sensor is not in air.\n"
        "The model assumes that all provided PDE-related data were measured in air, and automatically correct for the fraction of light which was reflected "
        "during the measurements from the sensor interface.\n"
        "The angular dependence also is converted from insidence to refracted angle: righ-click on the 'Show' button will plot a graph of the computed angular factor vs refracted angle.\n"
        "Due to air->material transition, the refracted angle data lack large angle values. Therefore, the largest-angle non-zero value is "
        "assigned for all angles with missing data.\n\n"
        "See test config: General/Optical sim/SensorPdeModelTester.json";
    guitools::message(txt, this);
}

void ASensorWindow::on_ledAngularWave_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->Angular_Wavelength = ui->ledAngularWave->text().toDouble();
}

void ASensorWindow::on_cobPDEmodel_activated(int index)
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->PDE_model = index;
    updateAngularButtons();
}
void ASensorWindow::on_cobPDEmodel_currentIndexChanged(int index)
{
    ui->labInAir1->setVisible(index == 1);
    ui->labInAir2->setVisible(index == 1);
    ui->labInAir3->setVisible(index == 1);
}

#include "TPaveText.h"
void ASensorWindow::on_pbCheckTimeFraction_clicked()
{
    int iSensorModel = ui->cobModel->currentIndex();
    if (iSensorModel < 0 || iSensorModel >= SensHub.countModels())
    {
        guitools::message("Sensor model does not exist!", this);
        return;
    }

    double inteTime = ui->lepIntegrationTime->text().toDouble() * 1e9; // in ns
    if (inteTime == 0)
    {
        guitools::message("Use non-zero integration time", this);
        return;
    }

    QDialog dia(this);
    dia.setWindowTitle("Photon fraction estimator");
    QVBoxLayout * mainLay = new QVBoxLayout(&dia);
    QHBoxLayout * lay = new QHBoxLayout();
    lay->addWidget(new QLabel("Primary scintillation from"));
    QComboBox * cobMats = new QComboBox();
    cobMats->addItems(AMaterialHub::getInstance().getListOfMaterialNames());
    lay->addWidget(cobMats);
    mainLay->addLayout(lay);
    QPushButton * pbEst = new QPushButton("Compute fraction of photons within integration time");
    connect(pbEst, &QPushButton::clicked, &dia, &QDialog::accept);
    mainLay->addWidget(pbEst);

    int res = dia.exec();
    if (res == QDialog::Rejected) return;

    int iMat = cobMats->currentIndex();
    AMaterialHub & MatHub = AMaterialHub::getInstance();
    if (iMat < 0 || iMat >= MatHub.countMaterials() )
    {
        guitools::message("Material does not exist!", this);
        return;
    }
    if (MatHub[iMat]->PrimarySpectrum.empty())
    {
        guitools::message(QString("Primary scintillation emission spectrum is not defined for the selected material"), this);
        return;
    }

    MatHub[iMat]->updateRuntimeOpticalProperties();

    TH1D * h = new TH1D("", "", 100,0,0);
    const size_t numPhot = 100000;
    size_t inside = 0;
    double delta = 1.0 / numPhot;
    for (size_t iPh = 0; iPh < numPhot; iPh++)
    {
        double time = MatHub[iMat]->generatePrimScintTime(ARandomHub::getInstance());
        h->Fill(time, delta);
        if (time < inteTime) inside++;
    }
    double fraction = 1.0 * inside / numPhot;

    h->GetXaxis()->SetTitle("Time, ns");
    emit requestDraw(h, "hist", true, true);

    TPaveText * la = new TPaveText(0.3, 0.5, 0.7, 0.6, "NDC");
    la->SetFillColor(0);
    la->SetBorderSize(1);
    la->SetLineColor(1);
    int alignLeftCenterRight = 0;
    la->SetTextAlign( (alignLeftCenterRight + 1) * 10 + 2);
    la->AddText("Fraction of photons emitted");

    QString frtxt = QString::number(fraction, 'g', 4);
    TString txt = "  within the integration time: " + TString(frtxt.toLatin1().data());
    qDebug() << txt;
    la->AddText(txt);
    emit requestDraw(la, "same", true, true);
}

