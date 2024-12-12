#include "ainterfaceruledialog.h"
#include "ui_ainterfaceruledialog.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "ainterfacewidgetfactory.h"
#include "guitools.h"
#include "ainterfaceruletester.h"
#include "agraphbuilder.h"
#include "afiletools.h"

#include <QJsonObject>
#include <QVBoxLayout>
#include <QDebug>

#include "TGraph.h"
#include "TH1D.h"

AInterfaceRuleDialog::AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent) :
    QDialog(parent),
    MatFrom(matFrom), MatTo(matTo),
    MatHub(AMaterialHub::getInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    ui(new Ui::AInterfaceRuleDialog)
{
    ui->setupUi(this);
    setWindowTitle("Photon tracing rule for an interface");

    QStringList matNames = MatHub.getListOfMaterialNames();
    ui->labFrom->setText(matNames.at(matFrom));
    ui->labTo->setText(matNames.at(matTo));

    ui->cobType->addItem("No special rule");
    QStringList avOv = AInterfaceRule::getAllInterfaceRuleTypes();
    ui->cobType->addItems(avOv);

    MatHub.updateRuntimeProperties();

    if (rule)
    {
        Rule = AInterfaceRule::interfaceRuleFactory(rule->getType(), matFrom, matTo);
        QJsonObject json; rule->writeToJson(json); Rule->readFromJson(json);
    }

    ui->swSurfaceModel->setVisible(false);
    ui->cbKillBackRefracted->setVisible(false);

    bool symmetric = (Rule && Rule->Symmetric);
    ui->cbSymmetric->setChecked(symmetric);

    updateGui();

    TesterWindow = new AInterfaceRuleTester(Rule,  matFrom, matTo, this);
    connect(TesterWindow, &AInterfaceRuleTester::requestClearGeometryViewer, this, &AInterfaceRuleDialog::requestClearGeometryViewer);
    connect(TesterWindow, &AInterfaceRuleTester::requestDraw,                this, &AInterfaceRuleDialog::requestDraw);
    connect(TesterWindow, &AInterfaceRuleTester::requestDrawLegend,          this, &AInterfaceRuleDialog::requestDrawLegend);
    connect(TesterWindow, &AInterfaceRuleTester::requestShowTracks,          this, &AInterfaceRuleDialog::requestShowTracks);
}

AInterfaceRuleDialog::~AInterfaceRuleDialog()
{
    //qDebug() << "Destr for AInterfaceRuleDialog";
    delete ui;
    clearTmpRules();
    delete Rule;
}

AInterfaceRule * AInterfaceRuleDialog::getRule()
{
    AInterfaceRule * r = Rule;
    Rule = nullptr;
    return r;
}

bool AInterfaceRuleDialog::isSetSymmetric() const
{
    return ui->cbSymmetric->isChecked();
}

void AInterfaceRuleDialog::updateGui()
{
    if (CustomWidget)
    {
        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        l->removeWidget(CustomWidget);
        delete CustomWidget; CustomWidget = nullptr;
    }

    if (Rule)
    {
        ui->frNoOverride->setVisible(false);
        ui->pbTestOverride->setEnabled(true);

        QStringList avOv = AInterfaceRule::getAllInterfaceRuleTypes();
        int index = avOv.indexOf(Rule->getType()); //TODO -> if not found?
        ui->cobType->setCurrentIndex(index+1);

        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        //customWidget = ovLocal->getEditWidget(this, MW->GraphWindow);
        CustomWidget = AInterfaceWidgetFactory::createEditWidget(Rule, this);
        l->insertWidget(customWidgetPositionInLayout, CustomWidget);
        connect(CustomWidget, &AInterfaceRuleWidget::requestDraw, this, &AInterfaceRuleDialog::requestDraw);
        connect(CustomWidget, &AInterfaceRuleWidget::requestDrawLegend, this, &AInterfaceRuleDialog::requestDrawLegend);

        //surface
        if (!Rule->canHaveRoughSurface()) Rule->SurfaceSettings.Model = ASurfaceSettings::Polished;
        int iModel = 0;
        switch (Rule->SurfaceSettings.Model)
        {
        case ASurfaceSettings::Polished        : iModel = 0; break;
        case ASurfaceSettings::Glisur          : iModel = 1; break;
        case ASurfaceSettings::Unified         : iModel = 2; break;
        case ASurfaceSettings::CustomNormal    : iModel = 3; break;
        default:
            qWarning() << "Invalid surface model!";
            iModel = 0;
            break;
        }
        ui->cobSurfaceModel->setCurrentIndex(iModel);
        ui->swSurfaceModel->setCurrentIndex(iModel);
        ui->cobSurfaceModel->setEnabled(Rule->canHaveRoughSurface());
        ui->lePolishGlisur->setText(QString::number(Rule->SurfaceSettings.Polish));
        ui->leSigmaAlphaUnified->setText(QString::number(Rule->SurfaceSettings.SigmaAlpha));
        ui->cbCustNorm_CorrectForOrientation->setChecked(Rule->SurfaceSettings.OrientationProbabilityCorrection);
        updateCustomNormalButtons();
        ui->cbKillBackRefracted->setChecked(Rule->SurfaceSettings.KillPhotonsRefractedBackward);
    }
    else
    {
        ui->frNoOverride->setVisible(true);
        ui->pbTestOverride->setEnabled(false);
        ui->cobSurfaceModel->setCurrentIndex(0);
        ui->swSurfaceModel->setCurrentIndex(0);
        ui->cobSurfaceModel->setEnabled(false);
    }

    updateSymmetricVisuals();
}

AInterfaceRule * AInterfaceRuleDialog::findInOpended(const QString & ovType)
{
    for (AInterfaceRule * ov : TmpRules)
        if (ov->getType() == ovType)
        {
            TmpRules.erase(ov);
            return ov;
        }

    return nullptr;
}

void AInterfaceRuleDialog::clearTmpRules()
{
    for (AInterfaceRule * ov : TmpRules) delete ov;
    TmpRules.clear();
}

void AInterfaceRuleDialog::on_pbAccept_clicked()
{
    if (Rule)
    {
        QString err = Rule->checkOverrideData();
        if (!err.isEmpty())
        {
            guitools::message(err, this);
            return;
        }
    }
    accept();
}

void AInterfaceRuleDialog::on_pbCancel_clicked()
{
    reject();
}

void AInterfaceRuleDialog::closeEvent(QCloseEvent *e)
{
    TesterWindow->hide();
    delete TesterWindow; TesterWindow = nullptr;

    QDialog::closeEvent(e);
    emit closed(true);
}

void AInterfaceRuleDialog::on_cobType_activated(int index)
{
    if (Rule) TmpRules.insert(Rule);
    Rule = nullptr;

    if (index != 0)
    {
        QString selectedType = ui->cobType->currentText();
        Rule = findInOpended(selectedType);
        if (!Rule) Rule = AInterfaceRule::interfaceRuleFactory(ui->cobType->currentText(), MatFrom, MatTo);
    }

    updateGui();
}

void AInterfaceRuleDialog::on_pbTestOverride_clicked()
{
    if (Rule && Rule->SurfaceSettings.isNotPolished())
    {
        if (!Rule->Symmetric && !Rule->SurfaceSettings.KillPhotonsRefractedBackward)
        {
            bool ok = guitools::confirm("Rule is not symmetric and \"killing\" of back-transmitted photons is not activated.\n"
                              "The test will still assume symmetric interface for handling such photons.\n"
                              "Continue?", this);
            if (!ok) return;
        }
    }

    TesterWindow->show();
    TesterWindow->updateGUI();
    TesterWindow->showGeometry();

    TesterWindow->restoreGeomStatus();
    if (TesterWindow->x() == 0 && TesterWindow->y() == 0) TesterWindow->move(x(), y());

    connect(TesterWindow, &AInterfaceRuleTester::closed, this, &AInterfaceRuleDialog::setEnabled);
    setEnabled(false);
    TesterWindow->setEnabled(true);
}

void AInterfaceRuleDialog::on_cobSurfaceModel_currentIndexChanged(int index)
{
    ui->swSurfaceModel->setCurrentIndex(index);
    ui->swSurfaceModel->setVisible(index != 0);
    ui->cbKillBackRefracted->setVisible(index != 0);
}

void AInterfaceRuleDialog::on_cobSurfaceModel_activated(int index)
{
    if (Rule)
    {
        switch (index)
        {
        case 0 : Rule->SurfaceSettings.Model = ASurfaceSettings::Polished; break;
        case 1 : Rule->SurfaceSettings.Model = ASurfaceSettings::Glisur; break;
        case 2 : Rule->SurfaceSettings.Model = ASurfaceSettings::Unified; break;
        case 3 : Rule->SurfaceSettings.Model = ASurfaceSettings::CustomNormal; break;
        default:
            qWarning() << "Error in selecting surface model!";
            Rule->SurfaceSettings.Model = ASurfaceSettings::Polished;
            break;
        }
    }
}

void AInterfaceRuleDialog::on_lePolishGlisur_editingFinished()
{
    bool ok;
    const double polish = ui->lePolishGlisur->text().toDouble(&ok);
    if (!ok || polish < 0 || polish > 1.0)
    {
        guitools::message("Polish should be a number in the range from 0 to 1", this);
        ui->lePolishGlisur->setText(QString::number(Rule->SurfaceSettings.Polish));
    }
    Rule->SurfaceSettings.Polish = polish;
}

void AInterfaceRuleDialog::on_leSigmaAlphaUnified_editingFinished()
{
    bool ok;
    const double sa = ui->leSigmaAlphaUnified->text().toDouble(&ok);
    if (!ok || sa < 0)
    {
        guitools::message("Sigma Alpha cannot have negative value", this);
        ui->leSigmaAlphaUnified->setText(QString::number(Rule->SurfaceSettings.SigmaAlpha));
    }
    Rule->SurfaceSettings.SigmaAlpha = sa;
}

void AInterfaceRuleDialog::on_pbLoadCustomNormalDistribution_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load file with distribution of the angle between the microfacet's and global's normal", "Data files (*.txt *.dat); All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, Rule->SurfaceSettings.NormalDeviation, true);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    if (ui->cobCustomNormalDistributionUnits->currentIndex() == 0)
        for (auto & pair : Rule->SurfaceSettings.NormalDeviation)
            pair.first *= 3.1415926535/180.0;

    if (Rule->SurfaceSettings.NormalDeviation.back().first > 0.5*3.1415926535)
        guitools::message("Angle range is suspiciously large: did you happen to forget to select the proper angle units?", this);

    if (Rule->SurfaceSettings.NormalDeviation.back().first < 0.5*3.1415926535  * 3.1415926535/180.0)
        guitools::message("Angle range is suspiciously short: did you happen to forget to select the proper angle units?", this);

    updateCustomNormalButtons();
}

void AInterfaceRuleDialog::on_pbShowCustomNormalDistribution_clicked()
{
    if (Rule->SurfaceSettings.NormalDeviation.empty())
    {
        guitools::message("Distribution is not loaded", this);
        return;
    }

    TGraph * g = AGraphBuilder::graph(Rule->SurfaceSettings.NormalDeviation);
    QString xLabel = "Angle between normals, ";
    if (ui->cobCustomNormalDistributionUnits->currentIndex() == 0)
    {
        // degrees
        AGraphBuilder::shift(g, 180.0/3.1415926535, 0);  // from radians
        xLabel += "degrees";
    }
    else
    {
        // radians
        xLabel += "radians";
    }
    AGraphBuilder::configure(g, "", xLabel, "", 2, 20, 0.5,  2, 1, 1);

    emit requestDraw(g, "APL", true, true);
}

void AInterfaceRuleDialog::on_pbRemoveCustomNormalDistribution_clicked()
{
    Rule->SurfaceSettings.NormalDeviation.clear();
    updateCustomNormalButtons();
}

void AInterfaceRuleDialog::updateCustomNormalButtons()
{
    if (!Rule) return;
    bool bHaveData = (Rule->SurfaceSettings.NormalDeviation.size() > 1);

    ui->pbShowCustomNormalDistribution->setEnabled(bHaveData);
    ui->pbRemoveCustomNormalDistribution->setEnabled(bHaveData);
}

void AInterfaceRuleDialog::on_pbShowCustomNormalDistribution_customContextMenuRequested(const QPoint &)
{
    if (Rule->SurfaceSettings.NormalDeviation.empty())
    {
        guitools::message("Distribution is not loaded", this);
        return;
    }

    QString err = Rule->SurfaceSettings.checkRuntimeData();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    //emit requestDraw(LocalRule->SurfaceSettings.NormalDistributionHist, "hist", false, true);

    TH1D * h = new TH1D("", "", 100,0,0);
    for (size_t i = 0; i < 1000000; i++)
    {
        double alpha = Rule->SurfaceSettings.NormalDistributionHist->GetRandom();
        h->Fill(alpha);
    }
    emit requestDraw(h, "hist", true, true);
}

void AInterfaceRuleDialog::on_pbInfo_clicked()
{
    QString txt;
    if (!Rule)
        txt = "The interface rule is not defined:\nUsing \"normal\" physics model (Fresnel + Snell) for this interface.\n\n"
              "The optical properties of this interface still can be tested:\n"
              "Select 'Simplistic' rule and keep all settings on default:\n"
              "all coefficents set to 0, and 'Polished' surface model.";
    else
    {
        txt = Rule->getFullDescription();
        if (txt.isEmpty()) txt = "Description is not provided";
    }
    guitools::message1(txt, "Info for the selected interface rule", this);
}

void AInterfaceRuleDialog::on_cobType_currentIndexChanged(int index)
{
    ui->frSurfaceModel->setVisible(index != 0);
}

void AInterfaceRuleDialog::on_cbCustNorm_CorrectForOrientation_clicked(bool checked)
{
    Rule->SurfaceSettings.OrientationProbabilityCorrection = checked;
}

void AInterfaceRuleDialog::on_cbKillBackRefracted_clicked(bool checked)
{
    Rule->SurfaceSettings.KillPhotonsRefractedBackward = checked;
}

void AInterfaceRuleDialog::on_cbSymmetric_clicked(bool checked)
{
    if (Rule) Rule->Symmetric = checked;
    updateSymmetricVisuals();
}

void AInterfaceRuleDialog::updateSymmetricVisuals()
{
    ui->cbSymmetric->setVisible(MatFrom != MatTo); // has no effect anyway

    bool symmetric = ui->cbSymmetric->isChecked(); // start from this in case there is no rule, then it remembers the settings
    if (Rule) symmetric = Rule->Symmetric;

    QString txt = "-->";
    if (symmetric) txt = "<-->";
    ui->labArrow->setText(txt);
    ui->cbSymmetric->setChecked(symmetric);
}
