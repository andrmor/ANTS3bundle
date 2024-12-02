#include "ainterfaceruledialog.h"
#include "ui_ainterfaceruledialog.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "ainterfacewidgetfactory.h"
#include "guitools.h"
#include "ainterfaceruletester.h"

#include <QJsonObject>
#include <QVBoxLayout>
#include <QDebug>

AInterfaceRuleDialog::AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent) :
    QDialog(parent),
    MatFrom(matFrom), MatTo(matTo),
    MatHub(AMaterialHub::getInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    ui(new Ui::AInterfaceRuleDialog)
{
    ui->setupUi(this);
    ui->pbInterceptorForEnter->setVisible(false);
    ui->pbInterceptorForEnter->setDefault(true);
    setWindowTitle("Photon tracing rules for material interface");

    QStringList matNames = MatHub.getListOfMaterialNames();
    ui->leMatFrom->setText(matNames.at(matFrom));
    ui->leMatTo->setText(matNames.at(matTo));
    ui->cobType->addItem("No special rule");
    QStringList avOv = AInterfaceRule::getAllInterfaceRuleTypes();
    ui->cobType->addItems(avOv);

    MatHub.updateRuntimeProperties();

    if (rule)
    {
        LocalRule = AInterfaceRule::interfaceRuleFactory(rule->getType(), matFrom, matTo);
        QJsonObject json; rule->writeToJson(json); LocalRule->readFromJson(json);
    }

    ui->swSurfaceModel->setVisible(false);

    updateCustomNormalButtons();

    updateGui();

    TesterWindow = new AInterfaceRuleTester(LocalRule,  matFrom, matTo, this);
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
    delete LocalRule;
}

AInterfaceRule * AInterfaceRuleDialog::getRule()
{
    AInterfaceRule * r = LocalRule;
    LocalRule = nullptr;
    return r;
}

void AInterfaceRuleDialog::updateGui()
{
    if (CustomWidget)
    {
        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        l->removeWidget(CustomWidget);
        delete CustomWidget; CustomWidget = nullptr;
    }

    if (LocalRule)
    {
        ui->frNoOverride->setVisible(false);
        ui->pbTestOverride->setVisible(true);

        QStringList avOv = AInterfaceRule::getAllInterfaceRuleTypes();
        int index = avOv.indexOf(LocalRule->getType()); //TODO -> if not found?
        ui->cobType->setCurrentIndex(index+1);

        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        //customWidget = ovLocal->getEditWidget(this, MW->GraphWindow);
        CustomWidget = AInterfaceWidgetFactory::createEditWidget(LocalRule, this);
        l->insertWidget(customWidgetPositionInLayout, CustomWidget);
        connect(CustomWidget, &AInterfaceRuleWidget::requestDraw, this, &AInterfaceRuleDialog::requestDraw);
        connect(CustomWidget, &AInterfaceRuleWidget::requestDrawLegend, this, &AInterfaceRuleDialog::requestDrawLegend);

        //surface
        if (!LocalRule->canHaveRoughSurface()) LocalRule->SurfaceSettings.Model = ASurfaceSettings::Polished;
        int iModel = 0;
        switch (LocalRule->SurfaceSettings.Model)
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
        ui->cobSurfaceModel->setEnabled(LocalRule->canHaveRoughSurface());
        ui->lePolishGlisur->setText(QString::number(LocalRule->SurfaceSettings.Polish));
        ui->leSigmaAlphaUnified->setText(QString::number(LocalRule->SurfaceSettings.SigmaAlpha));
    }
    else
    {
        ui->frNoOverride->setVisible(true);
        ui->pbTestOverride->setVisible(false);
        ui->cobSurfaceModel->setCurrentIndex(0);
        ui->swSurfaceModel->setCurrentIndex(0);
        ui->cobSurfaceModel->setEnabled(false);
    }
}

AInterfaceRule * AInterfaceRuleDialog::findInOpended(const QString & ovType)
{
    for (AInterfaceRule* ov : qAsConst(TmpRules))
        if (ov->getType() == ovType)
        {
            TmpRules.remove(ov);
            return ov;
        }

    return nullptr;
}

void AInterfaceRuleDialog::clearTmpRules()
{
    for (AInterfaceRule * ov : qAsConst(TmpRules)) delete ov;
    TmpRules.clear();
}

void AInterfaceRuleDialog::on_pbAccept_clicked()
{
    if (LocalRule)
    {
        QString err = LocalRule->checkOverrideData();
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
    if (LocalRule) TmpRules << LocalRule;
    LocalRule = nullptr;

    if (index != 0)
    {
         QString selectedType = ui->cobType->currentText();
         LocalRule = findInOpended(selectedType);
         if (!LocalRule)
            LocalRule = AInterfaceRule::interfaceRuleFactory(ui->cobType->currentText(), MatFrom, MatTo);
    }
    updateGui();
}

void AInterfaceRuleDialog::on_pbTestOverride_clicked()
{
    TesterWindow->show();
    TesterWindow->updateGUI();
    TesterWindow->showGeometry();

    TesterWindow->move(x(), y());

    connect(TesterWindow, &AInterfaceRuleTester::closed, this, &AInterfaceRuleDialog::setEnabled);
    setEnabled(false);
    TesterWindow->setEnabled(true);
}

void AInterfaceRuleDialog::on_cobSurfaceModel_currentIndexChanged(int index)
{
    ui->swSurfaceModel->setCurrentIndex(index);
    ui->swSurfaceModel->setVisible(index != 0);
}

void AInterfaceRuleDialog::on_cobSurfaceModel_activated(int index)
{
    if (LocalRule)
    {
        switch (index)
        {
        case 0 : LocalRule->SurfaceSettings.Model = ASurfaceSettings::Polished; break;
        case 1 : LocalRule->SurfaceSettings.Model = ASurfaceSettings::Glisur; break;
        case 2 : LocalRule->SurfaceSettings.Model = ASurfaceSettings::Unified; break;
        case 3 : LocalRule->SurfaceSettings.Model = ASurfaceSettings::CustomNormal; break;
        default:
            qWarning() << "Error in selecting surface model!";
            LocalRule->SurfaceSettings.Model = ASurfaceSettings::Polished;
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
        ui->lePolishGlisur->setText(QString::number(LocalRule->SurfaceSettings.Polish));
    }
    LocalRule->SurfaceSettings.Polish = polish;
}


void AInterfaceRuleDialog::on_leSigmaAlphaUnified_editingFinished()
{
    bool ok;
    const double sa = ui->leSigmaAlphaUnified->text().toDouble(&ok);
    if (!ok || sa < 0)
    {
        guitools::message("Sigma Alpha cannot have negative value", this);
        ui->leSigmaAlphaUnified->setText(QString::number(LocalRule->SurfaceSettings.SigmaAlpha));
    }
    LocalRule->SurfaceSettings.SigmaAlpha = sa;
}

#include "afiletools.h"
void AInterfaceRuleDialog::on_pbLoadCustomNormalDistribution_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load file with distribution of the angle between the microfacet's and global's normal", "Data files (*.txt *.dat); All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = ftools::loadPairs(fileName, LocalRule->SurfaceSettings.NormalDeviation, true);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    if (ui->cobCustomNormalDistributionUnits->currentIndex() == 0)
        for (auto & pair : LocalRule->SurfaceSettings.NormalDeviation)
            pair.first *= 3.1415926535/180.0;

    if (LocalRule->SurfaceSettings.NormalDeviation.back().first > 0.5*3.1415926535)
        guitools::message("Angle range is suspiciously large: did you happen to forget to select the proper angle units?", this);

    if (LocalRule->SurfaceSettings.NormalDeviation.back().first < 0.5*3.1415926535  * 3.1415926535/180.0)
        guitools::message("Angle range is suspiciously short: did you happen to forget to select the proper angle units?", this);

    updateCustomNormalButtons();
}

#include "agraphbuilder.h"
#include "TGraph.h"
void AInterfaceRuleDialog::on_pbShowCustomNormalDistribution_clicked()
{
    if (LocalRule->SurfaceSettings.NormalDeviation.empty())
    {
        guitools::message("Distribution is not loaded", this);
        return;
    }

    TGraph * g = AGraphBuilder::graph(LocalRule->SurfaceSettings.NormalDeviation);
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
    LocalRule->SurfaceSettings.NormalDeviation.clear();
    updateCustomNormalButtons();
}

void AInterfaceRuleDialog::updateCustomNormalButtons()
{
    if (!LocalRule) return;
    bool bHaveData = (LocalRule->SurfaceSettings.NormalDeviation.size() > 1);

    ui->pbShowCustomNormalDistribution->setEnabled(bHaveData);
    ui->pbRemoveCustomNormalDistribution->setEnabled(bHaveData);
}

#include "TH1D.h"
void AInterfaceRuleDialog::on_pbShowCustomNormalDistribution_customContextMenuRequested(const QPoint &)
{
    if (LocalRule->SurfaceSettings.NormalDeviation.empty())
    {
        guitools::message("Distribution is not loaded", this);
        return;
    }

    QString err = LocalRule->SurfaceSettings.checkRuntimeData();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    //emit requestDraw(LocalRule->SurfaceSettings.NormalDistributionHist, "hist", false, true);

    TH1D * h = new TH1D("", "", 100,0,0);
    for (size_t i = 0; i < 1000000; i++)
    {
        double alpha = LocalRule->SurfaceSettings.NormalDistributionHist->GetRandom();
        h->Fill(alpha);
    }
    emit requestDraw(h, "hist", true, true);
}

void AInterfaceRuleDialog::on_pbInfo_clicked()
{
    QString txt;
    if (!LocalRule)
        txt = "The interface rule is not defined:\nUsing \"normal\" physics model (Fresnel + Snell) for this interface";
    else
    {
        txt = LocalRule->getFullDescription();
        if (txt.isEmpty()) txt = "Description is not provided";
    }
    guitools::message1(txt, "Info for the selected interface rule", this);
}

void AInterfaceRuleDialog::on_cobType_currentIndexChanged(int index)
{
    ui->frSurfaceModel->setVisible(index != 0);
}
