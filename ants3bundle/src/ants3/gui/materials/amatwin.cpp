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
#include "ageometrywindow.h"
#include "agraphbuilder.h"
#include "ageant4inspectormanager.h"

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
    ui->pbUpdateTmpMaterial->setVisible(false);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->leComposition->setAlignment(Qt::AlignHCenter);

    on_cbGas_toggled(ui->cbGas->isChecked());
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

void AMatWin::setWasModified(bool flag)
{
    if (flagDisreguardChange) return;

    bMaterialWasModified = flag;
    ui->pbAcceptChanges->setStyleSheet( flag ? "QPushButton {color: red;}" : DefaultPBStyle );

    updateActionButtons();
}

void AMatWin::updateGui()
{
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

    QString error = tmpMaterial.convertPressureToDensity();
    if (!error.isEmpty())
    {
        guitools::message(error, this);
        return false;
    }

    error = tmpMaterial.checkMaterial();
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

}

#include <QCompleter>
#include <QStringListModel>
void AMatWin::configureG4Materials()
{
    const QStringList mats{"G4_H",
                           "G4_He",
                           "G4_Li",
                           "G4_Be",
                           "G4_B",
                           "G4_C",
                           "G4_N",
                           "G4_O",
                           "G4_F",
                           "G4_Ne",
                           "G4_Na",
                           "G4_Mg",
                           "G4_Al",
                           "G4_Si",
                           "G4_P",
                           "G4_S",
                           "G4_Cl",
                           "G4_Ar",
                           "G4_K",
                           "G4_Ca",
                           "G4_Sc",
                           "G4_Ti",
                           "G4_V",
                           "G4_Cr",
                           "G4_Mn",
                           "G4_Fe",
                           "G4_Co",
                           "G4_Ni",
                           "G4_Cu",
                           "G4_Zn",
                           "G4_Ga",
                           "G4_Ge",
                           "G4_As",
                           "G4_Se",
                           "G4_Br",
                           "G4_Kr",
                           "G4_Rb",
                           "G4_Sr",
                           "G4_Y",
                           "G4_Zr",
                           "G4_Nb",
                           "G4_Mo",
                           "G4_Tc",
                           "G4_Ru",
                           "G4_Rh",
                           "G4_Pd",
                           "G4_Ag",
                           "G4_Cd",
                           "G4_In",
                           "G4_Sn",
                           "G4_Sb",
                           "G4_Te",
                           "G4_I",
                           "G4_Xe",
                           "G4_Cs",
                           "G4_Ba",
                           "G4_La",
                           "G4_Ce",
                           "G4_Pr",
                           "G4_Nd",
                           "G4_Pm",
                           "G4_Sm",
                           "G4_Eu",
                           "G4_Gd",
                           "G4_Tb",
                           "G4_Dy",
                           "G4_Ho",
                           "G4_Er",
                           "G4_Tm",
                           "G4_Yb",
                           "G4_Lu",
                           "G4_Hf",
                           "G4_Ta",
                           "G4_W",
                           "G4_Re",
                           "G4_Os",
                           "G4_Ir",
                           "G4_Pt",
                           "G4_Au",
                           "G4_Hg",
                           "G4_Tl",
                           "G4_Pb",
                           "G4_Bi",
                           "G4_Po",
                           "G4_At",
                           "G4_Rn",
                           "G4_Fr",
                           "G4_Ra",
                           "G4_Ac",
                           "G4_Th",
                           "G4_Pa",
                           "G4_U",
                           "G4_Np",
                           "G4_Pu",
                           "G4_Am",
                           "G4_Cm",
                           "G4_Bk",
                           "G4_Cf",
                           "G4_A-150_TISSUE", "G4_ACETONE", "G4_ACETYLENE", "G4_ADENINE", "G4_ADIPOSE_TISSUE_ICRP", "G4_AIR", "G4_ALANINE", "G4_ALUMINUM_OXIDE",
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
    //Completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    Completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    //Completer->setCaseSensitivity(Qt::CaseSensitive);
    Completer->setCaseSensitivity(Qt::CaseInsensitive);
    Completer->setWrapAround(false);
    ui->leG4Material->setCompleter(Completer);
}

void appendLabText(QLabel * lab, const QString & addText)
{
    QString text = lab->text();
    if (!text.isEmpty()) text += '\n';
    text += addText;
    lab->setText(text);
}

void AMatWin::fillElementInfo()
{
    ui->labInfoElement->clear();
    ui->labInfoAtFraction->clear();
    ui->labInfoMassFraction->clear();
    ui->labInfoIsotope->clear();

    QString allStr = tmpMaterial.Composition.printComposition();
    const QStringList lines = allStr.split('\n', Qt::SkipEmptyParts);
    for (const QString & line : lines)
    {
        const QStringList s = line.split('\t', Qt::SkipEmptyParts);
        if (s.size() < 4) continue;
        appendLabText(ui->labInfoElement,      s[0]);
        appendLabText(ui->labInfoAtFraction,   s[1]);
        appendLabText(ui->labInfoMassFraction, s[2]);
        appendLabText(ui->labInfoIsotope,      s[3]);
    }
}

std::pair<QString,int> pressureToStringAndCobIndex(double pressure_bar, const QString & units)
{
    double factor = 1.0;
    int    index = 0;
    QString sPressure;
    if      (units == "bar") {factor = 1.0; index = 0;}
    else if (units == "mbar") {factor = 1e3; index = 1;}
    else if (units == "hPa") {factor = 1e3; index = 1;}
    else qWarning() << "Unknown preffered pressure units:" << units;

    double pressure = pressure_bar * factor;
    sPressure = QString::number(pressure);
    return {sPressure, index};
}

void AMatWin::updateTmpMaterialGui()
{
    ui->leName->setText(tmpMaterial.Name);

    ui->ledDensity->setText( QString::number(tmpMaterial.Composition.Density) );
    ui->ledT->setText( QString::number(tmpMaterial.Composition.Temperature) );
    ui->cbGas->setChecked(tmpMaterial.Composition.Gas);
    if (tmpMaterial.Composition.Gas)
    {
        std::pair<QString,int> pair = pressureToStringAndCobIndex(tmpMaterial.Composition.Pressure_bar, tmpMaterial.Composition.P_gui_units);
        ui->ledPressure->setText(pair.first);
        ui->cobPressureUnits->setCurrentIndex(pair.second);
    }
    else
    {
        ui->ledPressure->setText("1");
        ui->cobPressureUnits->setCurrentIndex(0);
    }

    ui->cobMeanExcitationEnergy->setCurrentIndex(tmpMaterial.Composition.UseCustomMeanExEnergy ? 1 : 0);
    ui->frMeanExcitationEnergy->setVisible(tmpMaterial.Composition.UseCustomMeanExEnergy);
    ui->ledMeanExcitationEnergy->setText( QString::number(tmpMaterial.Composition.MeanExEnergy) );

    ui->leComposition->setText( tmpMaterial.Composition.getCompositionString() );
    fillElementInfo();

    ui->cobCompositionType->setCurrentIndex(tmpMaterial.UseG4Material ? 1 : 0);
    ui->leG4Material->setText(tmpMaterial.G4MaterialName);
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
    for (const QString & s : tmpMaterial.Tags)
        sTags.append(s.simplified() + ", ");
    if (sTags.size() > 1) sTags.chop(2);
    ui->leTags->setText(sTags);

    updateWaveButtons();
    updateWarningIcons();
}

void AMatWin::updateWarningIcons()
{
    /*
    if (tmpMaterial.Composition.countElements() == 0)
    {
        QPixmap pm(QSize(16,16));
        pm.fill(Qt::transparent);
        QPainter b(&pm);
        b.setBrush(QBrush(Qt::yellow));
        b.drawEllipse(0, 2, 10, 10);
        ui->twProperties->setTabIcon(0, QIcon(pm));
    }
    else
    */
    ui->twProperties->setTabIcon(0, QIcon());
}

double pressureToBars(double p, const QString & units)
{
    double factor = 1.0;

    if      (units == "bar")  factor = 1.0;
    else if (units == "mbar") factor = 1e-3;
    else if (units == "hPa")  factor = 1e-3;
    else qWarning() << "Not implemented pressure unit:" << units << "--> assuming bar";

    return p * factor;
}

void AMatWin::on_pbUpdateTmpMaterial_clicked()
{  
    tmpMaterial.Name = ui->leName->text();

    tmpMaterial.UseG4Material = (ui->cobCompositionType->currentIndex() == 1);
    tmpMaterial.G4MaterialName = ui->leG4Material->text();

    tmpMaterial.Composition.Density = ui->ledDensity->text().toDouble();
    tmpMaterial.Composition.Temperature = ui->ledT->text().toDouble();

    tmpMaterial.Composition.Gas = ui->cbGas->isChecked();
    if (tmpMaterial.Composition.Gas)
    {
        double p = ui->ledPressure->text().toDouble();
        QString units = ui->cobPressureUnits->currentText();
        tmpMaterial.Composition.Pressure_bar = pressureToBars(p, units);
        tmpMaterial.Composition.P_gui_units = units;
    }
    else
    {
        tmpMaterial.Composition.Pressure_bar = 1.0;
        tmpMaterial.Composition.P_gui_units = "bar";
    }

    tmpMaterial.Composition.UseCustomMeanExEnergy = (ui->cobMeanExcitationEnergy->currentIndex() == 1);
    tmpMaterial.Composition.MeanExEnergy = ui->ledMeanExcitationEnergy->text().toDouble();

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
}

void AMatWin::setMaterial(int index)
{
    switchToMaterial(index);
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

    if (name.isEmpty())
    {
        guitools::message("Name cannot be empty!", this);
        return;
    }

    QChar first = name[0];
    if ( (first >= QChar('a') && first <= QChar('z')) || (first >= QChar('A') && first <= QChar('Z'))) ;//ok
    else
    {
        //name.remove(0,1);
        //name.insert(0, "Mat");
        //ui->leName->setText(name);
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

    ui->cobActiveMaterials->setCurrentIndex(-1); //to avoid confusion (and update is disabled for -1)
    LastShownMaterial = -1;

    updateTmpMaterialGui(); //refresh indication of tmpMaterial
    updateWaveButtons(); //refresh button state for Wave-resolved properties
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

void AMatWin::on_cobCompositionType_currentIndexChanged(int /*index*/)
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

    MatHub.addNewMaterial(matName);

    emit requestRebuildDetector();
    //updateGui();

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
    tmpMaterial.Comments = ui->pteComments->toPlainText();
    bool ok = checkCurrentMaterial();
    if (!ok) return;

    const QString newName = ui->leName->text();
    const int iMat = ui->cobActiveMaterials->currentIndex();       // -1    if material was just loaded
    const QString oldName = ui->cobActiveMaterials->currentText(); // empty if material was just loaded

    if (newName != oldName)
    {
        const int iFound = MatHub.findMaterial(newName);
        if (iFound != -1 && iFound != iMat)
        {
            guitools::message("Material with this name already exists!", this);
            return;
        }
    }
    if (!oldName.isEmpty()) tmpMaterial.Name = oldName;
    MatHub.copyToMaterials(tmpMaterial);

    if (iMat != -1 && newName != oldName)
    {
        ok = MatHub.renameMaterial(iMat, newName);
        if (!ok)
        {
            guitools::message("Unexpected error during rename procedure!", this);
            return;
        }
    }

    emit requestRebuildDetector();

    switchToMaterial(iMat == -1 ? MatHub.countMaterials()-1 : iMat);
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

void AMatWin::on_pbHelpComposition_clicked()
{
    QString s = "Composition string examples\n\n"
            "C5O2H8\n"
            "C0.5O0.2H0.8     <- possible to use real numbers in formulas\n"
            "H2O:6 + C2H5OH:3.99     <- mixture using molecular(atomic) fractions\n"
            "H2O/6 + C2H5OH/3.99     <- mixture using weight fractions\n"
            "{10B:97.0+11B:3.0}4C     <- custom isotope composition\n"
            "H2O/10.6 + (NaCl:1 + KCl:2.5)/1.4     <- possible to use an arbitrary number and level of brackets\n"
            " It is possible to transform from molecular fractions to weight but not the opposite!\n"
            "";

    guitools::message1notModal(s, "Composition string", this);
}

void AMatWin::on_leComposition_editingFinished()
{
    if (ui->leComposition->text().simplified() == tmpMaterial.Composition.getCompositionString()) return;

    ui->leComposition->blockSignals(true);  // -->
    bool ok = tmpMaterial.Composition.setCompositionString(ui->leComposition->text());
    if (ok) updateTmpMaterialGui();
    else guitools::message(tmpMaterial.Composition.ErrorString, this);
    ui->leComposition->blockSignals(false);  // <--

    setWasModified(true);
}

void AMatWin::on_pbInspectG4Material_clicked()
{
    //disableInterface(true);
    qApp->processEvents();

    AGeant4InspectorManager & G4Inspector = AGeant4InspectorManager::getInstance();
    //ui->pbAbort->setEnabled(true);

    AG4MaterialRecord reply;
    G4Inspector.inspectMaterial(tmpMaterial.G4MaterialName, reply);

    if (!G4Inspector.ErrorString.isEmpty())
        guitools::message(G4Inspector.ErrorString, this);
    else
    {
        QString str;
        str += "Name:\t\t"    + reply.Name + "\n\n";
        str += "Density:\t\t" + QString::number(reply.Density) + " g/cm3" + "\n\n";
        str += "Composition\n";
        str += " by weight:\t\t" + reply.WeightFractions + '\n';
        str += " by atoms:\t\t" + (reply.AtomFractions.isEmpty() ? "Not specified" : reply.AtomFractions) + "\n\n";
        reply.Formula.remove('_');
        str += "Formula:\t\t" + (reply.Formula.isEmpty() ? "Not specified" : reply.Formula);
        str += "\n\n";
        str += "Temperature:\t" + QString::number(reply.Temperature) + " K\n\n";
        str += "Mean Excitation Energy:\t" + QString::number(reply.MeanExcitationEnergy) + " eV";

        guitools::message1notModal(str, "Geant4 material", this);
    }
}

void AMatWin::on_cobMeanExcitationEnergy_currentIndexChanged(int index)
{
    ui->frMeanExcitationEnergy->setVisible(index == 1);
}

void AMatWin::on_cbGas_toggled(bool checked)
{
    ui->labPressure->setVisible(checked);
    ui->ledPressure->setVisible(checked);
    ui->cobPressureUnits->setVisible(checked);

    ui->ledDensity->setReadOnly(checked);
    ui->ledDensity->setToolTip(checked ? "Cannot modify pressure directly if 'Gas' option is selected!" : "");
}

void AMatWin::on_ledPressure_editingFinished()
{
    on_pbUpdateTmpMaterial_clicked();
    QString err = tmpMaterial.convertPressureToDensity();
    if (!err.isEmpty()) guitools::message(err, this);
    ui->ledDensity->setText(QString::number(tmpMaterial.Composition.Density));
}

void AMatWin::on_cbGas_clicked(bool checked)
{
    if (checked) on_ledPressure_editingFinished();
    else on_pbUpdateTmpMaterial_clicked();
}

