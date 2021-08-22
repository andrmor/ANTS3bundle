#include "ainterfacerulewin.h"
#include "ui_ainterfacerulewin.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "ainterfaceruledialog.h"

AInterfaceRuleWin::AInterfaceRuleWin(QWidget *parent) :
    QMainWindow(parent),
    MatHub(AMaterialHub::getConstInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    ui(new Ui::AInterfaceRuleWin)
{
    ui->setupUi(this);

    connect(ui->tabwMat, &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onMatCellDoubleClicked);
    connect(&MatHub, &AMaterialHub::materialsChanged, this, &AInterfaceRuleWin::updateGui);
    connect(&RuleHub, &AInterfaceRuleHub::rulesLoaded, this, &AInterfaceRuleWin::updateGui);

    updateGui();
}

AInterfaceRuleWin::~AInterfaceRuleWin()
{
    delete ui;
}

void AInterfaceRuleWin::updateGui()
{
    updateMatGui();
}

void AInterfaceRuleWin::updateMatGui()
{
    const int numMat = MatHub.countMaterials();

    ui->tabwMat->setColumnCount(numMat);
    ui->tabwMat->setRowCount(numMat);

    ui->tabwMat->setVerticalHeaderLabels(MatHub.getListOfMaterialNames());
    ui->tabwMat->setHorizontalHeaderLabels(MatHub.getListOfMaterialNames());

    for (int ifrom = 0; ifrom < numMat; ifrom++)
        for (int ito = 0; ito < numMat; ito++)
        {
            AInterfaceRule * ov = RuleHub.MaterialRules[ifrom][ito];
            QString text;
            if (ov) text = ov->getAbbreviation();
            QTableWidgetItem * it = new QTableWidgetItem(text);
            it->setTextAlignment(Qt::AlignCenter);
            if (ov)
            {
                it->setBackground(QBrush(Qt::lightGray));
                it->setToolTip(ov->getLongReportLine());
            }
            it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            ui->tabwMat->setItem(ifrom, ito, it);
        }

}

void AInterfaceRuleWin::onMatCellDoubleClicked()
{
    int iFrom = ui->tabwMat->currentRow();
    int iTo   = ui->tabwMat->currentColumn();

    AInterfaceRuleDialog * d = new AInterfaceRuleDialog(iFrom, iTo, this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->setWindowModality(Qt::WindowModal);
    QObject::connect(d, &AInterfaceRuleDialog::accepted, this, &AInterfaceRuleWin::onMatDialogAccepted);
    d->show();
}

void AInterfaceRuleWin::onMatDialogAccepted()
{
    updateMatGui();
}
