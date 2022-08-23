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

    updateGui();

    TesterWindow = new AInterfaceRuleTester(&LocalRule,  matFrom, matTo, this);
    connect(TesterWindow, &AInterfaceRuleTester::requestClearGeometryViewer, this, &AInterfaceRuleDialog::requestClearGeometryViewer);
    connect(TesterWindow, &AInterfaceRuleTester::requestDraw,                this, &AInterfaceRuleDialog::requestDraw);
    connect(TesterWindow, &AInterfaceRuleTester::requestDrawLegend,          this, &AInterfaceRuleDialog::requestDrawLegend);
    connect(TesterWindow, &AInterfaceRuleTester::requestShowTracks,          this, &AInterfaceRuleDialog::requestShowTracks);

//    TesterWindow->readFromJson(MW->OvTesterSettings);  !!!***
}

AInterfaceRuleDialog::~AInterfaceRuleDialog()
{
    qDebug() << "Destr for AInterfaceRuleDialog";
    //TesterWindow is saved and deleted on CloseEvent
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
    if (customWidget)
    {
        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        l->removeWidget(customWidget);
        delete customWidget; customWidget = nullptr;
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
        customWidget = AInterfaceWidgetFactory::createEditWidget(LocalRule, this, nullptr); // !!!***
        l->insertWidget(customWidgetPositionInLayout, customWidget);

        //surface
        if (!LocalRule->canHaveRoughSurface()) LocalRule->SurfaceSettings.Model = ASurfaceSettings::Polished;
        int iModel = 0;
        switch (LocalRule->SurfaceSettings.Model)
        {
        case ASurfaceSettings::Polished        : iModel = 0; break;
        case ASurfaceSettings::GaussSimplistic : iModel = 1; break;
        default:
            qWarning() << "Invalid surface model!";
            iModel = 0;
            break;
        }
        ui->cobSurfaceModel->setCurrentIndex(iModel);
        ui->swSurfaceModel->setCurrentIndex(iModel);
        ui->cobSurfaceModel->setEnabled(LocalRule->canHaveRoughSurface());
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
//    TesterWindow->writeToJson(MW->OvTesterSettings);  !!!***
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
        case 1 : LocalRule->SurfaceSettings.Model = ASurfaceSettings::GaussSimplistic; break;
        default:
            qWarning() << "Error in selecting surface model!";
            LocalRule->SurfaceSettings.Model = ASurfaceSettings::Polished;
            break;
        }
    }
}

