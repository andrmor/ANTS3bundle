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

AInterfaceRuleDialog::AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent) :
    QDialog(parent),
    MatHub(AMaterialHub::getInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    MatFrom(matFrom), MatTo(matTo),
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

    updateGui();

//    TesterWindow = new AOpticalOverrideTester(&ovLocal, MW, matFrom, matTo, this);
//    TesterWindow->readFromJson(MW->OvTesterSettings);
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
    }
    else
    {
        ui->frNoOverride->setVisible(true);
        ui->pbTestOverride->setVisible(false);
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
//    TesterWindow->writeToJson(MW->OvTesterSettings);
//    TesterWindow->hide();
//    delete TesterWindow; TesterWindow = nullptr;

    QDialog::closeEvent(e);
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
//    TesterWindow->show();
//    TesterWindow->updateGUI();
//    TesterWindow->showGeometry();
}
