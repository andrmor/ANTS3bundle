#include "asensorwindow.h"
#include "ui_asensorwindow.h"
#include "asensorhub.h"
#include "asensormodel.h"
#include "guitools.h"
#include "afiletools.h"
#include "agraphbuilder.h"
#include "aphotonsimhub.h"

#include <QDoubleValidator>

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

    ASensorModel * mod = SensHub.model(iModel); // can be nullptr

    if (mod)
    {
        ui->cobSensorType->setCurrentIndex(mod->SiPM ? 1 : 0);
        ui->sbPixelsX->setValue(mod->PixelsX);
        ui->sbPixelsY->setValue(mod->PixelsY);
        ui->lepPixelSizeX->setText(QString::number(mod->PixelSizeX));
        ui->lepPixelSizeY->setText(QString::number(mod->PixelSizeY));
        ui->lepPixelSpacingX->setText(QString::number(mod->PixelSpacingX));
        ui->lepPixelSpacingY->setText(QString::number(mod->PixelSpacingY));

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
        ui->lepElGainFactor->setText(QString::number(mod->ElectronicGainFactor));
        ui->lepAverageSignalPerPhE->setText(QString::number(mod->AverageSignalPerPhEl));
        ui->lepNormalSigma->setText(QString::number(mod->NormalSigma));
        ui->lepGammaShape->setText(QString::number(mod->GammaShape));
    }

    updatePdeButtons();
    updateAngularButtons();
    updateAreaButtons();
    updatePhElToSigButtons();
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
    updateAngularButtons();
    updateAreaButtons();

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

void ASensorWindow::updateAngularButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->AngularFactors.empty();

    ui->pbShowAngular->setEnabled(enabled);
    ui->pbRemoveAngular->setEnabled(enabled);
    ui->pbShowBinnedAngular->setEnabled(enabled);
}

void ASensorWindow::updateAreaButtons()
{
    bool enabled = false;

    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (mod) enabled = !mod->AreaFactors.empty();

    ui->pbShowArea->setEnabled(enabled);
    ui->pbRemoveArea->setEnabled(enabled);
    ui->lepAreaStepX->setEnabled(enabled);
    ui->lepAreaStepY->setEnabled(enabled);
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
void ASensorWindow::on_pbShowBinnedAngular_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;
    if (mod->AngularFactors.empty()) return;

    mod->updateRuntimeProperties();

    std::vector<double> angles;
    for (int i = 0; i < 91; i++) angles.push_back(i);

    TGraph * gr = AGraphBuilder::graph(angles, mod->AngularBinned);
    AGraphBuilder::configure(gr, QString("Binned angular sensitivity, model%0").arg(iModel), "Incidence angle, deg", "Sensitivity factor", 4, 20, 1, 4);
    emit requestDraw(gr, "APL", true, true);
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

     mod->updateRuntimeProperties(); // to enable mod->getPixelHit()

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

void ASensorWindow::on_lepElGainFactor_editingFinished()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->ElectronicGainFactor = ui->lepElGainFactor->text().toDouble();
}

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

    QString fname = guitools::dialogLoadFile(this, "Load factors for ph.e- to signal convertion.\nShould start from 0 ph.e-!", "");
    if (fname.isEmpty()) return;

    std::vector<std::pair<double,double>> data;
    QString err = ftools::loadPairs(fname, data, true);
    if (err.isEmpty())
    {
        ASensorModel dummy;
        dummy.CustomSignalPerPhEl = data;
        err = dummy.checkPhElToSignals();
        if (err.isEmpty())
        {
            mod->CustomSignalPerPhEl = data;
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
    if (mod->CustomSignalPerPhEl.empty()) return;

    TGraph * gr = AGraphBuilder::graph(mod->CustomSignalPerPhEl);
    AGraphBuilder::configure(gr, QString("Custom ph.e- to signal factors for model%0").arg(iModel), "Number of ph.e-", "");
    emit requestDraw(gr, "APL", true, true);
}

void ASensorWindow::on_pbRemoveCustomPhElSig_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    mod->CustomSignalPerPhEl.clear();
    updatePhElToSigButtons();
}

void ASensorWindow::updatePhElToSigButtons()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

    ui->pbShowCustomPhElSig->setDisabled(mod->CustomSignalPerPhEl.empty());
    ui->pbRemoveCustomPhElSig->setDisabled(mod->CustomSignalPerPhEl.empty());
}

#include <QDialog>
#include <QSpinBox>
#include <QDoubleValidator>
static int    lastSelectedTimes = 10000;
static double lastSelectedPhEl = 1.0;
void ASensorWindow::on_pbTestPhElSignal_clicked()
{
    int iModel = ui->cobModel->currentIndex();
    ASensorModel * mod = SensHub.model(iModel);
    if (!mod) return;

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
        lay->addWidget(new QLabel("photoelectrons"));
        QSpinBox * sbTimes = new QSpinBox(); sbTimes->setMaximum(1e9); sbTimes->setMinimum(1); sbTimes->setValue(lastSelectedTimes);
        lay->addWidget(sbTimes);
        lay->addWidget(new QLabel("times"));
    vlay->addLayout(lay);

    auto click = [this, mod, dialog, lePE, sbTimes]()
    {
        lastSelectedPhEl = lePE->text().toDouble();
        lastSelectedTimes = sbTimes->value();
        auto hist1D = new TH1D("tmpHistElTgen", "test", 100, 0, 0);
        for (int i = 0; i < lastSelectedTimes; i++)
            hist1D->Fill(mod->convertHitsToSignal(lastSelectedPhEl));
        hist1D->SetXTitle("PM signal");
        emit requestDraw(hist1D, "hist", true, true);

        dialog->accept();
    };
    connect(pb, &QPushButton::clicked, this, click);

    dialog->setMinimumWidth(width());
    dialog->show();
    dialog->move(dialog->x(), QCursor::pos().y() - 15);
    dialog->exec();
}
