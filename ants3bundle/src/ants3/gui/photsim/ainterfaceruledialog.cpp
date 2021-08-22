#include "ainterfaceruledialog.h"
#include "ui_ainterfaceruledialog.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "ainterfacewidgetfactory.h"
#include "guitools.h"
//#include "aopticaloverridetester.h"

#include <QJsonObject>
#include <QVBoxLayout>
#include <QDebug>

AInterfaceRuleDialog::AInterfaceRuleDialog(int matFrom, int matTo, QWidget * parent) :
    QDialog(parent),
    MatHub(AMaterialHub::getInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    ui(new Ui::AInterfaceRuleDialog),
    matFrom(matFrom), matTo(matTo)
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

    AInterfaceRule * ov = RuleHub.MaterialRules[matFrom][matTo];
    if (ov)
    {
        ovLocal = AInterfaceRule::interfaceRuleFactory(ov->getType(), matFrom, matTo);
        QJsonObject json; ov->writeToJson(json); ovLocal->readFromJson(json);
    }

    updateGui();

//    TesterWindow = new AOpticalOverrideTester(&ovLocal, MW, matFrom, matTo, this);
//    TesterWindow->readFromJson(MW->OvTesterSettings);
}

AInterfaceRuleDialog::~AInterfaceRuleDialog()
{
    //TesterWindow is saved and deleted on CloseEvent
    delete ui;
}

void AInterfaceRuleDialog::updateGui()
{
    if (customWidget)
    {
        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        l->removeWidget(customWidget);
        delete customWidget; customWidget = nullptr;
    }

    if (ovLocal)
    {
        ui->frNoOverride->setVisible(false);
        ui->pbTestOverride->setVisible(true);

        QStringList avOv = AInterfaceRule::getAllInterfaceRuleTypes();
        int index = avOv.indexOf(ovLocal->getType()); //TODO -> if not found?
        ui->cobType->setCurrentIndex(index+1);

        QVBoxLayout* l = static_cast<QVBoxLayout*>(layout());
        //customWidget = ovLocal->getEditWidget(this, MW->GraphWindow);
        customWidget = AInterfaceWidgetFactory::createEditWidget(ovLocal, this, nullptr); // !!!***
        l->insertWidget(customWidgetPositionInLayout, customWidget);
    }
    else
    {
        ui->frNoOverride->setVisible(true);
        ui->pbTestOverride->setVisible(false);
    }
}

AInterfaceRule * AInterfaceRuleDialog::findInOpended(const QString &ovType)
{
    for (AInterfaceRule* ov : openedOVs)
        if (ov->getType() == ovType) return ov;
    return 0;
}

void AInterfaceRuleDialog::clearOpenedExcept(AInterfaceRule *keepOV)
{
    for (AInterfaceRule * ov : openedOVs)
        if (ov != keepOV) delete ov;
    openedOVs.clear();
}

void AInterfaceRuleDialog::on_pbAccept_clicked()
{
    if (ovLocal)
    {
        QString err = ovLocal->checkOverrideData();
        if (!err.isEmpty())
        {
            guitools::message(err, this);
            return;
        }
    }

    clearOpenedExcept(ovLocal);

    delete RuleHub.MaterialRules[matFrom][matTo]; RuleHub.MaterialRules[matFrom][matTo] = ovLocal;
    ovLocal = nullptr;
    accept();
}

void AInterfaceRuleDialog::on_pbCancel_clicked()
{
    reject();
}

void AInterfaceRuleDialog::closeEvent(QCloseEvent *e)
{
    clearOpenedExcept(ovLocal); //to avoid double-delete
    delete ovLocal; ovLocal = 0;

//    TesterWindow->writeToJson(MW->OvTesterSettings);
//    TesterWindow->hide();
//    delete TesterWindow; TesterWindow = nullptr;

    QDialog::closeEvent(e);
}

void AInterfaceRuleDialog::on_cobType_activated(int index)
{
    if (ovLocal) openedOVs << ovLocal;
    ovLocal = 0;

    if (index != 0)
    {
         QString selectedType = ui->cobType->currentText();
         ovLocal = findInOpended(selectedType);
         if (!ovLocal)
            ovLocal = AInterfaceRule::interfaceRuleFactory(ui->cobType->currentText(), matFrom, matTo);
    }
    updateGui();
}

void AInterfaceRuleDialog::on_pbTestOverride_clicked()
{
//    TesterWindow->show();
//    TesterWindow->updateGUI();
//    TesterWindow->showGeometry();
}
