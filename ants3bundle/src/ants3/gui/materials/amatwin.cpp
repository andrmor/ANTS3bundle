#include "amatwin.h"
#include "ui_amatwin.h"
#include "mainwindow.h"
#include "amaterialhub.h"
#include "ageometryhub.h"
#include "a3global.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "guitools.h"
#include "acommonfunctions.h"
#include "achemicalelement.h"
#include "aelementandisotopedelegates.h"
#include "ageometrywindow.h"
#include "agraphbuilder.h"

#include <QDebug>
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

#include "TGraph.h"
#include "TH1.h"
#include "TAxis.h"
#include "TGeoManager.h"
#include "TAttLine.h"
#include "TAttMarker.h"

AMatWin::AMatWin(QWidget * parent) :
    AGuiWindow("Mat", parent),
    Geometry(AGeometryHub::getInstance()),
    MatHub(AMaterialHub::getInstance()),
    GlobSet(A3Global::getInstance()),
    ui(new Ui::AMatWin)
{
    ui->setupUi(this);

    this->move(15,15);

    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    //windowFlags |= Qt::Tool;
    this->setWindowFlags( windowFlags );

    DefaultPBStyle = ui->pbAcceptChanges->styleSheet();

    configureG4Materials();

    ui->pbWasModified->setVisible(false);
    ui->labContextMenuHelp->setVisible(false);
    ui->pbUpdateTmpMaterial->setVisible(false);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    connect(&MatHub, &AMaterialHub::materialsChanged, this, &AMatWin::onMaterialsChanged);

    ui->leChemicalComposition->installEventFilter(this);
    ui->leCompositionByWeight->installEventFilter(this);
}

AMatWin::~AMatWin()
{
    delete ui;
}

void AMatWin::initWindow()
{
    updateGui();
    switchToMaterial(0);
}

bool AMatWin::eventFilter(QObject * object, QEvent * event)
{
    if(object == ui->leChemicalComposition && event->type() == QEvent::MouseButtonPress) // FocusIn
    {
        modifyChemicalComposition();
        return true;
    }
    if(object == ui->leCompositionByWeight && event->type() == QEvent::MouseButtonPress) // FocusIn
    {
        modifyByWeight();
        return true;
    }
    return false;
}

void AMatWin::setWasModified(bool flag)
{
    if (flagDisreguardChange) return;

    bMaterialWasModified = flag;
    ui->pbAcceptChanges->setStyleSheet( flag ? "QPushButton {color: red;}" : DefaultPBStyle );

    updateActionButtons();
}

void AMatWin::updateGui()
{   
    if (bLockTmpMaterial) return;

    int current = ui->cobActiveMaterials->currentIndex();

    ui->cobActiveMaterials->clear();
    ui->cobActiveMaterials->addItems(MatHub.getListOfMaterialNames());

    int matCount = ui->cobActiveMaterials->count();
    if (current < -1 || current >= matCount)
        current = matCount - 1;

    switchToMaterial(current);
    setWasModified(false);
}

bool AMatWin::checkCurrentMaterial()
{
    if ( !parseDecayOrRaiseTime(true) )  return false;  //error messaging inside
    if ( !parseDecayOrRaiseTime(false) ) return false;  //error messaging inside

    QString error = tmpMaterial.checkMaterial();
    if (!error.isEmpty())
    {
        guitools::message(error, this);
        return false;
    }

    return true;
}

void AMatWin::switchToMaterial(int index)
{
    if (index == -1)
    {
        ui->cobActiveMaterials->setCurrentIndex(-1);
        LastShownMaterial = index;
        return;
    }

    if (index < 0 || index >= MatHub.countMaterials()) return;

    MatHub.copyMaterialToTmp(index, tmpMaterial);
    ui->cobActiveMaterials->setCurrentIndex(index);
    LastShownMaterial = index;
    updateTmpMaterialGui();

    setWasModified(false);
}

void AMatWin::on_cobActiveMaterials_activated(int index)
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
    switchToMaterial(index);
}

void AMatWin::updateWaveButtons()
{   
    bool bPrimSpec = !tmpMaterial.PrimarySpectrum.empty();
    ui->pbShowPrimSpectrum->setEnabled(bPrimSpec);
    ui->pbDeletePrimSpectrum->setEnabled(bPrimSpec);

    bool bSecSpec = !tmpMaterial.SecondarySpectrum.empty();
    ui->pbShowSecSpectrum->setEnabled(bSecSpec);
    ui->pbDeleteSecSpectrum->setEnabled(bSecSpec);

    bool bN = !tmpMaterial.RefIndex_Wave.empty();
    ui->pbShowNlambda->setEnabled(bN);
    ui->pbDeleteNlambda->setEnabled(bN);

    bool bA = !tmpMaterial.AbsCoeff_Wave.empty();
    ui->pbShowABSlambda->setEnabled(bA);
    ui->pbDeleteABSlambda->setEnabled(bA);

    bool bCompexN = !tmpMaterial.RefIndexComplex_Wave.empty();
    ui->pbShowComplexN->setEnabled(bCompexN);
    ui->pbDeleteComplexN->setEnabled(bCompexN);

    bool bR = !tmpMaterial.ReemissionProb_Wave.empty();
    ui->pbShowReemProbLambda->setEnabled(bR);
    ui->pbDeleteReemisProbLambda->setEnabled(bR);
}

void AMatWin::updateG4RelatedGui()
{
    bool bDisable = ui->cbG4Material->isChecked();
    std::vector<QWidget*> widgs = {ui->ledDensity, ui->pbMaterialInfo, ui->ledT, ui->leChemicalComposition,
                                   ui->leCompositionByWeight, ui->trwChemicalComposition, ui->cbShowIsotopes};
    for (QWidget * w : widgs) w->setDisabled(bDisable);
}

#include <QCompleter>
#include <QStringListModel>
void AMatWin::configureG4Materials()
{
    const QStringList mats{"G4_A-150_TISSUE", "G4_ACETONE", "G4_ACETYLENE", "G4_ADENINE", "G4_ADIPOSE_TISSUE_ICRP", "G4_AIR", "G4_ALANINE", "G4_ALUMINUM_OXIDE",
                           "G4_AMBER", "G4_AMMONIA", "G4_ANILINE", "G4_ANTHRACENE", "G4_B-100_BONE", "G4_BAKELITE", "G4_BARIUM_FLUORIDE", "G4_BARIUM_SULFATE",
                           "G4_BENZENE", "G4_BERYLLIUM_OXIDE", "G4_BGO", "G4_BLOOD_ICRP", "G4_BONE_COMPACT_ICRU", "G4_BONE_CORTICAL_ICRP", "G4_BORON_CARBIDE",
                           "G4_BORON_OXIDE", "G4_BRAIN_ICRP", "G4_BUTANE", "G4_N-BUTYL_ALCOHOL", "G4_C-552", "G4_CADMIUM_TELLURIDE", "G4_CADMIUM_TUNGSTATE",
                           "G4_CALCIUM_CARBONATE", "G4_CALCIUM_FLUORIDE", "G4_CALCIUM_OXIDE", "G4_CALCIUM_SULFATE", "G4_CALCIUM_TUNGSTATE", "G4_CARBON_DIOXIDE",
                           "G4_CARBON_TETRACHLORIDE", "G4_CELLULOSE_CELLOPHANE", "G4_CELLULOSE_BUTYRATE", "G4_CELLULOSE_NITRATE", "G4_CERIC_SULFATE",
                           "G4_CESIUM_FLUORIDE", "G4_CESIUM_IODIDE", "G4_CHLOROBENZENE", "G4_CHLOROFORM", "G4_CONCRETE", "G4_CYCLOHEXANE",
                           "G4_1,2-DICHLOROBENZENE", "G4_DICHLORODIETHYL_ETHER", "G4_1,2-DICHLOROETHANE", "G4_DIETHYL_ETHER", "G4_N,N-DIMETHYL_FORMAMIDE",
                           "G4_DIMETHYL_SULFOXIDE", "G4_ETHANE", "G4_ETHYL_ALCOHOL", "G4_ETHYL_CELLULOSE", "G4_ETHYLENE", "G4_EYE_LENS_ICRP", "G4_FERRIC_OXIDE",
                           "G4_FERROBORIDE", "G4_FERROUS_OXIDE", "G4_FERROUS_SULFATE", "G4_FREON-12", "G4_FREON-12B2", "G4_FREON-13", "G4_FREON-13B1",
                           "G4_FREON-13I1", "G4_GADOLINIUM_OXYSULFIDE", "G4_GALLIUM_ARSENIDE", "G4_GEL_PHOTO_EMULSION", "G4_Pyrex_Glass", "G4_GLASS_LEAD",
                           "G4_GLASS_PLATE", "G4_GLUTAMINE", "G4_GLYCEROL", "G4_GUANINE", "G4_GYPSUM", "G4_N-HEPTANE", "G4_N-HEXANE", "G4_KAPTON",
                           "G4_LANTHANUM_OXYBROMIDE", "G4_LANTHANUM_OXYSULFIDE", "G4_LEAD_OXIDE", "G4_LITHIUM_AMIDE", "G4_LITHIUM_CARBONATE",
                           "G4_LITHIUM_FLUORIDE", "G4_LITHIUM_HYDRIDE", "G4_LITHIUM_IODIDE", "G4_LITHIUM_OXIDE", "G4_LITHIUM_TETRABORATE", "G4_LUNG_ICRP",
                           "G4_M3_WAX", "G4_MAGNESIUM_CARBONATE", "G4_MAGNESIUM_FLUORIDE", "G4_MAGNESIUM_OXIDE", "G4_MAGNESIUM_TETRABORATE", "G4_MERCURIC_IODIDE",
                           "G4_METHANE", "G4_METHANOL", "G4_MIX_D_WAX", "G4_MS20_TISSUE", "G4_MUSCLE_SKELETAL_ICRP", "G4_MUSCLE_STRIATED_ICRU",
                           "G4_MUSCLE_WITH_SUCROSE", "G4_MUSCLE_WITHOUT_SUCROSE", "G4_NAPHTHALENE", "G4_NITROBENZENE", "G4_NITROUS_OXIDE", "G4_NYLON-8062",
                           "G4_NYLON-6-6", "G4_NYLON-6-10", "G4_NYLON-11_RILSAN", "G4_OCTANE", "G4_PARAFFIN", "G4_N-PENTANE", "G4_PHOTO_EMULSION",
                           "G4_PLASTIC_SC_VINYLTOLUENE", "G4_PLUTONIUM_DIOXIDE", "G4_POLYACRYLONITRILE", "G4_POLYCARBONATE", "G4_POLYCHLOROSTYRENE",
                           "G4_POLYETHYLENE", "G4_MYLAR", "G4_PLEXIGLASS", "G4_POLYOXYMETHYLENE", "G4_POLYPROPYLENE", "G4_POLYSTYRENE", "G4_TEFLON",
                           "G4_POLYTRIFLUOROCHLOROETHYLENE", "G4_POLYVINYL_ACETATE", "G4_POLYVINYL_ALCOHOL", "G4_POLYVINYL_BUTYRAL", "G4_POLYVINYL_CHLORIDE",
                           "G4_POLYVINYLIDENE_CHLORIDE", "G4_POLYVINYLIDENE_FLUORIDE", "G4_POLYVINYL_PYRROLIDONE", "G4_POTASSIUM_IODIDE", "G4_POTASSIUM_OXIDE",
                           "G4_PROPANE", "G4_lPROPANE", "G4_N-PROPYL_ALCOHOL", "G4_PYRIDINE", "G4_RUBBER_BUTYL", "G4_RUBBER_NATURAL", "G4_RUBBER_NEOPRENE",
                           "G4_SILICON_DIOXIDE", "G4_SILVER_BROMIDE", "G4_SILVER_CHLORIDE", "G4_SILVER_HALIDES", "G4_SILVER_IODIDE", "G4_SKIN_ICRP",
                           "G4_SODIUM_CARBONATE", "G4_SODIUM_IODIDE", "G4_SODIUM_MONOXIDE", "G4_SODIUM_NITRATE", "G4_STILBENE", "G4_SUCROSE", "G4_TERPHENYL",
                           "G4_TESTIS_ICRP", "G4_TETRACHLOROETHYLENE", "G4_THALLIUM_CHLORIDE", "G4_TISSUE_SOFT_ICRP", "G4_TISSUE_SOFT_ICRU-4", "G4_TISSUE-METHANE",
                           "G4_TISSUE-PROPANE", "G4_TITANIUM_DIOXIDE", "G4_TOLUENE", "G4_TRICHLOROETHYLENE", "G4_TRIETHYL_PHOSPHATE", "G4_TUNGSTEN_HEXAFLUORIDE",
                           "G4_URANIUM_DICARBIDE", "G4_URANIUM_MONOCARBIDE", "G4_URANIUM_OXIDE", "G4_UREA", "G4_VALINE", "G4_VITON", "G4_WATER", "G4_WATER_VAPOR",
                           "G4_XYLENE", "G4_GRAPHITE", "G4_lH2", "G4_lN2", "G4_lO2", "G4_lAr", "G4_lBr", "G4_lKr", "G4_lXe", "G4_PbWO4", "G4_Galactic",
                           "G4_GRAPHITE_POROUS", "G4_LUCITE", "G4_BRASS", "G4_BRONZE", "G4_STAINLESS-STEEL", "G4_CR39", "G4_OCTADECANOL", "G4_KEVLAR",
                           "G4_DACRON", "G4_NEOPRENE", "G4_CYTOSINE", "G4_THYMINE", "G4_URACIL", "G4_DNA_ADENINE", "G4_DNA_GUANINE", "G4_DNA_CYTOSINE",
                           "G4_DNA_THYMINE", "G4_DNA_URACIL", "G4_DNA_ADENOSINE", "G4_DNA_GUANOSINE", "G4_DNA_CYTIDINE", "G4_DNA_URIDINE", "G4_DNA_METHYLURIDINE",
                           "G4_DNA_MONOPHOSPHATE", "G4_DNA_A", "G4_DNA_G", "G4_DNA_C", "G4_DNA_U", "G4_DNA_MU"};

    QCompleter * Completer = new QCompleter(this);
    QStringListModel * CompletitionModel = new QStringListModel(mats, this);
    Completer->setModel(CompletitionModel);
    Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    //completer->setCompletionMode(QCompleter::PopupCompletion);
    Completer->setFilterMode(Qt::MatchContains);
    //completer->setFilterMode(Qt::MatchStartsWith);
    Completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    Completer->setCaseSensitivity(Qt::CaseSensitive);
    Completer->setWrapAround(false);
    ui->leG4Material->setCompleter(Completer);
}

void AMatWin::updateTmpMaterialGui()
{
    ui->leName->setText(tmpMaterial.Name);

    ui->ledDensity->setText( QString::number(tmpMaterial.Density) );
    ui->ledT->setText( QString::number(tmpMaterial.Temperature) );

    ui->leChemicalComposition->setText( tmpMaterial.Composition.getCompositionString() );
    ui->leCompositionByWeight->setText( tmpMaterial.Composition.getCompositionByWeightString() );
    ShowTreeWithChemicalComposition();

    ui->cbG4Material->setChecked(tmpMaterial.UseNistMaterial);
    ui->leG4Material->setText(tmpMaterial.NistMaterial);
    updateG4RelatedGui();

    ui->ledN->setText( QString::number(tmpMaterial.RefIndex) );
    ui->ledAbs->setText( QString::number(tmpMaterial.AbsCoeff) );

    ui->cobNAbsOrComplex->setCurrentIndex(tmpMaterial.Dielectric ? 0 : 1);
    ui->ledReN->setText(QString::number(tmpMaterial.RefIndexComplex.real()));
    ui->ledImN->setText(QString::number(tmpMaterial.RefIndexComplex.imag()));

    ui->ledReemissionProbability->setText( QString::number(tmpMaterial.ReemissionProb) );

    QString s = ( tmpMaterial.RayleighMFP > 0 ? QString::number(tmpMaterial.RayleighMFP)
                                              : "" );
    ui->ledRayleigh->setText(s);
    ui->ledRayleighWave->setText( QString::number(tmpMaterial.RayleighWave) );

    //decay time
    if ( tmpMaterial.PriScint_Decay.empty() )
        s = "0";
    else if (tmpMaterial.PriScint_Decay.size() == 1)
        s = QString::number( tmpMaterial.PriScint_Decay.front().first );
    else
    {
        s.clear();
        for (const auto & pair : tmpMaterial.PriScint_Decay)
        {
            s += QString::number(pair.first);
            s += ":";
            s += QString::number(pair.second);
            s += " & ";
        }
        s.chop(3);
    }
    ui->lePriT->setText(s);

    //rise time
    if ( tmpMaterial.PriScint_Raise.empty() )
        s = "0";
    else if (tmpMaterial.PriScint_Raise.size() == 1)
        s = QString::number(tmpMaterial.PriScint_Raise.front().first);
    else
    {
        s.clear();
        for (const auto & pair : tmpMaterial.PriScint_Raise)
        {
            s += QString::number(pair.first);
            s += ":";
            s += QString::number(pair.second);
            s += " & ";
        }
        s.chop(3);
    }
    ui->lePriT_raise->setText(s);

    ui->ledW->setText( QString::number(tmpMaterial.W*1000.0) );  // keV->eV
    ui->ledSecYield->setText( QString::number(tmpMaterial.SecScintPhotonYield) );
    ui->ledSecT->setText( QString::number(tmpMaterial.SecScintDecayTime) );
    ui->ledEDriftVelocity->setText( QString::number(tmpMaterial.ElDriftVelocity) );
    ui->ledEDiffL->setText( QString::number(tmpMaterial.ElDiffusionL) );
    ui->ledEDiffT->setText( QString::number(tmpMaterial.ElDiffusionT) );

    ui->ledPrimaryYield->setText(QString::number(tmpMaterial.PhotonYield));
    ui->ledIntEnergyRes->setText(QString::number(tmpMaterial.IntrEnergyRes));

    ui->pteComments->clear();
    ui->pteComments->appendPlainText(tmpMaterial.Comments);

    QString sTags;
    for (const QString & s : qAsConst(tmpMaterial.Tags))
        sTags.append(s.simplified() + ", ");
    if (sTags.size() > 1) sTags.chop(2);
    ui->leTags->setText(sTags);

    updateWaveButtons();
    updateWarningIcons();
}

void AMatWin::updateWarningIcons()
{
    if (tmpMaterial.Composition.countElements() == 0)
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

void AMatWin::on_pbUpdateTmpMaterial_clicked()
{  
    tmpMaterial.Name = ui->leName->text();
    tmpMaterial.Density = ui->ledDensity->text().toDouble();
    tmpMaterial.Temperature = ui->ledT->text().toDouble();
    tmpMaterial.RefIndex = ui->ledN->text().toDouble();
    tmpMaterial.AbsCoeff = ui->ledAbs->text().toDouble();
    tmpMaterial.ReemissionProb = ui->ledReemissionProbability->text().toDouble();

    tmpMaterial.Dielectric = (ui->cobNAbsOrComplex->currentIndex() == 0);
    tmpMaterial.RefIndexComplex = { ui->ledReN->text().toDouble(), ui->ledImN->text().toDouble() };

    tmpMaterial.PhotonYield = ui->ledPrimaryYield->text().toDouble();
    //tmpMaterial.IntrEnResDefault   = ui->ledIntEnergyRes->text().toDouble(); //custom procedure on editing finished!

    tmpMaterial.W = ui->ledW->text().toDouble()*0.001; //eV -> keV
    tmpMaterial.SecScintPhotonYield = ui->ledSecYield->text().toDouble();
    tmpMaterial.SecScintDecayTime = ui->ledSecT->text().toDouble();
    tmpMaterial.ElDriftVelocity = ui->ledEDriftVelocity->text().toDouble();

    tmpMaterial.ElDiffusionL = ui->ledEDiffL->text().toDouble();
    tmpMaterial.ElDiffusionT = ui->ledEDiffT->text().toDouble();

    tmpMaterial.Comments = ui->pteComments->document()->toPlainText();

    const QStringList slTags = ui->leTags->text().split(',', Qt::SkipEmptyParts);
    tmpMaterial.Tags.clear();
    for (const QString & s : slTags)
        tmpMaterial.Tags.push_back(s.simplified());

    tmpMaterial.UseNistMaterial = ui->cbG4Material->isChecked();
    tmpMaterial.NistMaterial = ui->leG4Material->text();
}

void AMatWin::setMaterial(int index)
{
    switchToMaterial(index);
}

void AMatWin::onMaterialsChanged()
{
    updateGui();
}

void AMatWin::on_ledIntEnergyRes_editingFinished()
{
    double newVal = ui->ledIntEnergyRes->text().toDouble();
    if (newVal < 0)
    {
        ui->ledIntEnergyRes->setText("0");
        guitools::message("Intrinsic energy resolution cannot be negative", this);
        return;
    }

    tmpMaterial.IntrEnergyRes = newVal;
    setWasModified(true);
}

void AMatWin::on_pbLoadPrimSpectrum_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load primary scintillation spectrum", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadPairs(fileName, tmpMaterial.PrimarySpectrum);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.PrimarySpectrum.empty();
    ui->pbShowPrimSpectrum->setEnabled(bHaveData);
    ui->pbDeletePrimSpectrum->setEnabled(bHaveData);
    setWasModified(true);
}

void AMatWin::on_pbShowPrimSpectrum_clicked()
{
    TGraph * g = AGraphBuilder::graph(tmpMaterial.PrimarySpectrum);
    AGraphBuilder::configure(g, "PrimaryScint spectrum",
                                "Wavelength, nm", "Emission probability, a.u.",
                                2, 20, 1,
                                2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AMatWin::on_pbDeletePrimSpectrum_clicked()
{
    tmpMaterial.PrimarySpectrum.clear();

    ui->pbShowPrimSpectrum->setEnabled(false);
    ui->pbDeletePrimSpectrum->setEnabled(false);
    setWasModified(true);
}

void AMatWin::on_pbLoadSecSpectrum_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load secondary scintillation spectrum", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadPairs(fileName, tmpMaterial.SecondarySpectrum);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.SecondarySpectrum.empty();
    ui->pbShowSecSpectrum->setEnabled(bHaveData);
    ui->pbDeleteSecSpectrum->setEnabled(bHaveData);
    setWasModified(true);
}

void AMatWin::on_pbShowSecSpectrum_clicked()
{
    TGraph * g = AGraphBuilder::graph(tmpMaterial.SecondarySpectrum);
    AGraphBuilder::configure(g, "SecScint spectrum",
                                "Wavelength, nm", "Emission probability, a.u.",
                                2, 20, 1,
                                2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AMatWin::on_pbDeleteSecSpectrum_clicked()
{
    tmpMaterial.SecondarySpectrum.clear();

    ui->pbShowSecSpectrum->setEnabled(false);
    ui->pbDeleteSecSpectrum->setEnabled(false);
    setWasModified(true);
}

void AMatWin::on_pbLoadNlambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load refractive index data", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadPairs(fileName, tmpMaterial.RefIndex_Wave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.RefIndex_Wave.empty();
    ui->pbShowNlambda->setEnabled(bHaveData);
    ui->pbDeleteNlambda->setEnabled(bHaveData);
    setWasModified(true);
}

void AMatWin::on_pbShowNlambda_clicked()
{
    TGraph * g = AGraphBuilder::graph(tmpMaterial.RefIndex_Wave);
    AGraphBuilder::configure(g, "Refractive index",
                                "Wavelength, nm", "Refractive index",
                                2, 20, 1,
                                2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AMatWin::on_pbDeleteNlambda_clicked()
{
    tmpMaterial.RefIndex_Wave.clear();

    ui->pbShowNlambda->setEnabled(false);
    ui->pbDeleteNlambda->setEnabled(false);

    setWasModified(true);
}

void AMatWin::on_pbLoadABSlambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load exponential bulk absorption data", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadPairs(fileName, tmpMaterial.AbsCoeff_Wave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.AbsCoeff_Wave.empty();
    ui->pbShowABSlambda->setEnabled(bHaveData);
    ui->pbDeleteABSlambda->setEnabled(bHaveData);
    setWasModified(true);
}

void AMatWin::on_pbShowABSlambda_clicked()
{
    TGraph * g = AGraphBuilder::graph(tmpMaterial.AbsCoeff_Wave);
    AGraphBuilder::configure(g, "Attenuation coefficient",
                                "Wavelength, nm", "Attenuation coefficient, mm^{-1}",
                                2, 20, 1,
                                2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AMatWin::on_pbDeleteABSlambda_clicked()
{
    tmpMaterial.AbsCoeff_Wave.clear();

    ui->pbShowABSlambda->setEnabled(false);
    ui->pbDeleteABSlambda->setEnabled(false);
    setWasModified(true);
}

void AMatWin::on_pbShowReemProbLambda_clicked()
{
    TGraph * g = AGraphBuilder::graph(tmpMaterial.ReemissionProb_Wave);
    AGraphBuilder::configure(g, "Reemission probability",
                                "Wavelength, nm", "Reemission probability",
                                2, 20, 1,
                                2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AMatWin::on_pbLoadReemisProbLambda_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load reemission probability vs wavelength", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadPairs(fileName, tmpMaterial.ReemissionProb_Wave);  //cleans previous data too
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.ReemissionProb_Wave.empty();
    ui->pbShowReemProbLambda->setEnabled(bHaveData);
    ui->pbDeleteReemisProbLambda->setEnabled(bHaveData);
    setWasModified(true);
}

void AMatWin::on_pbDeleteReemisProbLambda_clicked()
{
    tmpMaterial.ReemissionProb_Wave.clear();

    ui->pbShowReemProbLambda->setEnabled(false);
    ui->pbDeleteReemisProbLambda->setEnabled(false);
    setWasModified(true);
}

void AMatWin::on_pbWasModified_clicked()
{
    setWasModified(true);
}

void AMatWin::on_leName_editingFinished()
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

void AMatWin::on_leName_textChanged(const QString & /*name*/)
{
    setWasModified(true);
}

void AMatWin::updateActionButtons()
{
    //ui->pbAcceptChanges->setEnabled(bMaterialWasModified);
    ui->frAcceptCancel->setEnabled(bMaterialWasModified);

    ui->pbAddNew->setEnabled(!bMaterialWasModified);
    ui->pbClone->setEnabled(!bMaterialWasModified);
}

void AMatWin::on_ledRayleighWave_editingFinished()
{
    double wave = ui->ledRayleighWave->text().toDouble();
    if (wave <= 0)
    {
        guitools::message("Wavelength should be a positive number!", this);
        wave = 500.0;
        ui->ledRayleighWave->setText("500");
    }

    tmpMaterial.RayleighWave = wave;
}

void AMatWin::on_ledRayleigh_textChanged(const QString &arg1)
{
    if (arg1 == "") ui->ledRayleighWave->setEnabled(false);
    else ui->ledRayleighWave->setEnabled(true);
}

void AMatWin::on_ledRayleigh_editingFinished()
{
    double ray;
    if (ui->ledRayleigh->text() == "") ray = 0;
    else ray = ui->ledRayleigh->text().toDouble();
    tmpMaterial.RayleighMFP = ray;
}

void AMatWin::on_pbRemoveRayleigh_clicked()
{
    ui->ledRayleigh->setText("");
    tmpMaterial.RayleighMFP = 0;
    setWasModified(true);
}

void AMatWin::on_pbShowUsage_clicked()
{
    QString name = tmpMaterial.Name;
    int index = ui->cobActiveMaterials->currentIndex();

    bool flagFound = false;
    TObjArray * list = Geometry.GeoManager->GetListOfVolumes();
    int size = list->GetEntries();
    for (int i = 0; i < size; i++)
    {
        TGeoVolume * vol = (TGeoVolume*)list->At(i);
        if (!vol) break;
        if (index == vol->GetMaterial()->GetIndex())
        {
            flagFound = true;
            break;
        }
    }

    Geometry.GeoManager->ClearTracks();
    if (flagFound)
    {
        Geometry.colorVolumes(2, index);
        emit requestShowGeometry(true, true, false);
    }
    else
    {
        emit requestShowGeometry(false, true, false);
        guitools::message("Current detector configuration does not have objects referring to material "+name, this);
    }
}

void AMatWin::on_actionSave_material_triggered()
{
    //checkig this material
    QString error = tmpMaterial.checkMaterial();
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

    tmpMaterial.writeToJson(json);
    js["Material"] = json;
    bool bOK = jstools::saveJsonToFile(js, fileName);
    if (!bOK) guitools::message("Failed to save json to file: "+fileName, this);
}

void AMatWin::on_actionLoad_material_triggered()
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

tmpMaterial.readFromJson(js);

    setWasModified(true);
    updateWaveButtons();
    //  MW->ListActiveParticles();

    ui->cobActiveMaterials->setCurrentIndex(-1); //to avoid confusion (and update is disabled for -1)
    LastShownMaterial = -1;

    updateTmpMaterialGui(); //refresh indication of tmpMaterial
    updateWaveButtons(); //refresh button state for Wave-resolved properties
}

void AMatWin::onAddIsotope(AChemicalElement *element)
{
    element->Isotopes << AIsotope(element->Symbol, 777, 0);
    tmpMaterial.Composition.updateMassRelatedPoperties();

    updateTmpMaterialGui();
    setWasModified(true);
}

void AMatWin::onRemoveIsotope(AChemicalElement *element, int isotopeIndexInElement)
{
    if (element->Isotopes.size()<2)
    {
        guitools::message("Cannot remove the last isotope!", this);
        return;
    }
    element->Isotopes.removeAt(isotopeIndexInElement);

    tmpMaterial.Composition.updateMassRelatedPoperties();

    updateTmpMaterialGui();
    setWasModified(true);
}

void AMatWin::IsotopePropertiesChanged(const AChemicalElement * /*element*/, int /*isotopeIndexInElement*/)
{
    tmpMaterial.Composition.updateMassRelatedPoperties();

    updateTmpMaterialGui();
    setWasModified(true);
}

void AMatWin::onRequestDraw(const QVector<double> &x, const QVector<double> &y, const QString &titleX, const QString &titleY)
{
    /*
    TGraph * g = MW->GraphWindow->ConstructTGraph(x, y, "", titleX, titleY, 4, 20, 1, 4, 1, 2);
    MW->GraphWindow->Draw(g, "APL");
    MW->GraphWindow->UpdateRootCanvas();
*/
}

void AMatWin::modifyChemicalComposition()
{
    QDialog* d = new QDialog(this);
    d->setWindowTitle("Enter element composition (molar fractions!)");

    QVBoxLayout* L = new QVBoxLayout();
    QHBoxLayout* l = new QHBoxLayout();
    QLineEdit* le = new QLineEdit(tmpMaterial.Composition.getCompositionString(), this);
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
        AMaterialComposition& mc = tmpMaterial.Composition;
        QString error = mc.setCompositionString(le->text(), true);
        if (!error.isEmpty())
        {
            guitools::message(error, d);
            continue;
        }

        updateTmpMaterialGui();
        break;
    }

    if (d->result() == 0) return;

    setWasModified(true);
    updateWarningIcons();
}

void AMatWin::modifyByWeight()
{
    QDialog* d = new QDialog(this);
    d->setWindowTitle("Enter element composition (fractions by weight!)");

    QVBoxLayout* L = new QVBoxLayout();
    QHBoxLayout* l = new QHBoxLayout();
    QLineEdit* le = new QLineEdit(tmpMaterial.Composition.getCompositionByWeightString(), this);
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
        AMaterialComposition& mc = tmpMaterial.Composition;
        QString error = mc.setCompositionByWeightString(le->text());
        if (!error.isEmpty())
        {
            guitools::message(error, d);
            continue;
        }

        updateTmpMaterialGui();
        break;
    }

    if (d->result() == 0) return;

    setWasModified(true);
    updateWarningIcons();
}

void AMatWin::ShowTreeWithChemicalComposition()
{
    bClearInProgress = true;
    ui->trwChemicalComposition->clear();
    bClearInProgress = false;

    bool bShowIsotopes = ui->cbShowIsotopes->isChecked();

    for (int i=0; i<tmpMaterial.Composition.countElements(); i++)
    {
        AChemicalElement* el = tmpMaterial.Composition.getElement(i);

        //new element
        AChemicalElementDelegate* elDel = new AChemicalElementDelegate(el, &bClearInProgress, ui->cbShowIsotopes->isChecked());
        QTreeWidgetItem* ElItem = new QTreeWidgetItem(ui->trwChemicalComposition);
        ui->trwChemicalComposition->setItemWidget(ElItem, 0, elDel);
        ElItem->setExpanded(bShowIsotopes);
        QObject::connect(elDel, &AChemicalElementDelegate::AddIsotopeActivated, this, &AMatWin::onAddIsotope, Qt::QueuedConnection);

        if (bShowIsotopes)
            for (int index = 0; index <el->Isotopes.size(); index++)
            {
                AIsotopeDelegate* isotopDel = new AIsotopeDelegate(el, index, &bClearInProgress);
                QTreeWidgetItem* twi = new QTreeWidgetItem();
                ElItem->addChild(twi);
                ui->trwChemicalComposition->setItemWidget(twi, 0, isotopDel);
                QObject::connect(isotopDel, &AIsotopeDelegate::RemoveIsotope, this, &AMatWin::onRemoveIsotope, Qt::QueuedConnection);
                QObject::connect(isotopDel, &AIsotopeDelegate::IsotopePropertiesChanged, this, &AMatWin::IsotopePropertiesChanged, Qt::QueuedConnection);
            }
    }
}

void AMatWin::on_cbShowIsotopes_clicked()
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

void AMatWin::on_pbMaterialInfo_clicked()
{
    if (ui->leChemicalComposition->text().isEmpty())
    {
        guitools::message("Chemical composition is not defined!", this);
        return;
    }

    double MAM = tmpMaterial.Composition.getMeanAtomMass();
    QString str = "Mean atom mass: " + QString::number(MAM, 'g', 4) + " a.u.\n";
    double AtDens = tmpMaterial.Density / MAM / 1.66054e-24;
    str += "Atom density: " + QString::number(AtDens, 'g', 4) + " cm-3\n";
    guitools::message(str, this);
}

void AMatWin::on_trwChemicalComposition_doubleClicked(const QModelIndex & /*index*/)
{
    if (!ui->cbShowIsotopes->isChecked())
    {
        ui->cbShowIsotopes->setChecked(true);
        ShowTreeWithChemicalComposition();
    }
}

void AMatWin::on_lePriT_editingFinished()
{
    if (bMessageLock) return;
    parseDecayOrRaiseTime(true);
}

void AMatWin::on_lePriT_raise_editingFinished()
{
    if (bMessageLock) return;
    parseDecayOrRaiseTime(false);
}

bool AMatWin::parseDecayOrRaiseTime(bool decay_or_raise)
{
    QString s = ( decay_or_raise ? ui->lePriT->text() : ui->lePriT_raise->text() );
    s = s.simplified();

    std::vector<std::pair<double,double>> & vec = ( decay_or_raise ? tmpMaterial.PriScint_Decay : tmpMaterial.PriScint_Raise);

    vec.clear();
    bool bErrorDetected = false;

    bool bSingle;
    double tau = s.toDouble(&bSingle);
    if (bSingle) vec.push_back( {tau, 1.0} );
    else
    {
        //const QStringList sl = s.split('&', Qt::SkipEmptyParts);
        const QStringList sl = s.split(QRegularExpression("(\\&|\\+)"), Qt::SkipEmptyParts);

        for (const QString & sr : sl)
        {
            QStringList oneTau = sr.split(':', Qt::SkipEmptyParts);
            if (oneTau.size() == 2)
            {
                bool bOK1, bOK2;
                double tau    = oneTau.at(0).toDouble(&bOK1);
                double weight = oneTau.at(1).toDouble(&bOK2);
                if (bOK1 && bOK2)
                    vec.push_back( {tau, weight} );
                else
                {
                    bErrorDetected = true;
                    break;
                }
            }
            else bErrorDetected = true;
        }
        if (vec.empty()) bErrorDetected = true;
    }

    if (bErrorDetected)
    {
        bMessageLock = true;
        QString s = ( decay_or_raise ? "Decay" : "Raise" );
        s += " time format error:\n\nUse a single double value of the time constant or,\n"
             "to define several exponential components, use this format:\n"
             "\n time_constant1 : stat_weight1  &  time_constant2 : stat_weight2  &  ...\ne.g., 25.5 : 0.25  &  250 : 0.75\n";
        guitools::message(s, this);
        bMessageLock = false;
        if (decay_or_raise) ui->lePriT->setFocus();
        else                ui->lePriT_raise->setFocus();
    }
    else
    {
        //on_pbUpdateTmpMaterial_clicked();
        setWasModified(true);
    }

    return !bErrorDetected;
}

void AMatWin::on_pbPriThelp_clicked()
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

#include "arandomhub.h"
void AMatWin::on_pbPriT_test_clicked()
{
    tmpMaterial.updateRuntimeProperties(); //to update sum of stat weights

    TH1D * h = new TH1D("h1", "", 1000, 0, 0);
    for (int i = 0; i < 1000000; i++)
        h->Fill( tmpMaterial.generatePrimScintTime(ARandomHub::getInstance()));

    h->GetXaxis()->SetTitle("Time, ns");
    TString title = "Time spectrum for ";
    title += tmpMaterial.Name.toLatin1().data();
    h->SetTitle(title);
    emit requestDraw(h, "hist", true, true);
}

void AMatWin::on_pbSecScintHelp_clicked()
{
    QString s = "Diffusion is NOT active in \"Only photons\" simulation mode!\n"
            "\n"
            "If drift velosity is set to 0, diffusion is disabled in this material!\n"
            "\n"
            "Warning!\n"
            "There are no checks for travel back in time and superluminal speed of electrons!";
    guitools::message(s, this);
}

void AMatWin::on_pteComments_textChanged()
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

#include <QDesktopServices>
void AMatWin::on_pbListGeant4Materials_clicked()
{
    QDesktopServices::openUrl(QUrl("https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html", QUrl::TolerantMode));
}

void AMatWin::on_cbG4Material_toggled(bool)
{
    updateG4RelatedGui();
}

void AMatWin::on_actionRemove_selected_material_triggered()
{
    on_pbRemove_clicked();
}

void AMatWin::on_pbRemove_clicked()
{
    int iMat = ui->cobActiveMaterials->currentIndex();

    bool ok = guitools::confirm(QString("Remove material %0?").arg(ui->cobActiveMaterials->currentText()), this);
    if (!ok) return;

    QString err = MatHub.tryRemoveMaterial(iMat);
    if (!err.isEmpty()) guitools::message(err, this);
    else emit requestRebuildDetector();
}

void AMatWin::on_actionAdd_default_material_triggered()
{
    on_pbAddNew_clicked();
}

void AMatWin::on_pbAddNew_clicked()
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Add new material", "All unsaved changes will be lost. Continue?", QMessageBox::Yes | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel)
            return;
    }

    QString matName = "NoName";
    int iCounter = 1;
    while (MatHub.findMaterial(matName) != -1)
        matName = QString("NoName%1").arg(iCounter++);

    MatHub.addNewMaterial(matName, true);

    emit requestRebuildDetector();
    updateGui();

    int index = ui->cobActiveMaterials->count() - 1;
    if (index > -1) switchToMaterial(index);
}

void AMatWin::on_pbClone_clicked()
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Clone material", "All unsaved changes will be lost. Continue?", QMessageBox::Yes | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel) return;
    }

    QString matName = tmpMaterial.Name + "_c";
    int iCounter = 1;
    while (MatHub.findMaterial(matName) != -1)
        matName = QString("%0_c%1").arg(tmpMaterial.Name).arg(iCounter++);

    tmpMaterial.Name = matName;
    MatHub.copyToMaterials(tmpMaterial);

    emit requestRebuildDetector();

    switchToMaterial(MatHub.countMaterials()-1);
}

void AMatWin::on_pbAcceptChanges_clicked()
{
    bool ok = checkCurrentMaterial();
    if (!ok) return;

    const QString newName = ui->leName->text();
    const int iMat = ui->cobActiveMaterials->currentIndex();
    const QString oldName = ui->cobActiveMaterials->currentText();

    tmpMaterial.Name = oldName;
    MatHub.copyToMaterials(tmpMaterial);

    if (newName != oldName)
    {
        const int iFound = MatHub.findMaterial(newName);
        if (iFound != -1 && iFound != iMat)
        {
            guitools::message("Material with this name already exists!", this);
            return;
        }

        ok = MatHub.renameMaterial(iMat, newName);
        if (!ok)
        {
            guitools::message("Unexpected error during rename procedure!", this);
            return;
        }
    }

    emit requestRebuildDetector();

    switchToMaterial(iMat);
}

void AMatWin::on_pbCancel_clicked()
{
    if (bMaterialWasModified)
    {
        int res = QMessageBox::question(this, "Material properties", "Cancel all changes?", QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::No)return;
    }
    switchToMaterial(ui->cobActiveMaterials->currentIndex());
}

void AMatWin::on_pbShowComplexN_clicked()
{
    TGraph * gre = AGraphBuilder::graph(tmpMaterial.RefIndexComplex_Wave, true);
    AGraphBuilder::configure(gre, "Real",
                                  "Wavelength, nm", "",
                                  2, 20, 1,
                                  2, 1,  1);

    double xminre, yminre, xmaxre, ymaxre;
    gre->ComputeRange(xminre, yminre, xmaxre, ymaxre);

    TGraph * gim = AGraphBuilder::graph(tmpMaterial.RefIndexComplex_Wave, false);
    AGraphBuilder::configure(gim, "Imaginary",
                                  "Wavelength, nm", "",
                                  3, 21, 1,
                                  3, 9,  1);
    double xminim, yminim, xmaxim, ymaxim;
    gim->ComputeRange(xminim, yminim, xmaxim, ymaxim);

    double min = std::min(yminre, yminim);
    double max = std::max(ymaxre, ymaxim);

    gre->SetMaximum(max*1.05);
    gre->SetMinimum(min - 0.05*fabs(min));

    emit requestDraw(gre, "APL", true, true);
    emit requestDraw(gim, "PLsame", true, true);
}
void AMatWin::on_pbLoadComplexN_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load complex refractive index vs wavelength", GlobSet.LastLoadDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();

    QString err = ftools::loadDoubleComplexPairs(fileName, tmpMaterial.RefIndexComplex_Wave, true);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    bool bHaveData = !tmpMaterial.RefIndexComplex_Wave.empty();
    ui->pbShowComplexN->setEnabled(bHaveData);
    ui->pbDeleteComplexN->setEnabled(bHaveData);
    setWasModified(true);
}
void AMatWin::on_pbDeleteComplexN_clicked()
{
    tmpMaterial.RefIndexComplex_Wave.clear();

    ui->pbShowComplexN->setEnabled(false);
    ui->pbDeleteComplexN->setEnabled(false);
    setWasModified(true);
}
