#include "materialinspectorwindow.h"
#include "ui_materialinspectorwindow.h"
#include "mainwindow.h"
#include "amaterialparticlecolection.h"
//#include "graphwindowclass.h"
//#include "windownavigatorclass.h"
#include "a3geometry.h"
#include "a3global.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "guitools.h"
#include "acommonfunctions.h"
//#include "ainternetbrowser.h"
//#include "amatparticleconfigurator.h"
#include "achemicalelement.h"
#include "aelementandisotopedelegates.h"
#include "geometrywindowclass.h"

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QDesktopServices>
#include <QLabel>
#include <QIcon>
#include <QGroupBox>
#include <QStringListModel>
#include <QVariantList>
#include <QDialog>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QThread>
#include <QPainter>

//Root
#include "TGraph.h"
#include "TH1.h"
#include "TAxis.h"
#include "TGeoManager.h"
#include "TAttLine.h"
#include "TAttMarker.h"

MaterialInspectorWindow::MaterialInspectorWindow(QWidget * parent) :
    QMainWindow(parent), //AGuiWindow("mat", parent),
    Geometry(A3Geometry::getInstance()),
    MpCollection(AMaterialParticleCollection::getInstance()),
    GlobSet(A3Global::getInstance()),
    ui(new Ui::MaterialInspectorWindow)
{
    ui->setupUi(this);

    this->move(15,15);

    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    //windowFlags |= Qt::Tool;
    this->setWindowFlags( windowFlags );

    //SetWasModified(false);
    ui->pbWasModified->setVisible(false);
    ui->pbUpdateInteractionIndication->setVisible(false);
    ui->labContextMenuHelp->setVisible(false);
    ui->pbUpdateTmpMaterial->setVisible(false);
    ui->cobStoppingPowerUnits->setCurrentIndex(1);
    ui->tabwNeutron->verticalHeader()->setVisible(false);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    QPixmap pm(QSize(16,16));
    pm.fill(Qt::transparent);
    QPainter b(&pm);
    b.setBrush(QBrush(Qt::yellow));
    b.drawEllipse(0, 2, 10, 10);
    ui->labAssumeZeroForEmpty->setPixmap(pm);
}

MaterialInspectorWindow::~MaterialInspectorWindow()
{
    delete ui;
}

void MaterialInspectorWindow::InitWindow()
{
    UpdateActiveMaterials();
    showMaterial(0);
}

void MaterialInspectorWindow::setWasModified(bool flag)
{
    if (flagDisreguardChange) return;

    bMaterialWasModified = flag;

    QString s = ( flag ? "<html><span style=\"color:#ff0000;\">Material was modified: Click \"Update\" or \"Add\" to save changes</span></html>"
                       : " " );
    ui->labMatWasModified->setText(s);

    updateActionButtons();
}

void MaterialInspectorWindow::UpdateActiveMaterials()
{   
    if (bLockTmpMaterial) return;

    int current = ui->cobActiveMaterials->currentIndex();

    ui->cobActiveMaterials->clear();
    ui->cobActiveMaterials->addItems(MpCollection.getListOfMaterialNames());

    int matCount = ui->cobActiveMaterials->count();
    if (current < -1 || current >= matCount)
        current = matCount - 1;

    showMaterial(current);
    setWasModified(false);
}

void MaterialInspectorWindow::on_pbAddNewMaterial_clicked()
{
    on_pbAddToActive_clicked(); //processes both update and add
}

void MaterialInspectorWindow::on_pbAddToActive_clicked()
{
    if ( !parseDecayOrRaiseTime(true) )  return;  //error messaging inside
    if ( !parseDecayOrRaiseTime(false) ) return;  //error messaging inside

    MpCollection.tmpMaterial.updateRuntimeProperties();

    const QString error = MpCollection.CheckTmpMaterial();
    if (!error.isEmpty())
    {
        guitools::message(error, this);
        return;
    }

    QString name = MpCollection.tmpMaterial.name;
    int index = MpCollection.FindMaterial(name);
    if (index == -1)
        index = MpCollection.countMaterials(); //then it will be appended, index = current size
    else
    {
        switch( QMessageBox::information( this, "", "Update properties for material "+name+"?", "Overwrite", "Cancel", 0, 1 ) )
        {
           case 0:  break;  //overwrite
           default: return; //cancel
        }
    }

    MpCollection.CopyTmpToMaterialCollection(); //if absent, new material is created!

    //MW->ReconstructDetector(true);
    //MW->UpdateMaterialListEdit(); //need?

    showMaterial(index);
}

void MaterialInspectorWindow::showMaterial(int index)
{
    if (index == -1)
    {
        ui->cobActiveMaterials->setCurrentIndex(-1);
        LastShownMaterial = index;
        return;
    }

    if (index < 0 || index > MpCollection.countMaterials()) return;

    MpCollection.CopyMaterialToTmp(index);
    ui->cobActiveMaterials->setCurrentIndex(index);
    LastShownMaterial = index;
    UpdateGui();

    ui->pbRename->setText("Rename " + ui->cobActiveMaterials->currentText());
    setWasModified(false);
}

void MaterialInspectorWindow::on_cobActiveMaterials_activated(int index)
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Explore another material", "All unsaved changes will be lost. Continue?", QMessageBox::Yes | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel)
        {
            ui->cobActiveMaterials->setCurrentIndex(LastShownMaterial);
            return;
        }
    }
    showMaterial(index);
}

void MaterialInspectorWindow::updateWaveButtons()
{   
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    bool bPrimSpec = (tmpMaterial.PrimarySpectrum_lambda.size() > 0);
    ui->pbShowPrimSpectrum->setEnabled(bPrimSpec);
    ui->pbDeletePrimSpectrum->setEnabled(bPrimSpec);

    bool bSecSpec = (tmpMaterial.SecondarySpectrum_lambda.size() > 0);
    ui->pbShowSecSpectrum->setEnabled(bSecSpec);
    ui->pbDeleteSecSpectrum->setEnabled(bSecSpec);

    bool bN = (tmpMaterial.nWave_lambda.size() > 0);
    ui->pbShowNlambda->setEnabled(bN);
    ui->pbDeleteNlambda->setEnabled(bN);

    bool bA = (tmpMaterial.absWave_lambda.size() > 0);
    ui->pbShowABSlambda->setEnabled(bA);
    ui->pbDeleteABSlambda->setEnabled(bA);

    ui->pbShowReemProbLambda->setEnabled( !tmpMaterial.reemisProbWave_lambda.isEmpty() );
    ui->pbDeleteReemisProbLambda->setEnabled( !tmpMaterial.reemisProbWave_lambda.isEmpty() );
}

void MaterialInspectorWindow::updateG4RelatedGui()
{
    bool bDisable = ui->cbG4Material->isChecked();
    QVector<QWidget*> widgs = {ui->ledDensity, ui->pbMaterialInfo, ui->ledT, ui->leChemicalComposition,
                            ui->pbModifyChemicalComposition, ui->leCompositionByWeight, ui->pbModifyByWeight,
                            ui->trwChemicalComposition, ui->cbShowIsotopes};
    for (QWidget * w : widgs) w->setDisabled(bDisable);
}

void MaterialInspectorWindow::UpdateGui()
{
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    ui->leName->setText(tmpMaterial.name);  

    ui->ledDensity->setText( QString::number(tmpMaterial.density) );
    ui->ledT->setText( QString::number(tmpMaterial.temperature) );

    ui->leChemicalComposition->setText( tmpMaterial.ChemicalComposition.getCompositionString() );
    ui->leCompositionByWeight->setText( tmpMaterial.ChemicalComposition.getCompositionByWeightString() );
    ShowTreeWithChemicalComposition();

    ui->cbG4Material->setChecked(tmpMaterial.bG4UseNistMaterial);
    ui->leG4Material->setText(tmpMaterial.G4NistMaterial);
    updateG4RelatedGui();

    ui->ledN->setText( QString::number(tmpMaterial.n) );
    ui->ledAbs->setText( QString::number(tmpMaterial.abs) );
    ui->ledReemissionProbability->setText( QString::number(tmpMaterial.reemissionProb) );

    QString s = ( tmpMaterial.rayleighMFP > 0 ? QString::number(tmpMaterial.rayleighMFP)
                                                : "" );
    ui->ledRayleigh->setText(s);
    ui->ledRayleighWave->setText( QString::number(tmpMaterial.rayleighWave) );

    //decay time
    if ( tmpMaterial.PriScint_Decay.isEmpty() )
        s = "0";
    else if (tmpMaterial.PriScint_Decay.size() == 1)
        s = QString::number( tmpMaterial.PriScint_Decay.first().value );
    else
    {
        s.clear();
        for (const APair_ValueAndWeight & pair : tmpMaterial.PriScint_Decay)
        {
            s += QString::number(pair.value);
            s += ":";
            s += QString::number(pair.statWeight);
            s += " & ";
        }
        s.chop(3);
    }
    ui->lePriT->setText(s);

    //rise time
    if ( tmpMaterial.PriScint_Raise.isEmpty() )
        s = "0";
    else if (tmpMaterial.PriScint_Raise.size() == 1)
        s = QString::number(tmpMaterial.PriScint_Raise.first().value);
    else
    {
        s.clear();
        for (const APair_ValueAndWeight& pair : tmpMaterial.PriScint_Raise)
        {
            s += QString::number(pair.value);
            s += ":";
            s += QString::number(pair.statWeight);
            s += " & ";
        }
        s.chop(3);
    }
    ui->lePriT_raise->setText(s);

    ui->ledW->setText( QString::number(tmpMaterial.W*1000.0) );  //keV->eV
    ui->ledSecYield->setText( QString::number(tmpMaterial.SecYield) );
    ui->ledSecT->setText( QString::number(tmpMaterial.SecScintDecayTime) );
    ui->ledEDriftVelocity->setText( QString::number(tmpMaterial.e_driftVelocity) );
    ui->ledEDiffL->setText( QString::number(tmpMaterial.e_diffusion_L) );
    ui->ledEDiffT->setText( QString::number(tmpMaterial.e_diffusion_T) );

    int lastSelected_cobYieldForParticle = ui->cobYieldForParticle->currentIndex();
    ui->cobYieldForParticle->clear();

    // !!!***
//    ui->cobYieldForParticle->addItems(QStringList() << PartList << "Undefined");
    int numPart = 0; //MpCollection.countParticles();
    if (lastSelected_cobYieldForParticle < 0) lastSelected_cobYieldForParticle = 0;
    if (lastSelected_cobYieldForParticle > numPart) //can be "Undefined"
        lastSelected_cobYieldForParticle = 0;
    ui->cobYieldForParticle->setCurrentIndex(lastSelected_cobYieldForParticle);
    double val = tmpMaterial.getPhotonYield(lastSelected_cobYieldForParticle);
    ui->ledPrimaryYield->setText(QString::number(val));
    val        = tmpMaterial.getIntrinsicEnergyResolution(lastSelected_cobYieldForParticle);
    ui->ledIntEnergyRes->setText(QString::number(val));

    ui->pteComments->clear();
    ui->pteComments->appendPlainText(tmpMaterial.Comments);

    QString sTags;
    for (const QString & s : tmpMaterial.Tags)
        sTags.append(s.simplified() + ", ");
    if (sTags.size() > 1) sTags.chop(2);
    ui->leTags->setText(sTags);

    updateWaveButtons();
    updateWarningIcons();
}

void MaterialInspectorWindow::updateWarningIcons()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    if (tmpMaterial.ChemicalComposition.countElements() == 0)
    {
        QPixmap pm(QSize(16,16));
        pm.fill(Qt::transparent);
        QPainter b(&pm);
        b.setBrush(QBrush(Qt::yellow));
        b.drawEllipse(0, 2, 10, 10);
        ui->twProperties->setTabIcon(0, QIcon(pm));
    }
    else ui->twProperties->setTabIcon(0, QIcon());
}

void MaterialInspectorWindow::updateEnableStatus()
{
    bool TrackingAllowed = ui->cbTrackingAllowed->isChecked();
    bool MaterialIsTransparent = ui->cbTransparentMaterial->isChecked();

    ui->cbTransparentMaterial->setEnabled(TrackingAllowed);
    ui->fEnDepProps->setEnabled(TrackingAllowed && !MaterialIsTransparent);

    ui->swMainMatParticle->setEnabled(!MaterialIsTransparent); // need?

    QFont font = ui->cbTransparentMaterial->font();
    font.setBold(MaterialIsTransparent);
    ui->cbTransparentMaterial->setFont(font);
}

void MaterialInspectorWindow::on_pbUpdateTmpMaterial_clicked()
{  
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    tmpMaterial.name = ui->leName->text();
    tmpMaterial.density = ui->ledDensity->text().toDouble();    
    tmpMaterial.temperature = ui->ledT->text().toDouble();
    tmpMaterial.n = ui->ledN->text().toDouble();
    tmpMaterial.abs = ui->ledAbs->text().toDouble();
    tmpMaterial.reemissionProb = ui->ledReemissionProbability->text().toDouble();

    double prYield = ui->ledPrimaryYield->text().toDouble();
    int iP = ui->cobYieldForParticle->currentIndex();
    if (iP > -1 && iP < tmpMaterial.MatParticle.size()) tmpMaterial.MatParticle[iP].PhYield = prYield;
    else tmpMaterial.PhotonYieldDefault = prYield;

    tmpMaterial.W = ui->ledW->text().toDouble()*0.001; //eV -> keV
    tmpMaterial.SecYield = ui->ledSecYield->text().toDouble();
    tmpMaterial.SecScintDecayTime = ui->ledSecT->text().toDouble();   
    tmpMaterial.e_driftVelocity = ui->ledEDriftVelocity->text().toDouble();

    tmpMaterial.e_diffusion_L = ui->ledEDiffL->text().toDouble();
    tmpMaterial.e_diffusion_T = ui->ledEDiffT->text().toDouble();

    tmpMaterial.Comments = ui->pteComments->document()->toPlainText();

    QStringList slTags = ui->leTags->text().split(',', Qt::SkipEmptyParts);
    tmpMaterial.Tags.clear();
    for (const QString & s : slTags)
        tmpMaterial.Tags << s.simplified();

    tmpMaterial.bG4UseNistMaterial = ui->cbG4Material->isChecked();
    tmpMaterial.G4NistMaterial = ui->leG4Material->text();
}

void MaterialInspectorWindow::SetMaterial(int index)
{
    showMaterial(index);
}

void MaterialInspectorWindow::on_ledIntEnergyRes_editingFinished()
{
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    double newVal = ui->ledIntEnergyRes->text().toDouble();
    if (newVal < 0)
    {
        ui->ledIntEnergyRes->setText("0");
        guitools::message("Intrinsic energy resolution cannot be negative", this);
        return;
    }

    int iP = ui->cobYieldForParticle->currentIndex();
    if (iP > -1 && iP < tmpMaterial.MatParticle.size())
        tmpMaterial.MatParticle[iP].IntrEnergyRes = newVal;
    else
        tmpMaterial.IntrEnResDefault = newVal;

    setWasModified(true);
}

void MaterialInspectorWindow::on_pbLoadPrimSpectrum_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load primary scintillation spectrum", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    QString err = ftools::loadDoubleVectorsFromFile(fileName, &tmpMaterial.PrimarySpectrum_lambda, &tmpMaterial.PrimarySpectrum);  //cleans previous data
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.PrimarySpectrum_lambda.isEmpty();
    ui->pbShowPrimSpectrum->setEnabled(bHaveData);
    ui->pbDeletePrimSpectrum->setEnabled(bHaveData);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowPrimSpectrum_clicked()
{
/*
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    TGraph * g = MW->GraphWindow->ConstructTGraph(tmpMaterial.PrimarySpectrum_lambda, tmpMaterial.PrimarySpectrum);
    MW->GraphWindow->configureGraph(g, "Emission spectrum",
                                    "Wavelength, nm", "Emission probability, a.u.",
                                    2, 20, 1,
                                    2, 1,  1);
    MW->GraphWindow->Draw(g, "APL");
*/
}

void MaterialInspectorWindow::on_pbDeletePrimSpectrum_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.PrimarySpectrum_lambda.clear();
    tmpMaterial.PrimarySpectrum.clear();

    ui->pbShowPrimSpectrum->setEnabled(false);
    ui->pbDeletePrimSpectrum->setEnabled(false);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbLoadSecSpectrum_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load secondary scintillation spectrum", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    QString err = ftools::loadDoubleVectorsFromFile(fileName, &tmpMaterial.SecondarySpectrum_lambda, &tmpMaterial.SecondarySpectrum);  //cleans previous data
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.SecondarySpectrum_lambda.isEmpty();
    ui->pbShowSecSpectrum->setEnabled(bHaveData);
    ui->pbDeleteSecSpectrum->setEnabled(bHaveData);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowSecSpectrum_clicked()
{
/*
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    TGraph * g = MW->GraphWindow->ConstructTGraph(tmpMaterial.SecondarySpectrum_lambda, tmpMaterial.SecondarySpectrum);
    MW->GraphWindow->configureGraph(g, "Emission spectrum",
                                    "Wavelength, nm", "Emission probability, a.u.",
                                    2, 20, 1,
                                    2, 1,  1);
    MW->GraphWindow->Draw(g, "APL");
*/
}

void MaterialInspectorWindow::on_pbDeleteSecSpectrum_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.SecondarySpectrum_lambda.clear();
    tmpMaterial.SecondarySpectrum.clear();

    ui->pbShowSecSpectrum->setEnabled(false);
    ui->pbDeleteSecSpectrum->setEnabled(false);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbLoadNlambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load refractive index data", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    QString err = ftools::loadDoubleVectorsFromFile(fileName, &tmpMaterial.nWave_lambda, &tmpMaterial.nWave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.nWave_lambda.isEmpty();
    ui->pbShowNlambda->setEnabled(bHaveData);
    ui->pbDeleteNlambda->setEnabled(bHaveData);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowNlambda_clicked()
{
/*
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    TGraph * g = MW->GraphWindow->ConstructTGraph(tmpMaterial.nWave_lambda, tmpMaterial.nWave);
    MW->GraphWindow->configureGraph(g, "Refractive index",
                                    "Wavelength, nm", "Refractive index",
                                    2, 20, 1,
                                    2, 1,  1);
    MW->GraphWindow->Draw(g, "APL");
*/
}

void MaterialInspectorWindow::on_pbDeleteNlambda_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.nWave_lambda.clear();
    tmpMaterial.nWave.clear();

    ui->pbShowNlambda->setEnabled(false);
    ui->pbDeleteNlambda->setEnabled(false);

    setWasModified(true);
}

void MaterialInspectorWindow::on_pbLoadABSlambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load exponential bulk absorption data", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    QString err = ftools::loadDoubleVectorsFromFile(fileName, &tmpMaterial.absWave_lambda, &tmpMaterial.absWave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.absWave_lambda.isEmpty();
    ui->pbShowABSlambda->setEnabled(bHaveData);
    ui->pbDeleteABSlambda->setEnabled(bHaveData);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowABSlambda_clicked()
{
/*
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    TGraph * g = MW->GraphWindow->ConstructTGraph(tmpMaterial.absWave_lambda, tmpMaterial.absWave);
    MW->GraphWindow->configureGraph(g, "Attenuation coefficient",
                                    "Wavelength, nm", "Attenuation coefficient, mm^{-1}",
                                    2, 20, 1,
                                    2, 1,  1);
    MW->GraphWindow->Draw(g, "APL");
*/
}

void MaterialInspectorWindow::on_pbDeleteABSlambda_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.absWave_lambda.clear();
    tmpMaterial.absWave.clear();

    ui->pbShowABSlambda->setEnabled(false);
    ui->pbDeleteABSlambda->setEnabled(false);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowReemProbLambda_clicked()
{
/*
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    TGraph * g = MW->GraphWindow->ConstructTGraph(tmpMaterial.reemisProbWave_lambda, tmpMaterial.reemisProbWave);
    MW->GraphWindow->configureGraph(g, "Reemission probability",
                                    "Wavelength, nm", "Reemission probability",
                                    2, 20, 1,
                                    2, 1,  1);
    MW->GraphWindow->Draw(g, "APL");
*/
}

void MaterialInspectorWindow::on_pbLoadReemisProbLambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load reemission probability vs wavelength", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    QString err = ftools::loadDoubleVectorsFromFile(fileName, &tmpMaterial.reemisProbWave_lambda, &tmpMaterial.reemisProbWave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.reemisProbWave_lambda.isEmpty();
    ui->pbShowReemProbLambda->setEnabled(bHaveData);
    ui->pbDeleteReemisProbLambda->setEnabled(bHaveData);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbDeleteReemisProbLambda_clicked()
{
    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.reemisProbWave_lambda.clear();
    tmpMaterial.reemisProbWave.clear();

    ui->pbShowReemProbLambda->setEnabled(false);
    ui->pbDeleteReemisProbLambda->setEnabled(false);
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbWasModified_clicked()
{
    setWasModified(true);
}

void MaterialInspectorWindow::on_leName_editingFinished()
{
  QString name = ui->leName->text();
  name.replace("+","_");
  name.replace(".","_");
  ui->leName->setText(name);

  if ( (name.at(0) >= QChar('a') && name.at(0) <= QChar('z')) || (name.at(0) >= QChar('A') && name.at(0) <= QChar('Z'))) ;//ok
  else
    {
      name.remove(0,1);
      name.insert(0, "A");
      ui->leName->setText(name);
      guitools::message("Name should start with an alphabetic character!", this);
      return;
    }
  on_pbUpdateTmpMaterial_clicked();
}

void MaterialInspectorWindow::on_leName_textChanged(const QString& /*name*/)
{
    //on text change - on chage this is a signal that it will be another material. These properties are recalculated anyway on
    //accepting changes/new material
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.absWaveBinned.clear();
    tmpMaterial.reemissionProbBinned.clear();
    tmpMaterial.nWaveBinned.clear();
    tmpMaterial.GeoMat = nullptr;  //do not delete! the original material has to have them
    tmpMaterial.GeoMed = nullptr;
    tmpMaterial.PrimarySpectrumHist = nullptr;
    tmpMaterial.SecondarySpectrumHist = nullptr;

    updateActionButtons();
}

void MaterialInspectorWindow::updateActionButtons()
{
    const QString name = ui->leName->text();
    int iMat = MpCollection.FindMaterial(name);
    if (iMat == -1)
    {
        // Material with this name does not exist
        ui->pbAddToActive->setEnabled(false);  //update button
        ui->pbAddNewMaterial->setEnabled(true);
        ui->pbRename->setEnabled(!bMaterialWasModified);
    }
    else
    {
        // Material with this name exists
        ui->pbAddToActive->setEnabled( ui->cobActiveMaterials->currentText() == name ); //update button
        ui->pbAddNewMaterial->setEnabled(false);
        ui->pbRename->setEnabled(false);
    }
}

void MaterialInspectorWindow::on_ledRayleighWave_editingFinished()
{
    double wave = ui->ledRayleighWave->text().toDouble();
    if (wave <= 0)
    {
        guitools::message("Wavelength should be a positive number!", this);
        wave = 500.0;
        ui->ledRayleighWave->setText("500");
    }

    MpCollection.tmpMaterial.rayleighWave = wave;
}

void MaterialInspectorWindow::on_ledRayleigh_textChanged(const QString &arg1)
{
    if (arg1 == "") ui->ledRayleighWave->setEnabled(false);
    else ui->ledRayleighWave->setEnabled(true);
}

void MaterialInspectorWindow::on_ledRayleigh_editingFinished()
{
    double ray;
    if (ui->ledRayleigh->text() == "") ray = 0;
    else ray = ui->ledRayleigh->text().toDouble();
    MpCollection.tmpMaterial.rayleighMFP = ray;
}

void MaterialInspectorWindow::on_pbRemoveRayleigh_clicked()
{
    ui->ledRayleigh->setText("");
    MpCollection.tmpMaterial.rayleighMFP = 0;
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbShowUsage_clicked()
{
/*
  AMaterial& tmpMaterial = MpCollection.tmpMaterial;
  QString name = tmpMaterial.name;
  int index = ui->cobActiveMaterials->currentIndex();

  bool flagFound = false;
  TObjArray* list = Geometry.GeoManager->GetListOfVolumes();
  int size = list->GetEntries();
  for (int i=0; i<size; i++)
    {
      TGeoVolume* vol = (TGeoVolume*)list->At(i);
      if (!vol) break;
      if (index == vol->GetMaterial()->GetIndex())
        {
          flagFound = true;
          break;
        }
    }

  Geometry.GeoManager->ClearTracks();
  MW->GeometryWindow->ClearGeoMarkers();
  if (flagFound)
    {
      Geometry.colorVolumes(2, index);
      MW->GeometryWindow->ShowGeometry(true, true, false);
    }
  else
    {
      MW->GeometryWindow->ShowGeometry(false);
      guitools::message("Current detector configuration does not have objects referring to material "+name, this);
    }
*/
}

void MaterialInspectorWindow::on_pbRename_clicked()
{
    if (bMaterialWasModified)
    {
        guitools::message("Material properties were modified!\nUpdate, add as new or cancel changes before renaming", this);
        return;
    }

    const QString newName = ui->leName->text();
    int iMat = ui->cobActiveMaterials->currentIndex();
    if (iMat < 0) return;

    const QString & oldName = MpCollection[iMat]->name;
    if (newName == oldName) return;

    for (int i = 0; i < MpCollection.countMaterials(); i++)
        if (i != iMat && newName == MpCollection[i]->name)
        {
            guitools::message("There is already a material with name " + newName, this);
            return;
        }

    MpCollection[iMat]->name = newName;
    ui->pbRename->setText("Rename " + newName);

    //MW->ReconstructDetector(true);
}

void MaterialInspectorWindow::on_actionSave_material_triggered()
{
  //checkig this material
  const QString error = MpCollection.CheckTmpMaterial();
  if ( !error.isEmpty() )
    {
      guitools::message(error, this);
      return;
    }

  QString starter = GlobSet.LastSaveDir;
  starter += "/Material_" + ui->leName->text();
  QString fileName = QFileDialog::getSaveFileName(this,"Save material", starter, "Material files (*.mat *.json);;All files (*.*)");
  if (fileName.isEmpty()) return;
  QFileInfo fileInfo(fileName);
  GlobSet.LastSaveDir = fileInfo.absolutePath();
  if (fileInfo.suffix().isEmpty()) fileName += ".mat";

  QJsonObject json, js;
    //MpCollection.writeMaterialToJson(imat, json);
    MpCollection.tmpMaterial.writeToJson(json);
  js["Material"] = json;
  bool bOK = jstools::saveJsonToFile(js, fileName);
  if (!bOK) guitools::message("Failed to save json to file: "+fileName, this);
}

void MaterialInspectorWindow::on_actionLoad_material_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load material", GlobSet.LastLoadDir, "Material files (*mat *.json);;All files (*.*)");
  if (fileName.isEmpty()) return;
  GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

  QJsonObject json, js;
  bool bOK = jstools::loadJsonFromFile(json, fileName);
  if (!bOK)
  {
      guitools::message("Cannot open file: "+fileName, this);
      return;
  }
  if (!json.contains("Material"))
    {
      guitools::message("File format error: Json with material settings not found", this);
      return;
    }
  js = json["Material"].toObject();
  MpCollection.tmpMaterial.readFromJson(js);

  setWasModified(true);
  updateWaveButtons();
//  MW->ListActiveParticles();

  ui->cobActiveMaterials->setCurrentIndex(-1); //to avoid confusion (and update is disabled for -1)
  LastShownMaterial = -1;

  UpdateGui(); //refresh indication of tmpMaterial
  updateWaveButtons(); //refresh button state for Wave-resolved properties
}

void MaterialInspectorWindow::on_cobYieldForParticle_activated(int index)
{
    const AMaterial & tmpMaterial = MpCollection.tmpMaterial;

    flagDisreguardChange = true; // -->
    ui->ledPrimaryYield->setText( QString::number(tmpMaterial.getPhotonYield(index)) );
    ui->ledIntEnergyRes->setText( QString::number(tmpMaterial.getIntrinsicEnergyResolution(index)) );
    flagDisreguardChange = false; // <--
}

void MaterialInspectorWindow::onAddIsotope(AChemicalElement *element)
{
    element->Isotopes << AIsotope(element->Symbol, 777, 0);

    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.ChemicalComposition.updateMassRelatedPoperties();

    UpdateGui();
    setWasModified(true);
}

void MaterialInspectorWindow::onRemoveIsotope(AChemicalElement *element, int isotopeIndexInElement)
{
    if (element->Isotopes.size()<2)
    {
        guitools::message("Cannot remove the last isotope!", this);
        return;
    }
    element->Isotopes.removeAt(isotopeIndexInElement);

    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.ChemicalComposition.updateMassRelatedPoperties();

    UpdateGui();
    setWasModified(true);
}

void MaterialInspectorWindow::IsotopePropertiesChanged(const AChemicalElement * /*element*/, int /*isotopeIndexInElement*/)
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    tmpMaterial.ChemicalComposition.updateMassRelatedPoperties();

    UpdateGui();
    setWasModified(true);
}

void MaterialInspectorWindow::onRequestDraw(const QVector<double> &x, const QVector<double> &y, const QString &titleX, const QString &titleY)
{
/*
    TGraph * g = MW->GraphWindow->ConstructTGraph(x, y, "", titleX, titleY, 4, 20, 1, 4, 1, 2);
    MW->GraphWindow->Draw(g, "APL");
    MW->GraphWindow->UpdateRootCanvas();
*/
}

void MaterialInspectorWindow::on_pbModifyChemicalComposition_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    QDialog* d = new QDialog(this);
    d->setWindowTitle("Enter element composition (molar fractions!)");

    QVBoxLayout* L = new QVBoxLayout();
        QHBoxLayout* l = new QHBoxLayout();
        QLineEdit* le = new QLineEdit(tmpMaterial.ChemicalComposition.getCompositionString(), this);
        le->setMinimumSize(400,25);
        QPushButton* pb = new QPushButton("Confirm", this);
        l->addWidget(le);
        l->addWidget(pb);
        connect(pb, SIGNAL(clicked(bool)), d, SLOT(accept()));
    L->addLayout(l);
    L->addWidget(new QLabel("Format examples:\n"));
    L->addWidget(new QLabel("C2H5OH   - use only integer values!"));
    L->addWidget(new QLabel("C:0.3333 + H:0.6667  -> molar fractions of 1/3 of carbon and 2/3 of hydrogen"));
    L->addWidget(new QLabel("H2O:9.0 + NaCl:0.2 -> 9.0 parts of H2O and 0.2 parts of NaCl"));
    d->setLayout(L);

    while (d->exec() != 0)
    {
        AMaterialComposition& mc = tmpMaterial.ChemicalComposition;
        QString error = mc.setCompositionString(le->text(), true);
        if (!error.isEmpty())
        {
            guitools::message(error, d);
            continue;
        }

        UpdateGui();
        break;
    }

    if (d->result() == 0) return;

    setWasModified(true);
    updateWarningIcons();
}

void MaterialInspectorWindow::on_pbModifyByWeight_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    QDialog* d = new QDialog(this);
    d->setWindowTitle("Enter element composition (fractions by weight!)");

    QVBoxLayout* L = new QVBoxLayout();
        QHBoxLayout* l = new QHBoxLayout();
        QLineEdit* le = new QLineEdit(tmpMaterial.ChemicalComposition.getCompositionByWeightString(), this);
        le->setMinimumSize(400,25);
        QPushButton* pb = new QPushButton("Confirm", this);
        l->addWidget(le);
        l->addWidget(pb);
        connect(pb, SIGNAL(clicked(bool)), d, SLOT(accept()));
    L->addLayout(l);
    L->addWidget(new QLabel("Give weight factors for each element separately, e.g.:\n"));
    L->addWidget(new QLabel("H:0.1112 + O:0.8889"));
    L->addWidget(new QLabel("\nNote that Ants will recalculate this composition to molar one,\n"
                            "and then show re-calculated weight factors with the sum of unity!\n\n"
                            "Any subsequent changes to isotope composition of involved elements\n"
                            "will modify the composition!"));
    d->setLayout(L);

    while (d->exec() != 0)
    {
        AMaterialComposition& mc = tmpMaterial.ChemicalComposition;
        QString error = mc.setCompositionByWeightString(le->text());
        if (!error.isEmpty())
        {
            guitools::message(error, d);
            continue;
        }

        UpdateGui();
        break;
    }

    if (d->result() == 0) return;

    setWasModified(true);
    updateWarningIcons();
}

void MaterialInspectorWindow::ShowTreeWithChemicalComposition()
{
    bClearInProgress = true;
    ui->trwChemicalComposition->clear();
    bClearInProgress = false;

    AMaterial& tmpMaterial = MpCollection.tmpMaterial;
    bool bShowIsotopes = ui->cbShowIsotopes->isChecked();

    for (int i=0; i<tmpMaterial.ChemicalComposition.countElements(); i++)
    {
        AChemicalElement* el = tmpMaterial.ChemicalComposition.getElement(i);

        //new element
        AChemicalElementDelegate* elDel = new AChemicalElementDelegate(el, &bClearInProgress, ui->cbShowIsotopes->isChecked());
        QTreeWidgetItem* ElItem = new QTreeWidgetItem(ui->trwChemicalComposition);
        ui->trwChemicalComposition->setItemWidget(ElItem, 0, elDel);
        ElItem->setExpanded(bShowIsotopes);
        QObject::connect(elDel, &AChemicalElementDelegate::AddIsotopeActivated, this, &MaterialInspectorWindow::onAddIsotope, Qt::QueuedConnection);

        if (bShowIsotopes)
            for (int index = 0; index <el->Isotopes.size(); index++)
            {
                AIsotopeDelegate* isotopDel = new AIsotopeDelegate(el, index, &bClearInProgress);
                QTreeWidgetItem* twi = new QTreeWidgetItem();
                ElItem->addChild(twi);
                ui->trwChemicalComposition->setItemWidget(twi, 0, isotopDel);
                QObject::connect(isotopDel, &AIsotopeDelegate::RemoveIsotope, this, &MaterialInspectorWindow::onRemoveIsotope, Qt::QueuedConnection);
                QObject::connect(isotopDel, &AIsotopeDelegate::IsotopePropertiesChanged, this, &MaterialInspectorWindow::IsotopePropertiesChanged, Qt::QueuedConnection);
            }
    }
}

void MaterialInspectorWindow::on_cbShowIsotopes_clicked()
{
    ShowTreeWithChemicalComposition();
}

void flagButton(QPushButton* pb, bool flag)
{
    QString toRed = "QPushButton {color: red;}";
    QString s = pb->styleSheet();

    if (flag)
    {
        if (!s.contains(toRed)) s += toRed;
    }
    else
    {
        if (s.contains(toRed)) s.remove(toRed);
    }

    pb->setStyleSheet(s);
}

void MaterialInspectorWindow::on_tabwNeutron_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "Menu not implemented" << ui->tabwNeutron->currentRow() << ui->tabwNeutron->currentColumn()<<pos;
}

void MaterialInspectorWindow::on_pbMaterialInfo_clicked()
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    if (ui->leChemicalComposition->text().isEmpty())
    {
        guitools::message("Chemical composition is not defined!", this);
        return;
    }

    double MAM = tmpMaterial.ChemicalComposition.getMeanAtomMass();
    QString str = "Mean atom mass: " + QString::number(MAM, 'g', 4) + " a.u.\n";
    double AtDens = tmpMaterial.density / MAM / 1.66054e-24;
    str += "Atom density: " + QString::number(AtDens, 'g', 4) + " cm-3\n";
    guitools::message(str, this);
}

void MaterialInspectorWindow::on_trwChemicalComposition_doubleClicked(const QModelIndex & /*index*/)
{
    if (!ui->cbShowIsotopes->isChecked())
    {
        ui->cbShowIsotopes->setChecked(true);
        ShowTreeWithChemicalComposition();
    }
}

void MaterialInspectorWindow::on_lePriT_editingFinished()
{
    if (bMessageLock) return;
    parseDecayOrRaiseTime(true);
}

void MaterialInspectorWindow::on_lePriT_raise_editingFinished()
{
    if (bMessageLock) return;
    parseDecayOrRaiseTime(false);
}

bool MaterialInspectorWindow::parseDecayOrRaiseTime(bool doParseDecay)
{
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    QString s = ( doParseDecay ? ui->lePriT->text() : ui->lePriT_raise->text() );
    s = s.simplified();

    QVector<APair_ValueAndWeight> & vec =
            ( doParseDecay ? tmpMaterial.PriScint_Decay : tmpMaterial.PriScint_Raise);

    vec.clear();
    bool bErrorDetected = false;

    bool bSingle;
    double tau = s.toDouble(&bSingle);
    if (bSingle)
        vec << APair_ValueAndWeight(tau, 1.0);
    else
    {
        QStringList sl = s.split('&', Qt::SkipEmptyParts);

        for (const QString& sr : sl)
        {
            QStringList oneTau = sr.split(':', Qt::SkipEmptyParts);
            if (oneTau.size() == 2)
            {
                bool bOK1, bOK2;
                double tau    = oneTau.at(0).toDouble(&bOK1);
                double weight = oneTau.at(1).toDouble(&bOK2);
                if (bOK1 && bOK2)
                    vec << APair_ValueAndWeight(tau, weight);
                else
                {
                    bErrorDetected = true;
                    break;
                }
            }
            else bErrorDetected = true;
        }
        if (vec.isEmpty()) bErrorDetected = true;
    }

    if (bErrorDetected)
    {
        bMessageLock = true;
        QString s = ( doParseDecay ? "Decay" : "Raise" );
        s += " time format error:\n\nUse a single double value of the time constant or,\n"
             "to define several exponential components, use this format:\n"
             "\n time_constant1 : stat_weight1  &  time_constant2 : stat_weight2  &  ...\ne.g., 25.5 : 0.25  &  250 : 0.75\n";
        guitools::message(s, this);
        bMessageLock = false;
    }
    else
        on_pbUpdateTmpMaterial_clicked();

    return !bErrorDetected;
}

void MaterialInspectorWindow::on_pbPriThelp_clicked()
{
    QString s = "The following is for both the decay and rise time generation:\n\n"
            "  If there is only one exponential component,"
            "  the time constant (\"decay time\") can be given directly.\n"
            "  To configure several exponential components, use\n"
            "  time_constant1 : stat_weight1  &  time_constant2 : stat_weight2  &  ...\n"
            "  e.g., 25.5 : 0.25  &  250 : 0.75\n"
            "  \n"
            "Model:\n"
            "  Yiping Shao, Phys. Med. Biol. 52 (2007) 1103–1117\n"
            "  The approach is generalised to have more than one rise/decay components.\n"
            "  Random generator is taken from G4Scintillation class of Geant4";
    guitools::message(s, this);
}

void MaterialInspectorWindow::on_pbPriT_test_clicked()
{
/*
    AMaterial& tmpMaterial = MpCollection.tmpMaterial;

    tmpMaterial.updateRuntimeProperties(); //to update sum of stat weights

    TH1D* h = new TH1D("h1", "", 1000, 0, 0);
    for (int i=0; i<1000000; i++)
        h->Fill( tmpMaterial.GeneratePrimScintTime(Detector->RandGen) );

    h->GetXaxis()->SetTitle("Time, ns");
    TString title = "Emission for ";
    title += tmpMaterial.name.toLatin1().data();
    h->SetTitle(title);
    MW->GraphWindow->Draw(h);
*/
}

void MaterialInspectorWindow::on_pbCopyPrYieldToAll_clicked()
{
    if (!guitools::confirm("Set the same primary yield value for all particles?", this)) return;

    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    double prYield = ui->ledPrimaryYield->text().toDouble();
    for (int iP = 0; iP < tmpMaterial.MatParticle.size(); iP++)
            tmpMaterial.MatParticle[iP].PhYield = prYield;
    tmpMaterial.PhotonYieldDefault = prYield;
    setWasModified(true);
}

void MaterialInspectorWindow::on_pbCopyIntrEnResToAll_clicked()
{
    if (!guitools::confirm("Set the same intrinsic energy resolution value for all particles?", this)) return;

    AMaterial & tmpMaterial = MpCollection.tmpMaterial;
    double EnRes = ui->ledIntEnergyRes->text().toDouble();
    for (int iP = 0; iP < tmpMaterial.MatParticle.size(); iP++)
            tmpMaterial.MatParticle[iP].IntrEnergyRes = EnRes;
    tmpMaterial.IntrEnResDefault = EnRes;
    setWasModified(true);
}

void MaterialInspectorWindow::on_cbTrackingAllowed_clicked()
{
    updateEnableStatus();
}

void MaterialInspectorWindow::on_cbTransparentMaterial_clicked()
{
    updateEnableStatus();
}

void MaterialInspectorWindow::on_pbSecScintHelp_clicked()
{
    QString s = "Diffusion is NOT active in \"Only photons\" simulation mode!\n"
            "\n"
            "If drift velosity is set to 0, diffusion is disabled in this material!\n"
            "\n"
            "Warning!\n"
            "There are no checks for travel back in time and superluminal speed of electrons!";
    guitools::message(s, this);
}

void MaterialInspectorWindow::on_pteComments_textChanged()
{
    if (!flagDisreguardChange) setWasModified(true);
}

/*
#include "amaterialloader.h"
void MaterialInspectorWindow::AddMaterialFromLibrary(QWidget * parentWidget)
{
    AMaterialLoader MLMpCollection;

    bool bLoaded = ML.LoadTmpMatFromGui(parentWidget);
    if (!bLoaded) return;

    const QString name = MpCollection.tmpMaterial.name;
    MW->ReconstructDetector(true);   // TODO: go for detector directly  --> move to loader
    int index = MpCollection.FindMaterial(name);

    showMaterial(index);
}
*/

/*
void MaterialInspectorWindow::on_actionLoad_from_material_library_triggered()
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Add new material", "All unsaved changes will be lost. Continue?", QMessageBox::Yes | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel)
            return;
    }

    AddMaterialFromLibrary(this);
}
*/

void MaterialInspectorWindow::on_actionAdd_default_material_triggered()
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Add new material", "All unsaved changes will be lost. Continue?", QMessageBox::Yes | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel)
            return;
    }

    MpCollection.AddNewMaterial("Not_defined", true);
    //MW->ReconstructDetector(true);

    int index = ui->cobActiveMaterials->count() - 1;
    if (index > -1)
        showMaterial(index);
}

void MaterialInspectorWindow::on_cbG4Material_toggled(bool)
{
    updateG4RelatedGui();
}
