#include "ainterfacerulewin.h"
#include "ui_ainterfacerulewin.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "ainterfaceruledialog.h"

#include <QDebug>

AInterfaceRuleWin::AInterfaceRuleWin(QWidget *parent) :
    AGuiWindow("Rule", parent),
    MatHub(AMaterialHub::getConstInstance()),
    RuleHub(AInterfaceRuleHub::getInstance()),
    ui(new Ui::AInterfaceRuleWin)
{
    ui->setupUi(this);

    connect(&MatHub, &AMaterialHub::materialsChanged, this, &AInterfaceRuleWin::updateGui);
    connect(&RuleHub, &AInterfaceRuleHub::rulesLoaded, this, &AInterfaceRuleWin::updateGui);

    connect(ui->tabwMat,     &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onMatCellDoubleClicked);
    connect(ui->tabwVolumes, &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onVolCellDoubleClicked);
    connect(ui->tabwVolumes, &QTableWidget::itemChanged, this, &AInterfaceRuleWin::onVolCellChanged);

    updateGui();
}

AInterfaceRuleWin::~AInterfaceRuleWin()
{
    delete ui;
}

void AInterfaceRuleWin::updateGui()
{
    updateMatGui();
    updateVolGui();
}

void AInterfaceRuleWin::updateMatGui()
{
    ui->tabwMat->clear();

    const int numMat = MatHub.countMaterials();
    ui->tabwMat->setColumnCount(numMat);
    ui->tabwMat->setRowCount(numMat);

    ui->tabwMat->setVerticalHeaderLabels(MatHub.getListOfMaterialNames());
    ui->tabwMat->setHorizontalHeaderLabels(MatHub.getListOfMaterialNames());

    NumMatRules = 0;
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
                NumMatRules++;
            }
            it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            ui->tabwMat->setItem(ifrom, ito, it);
        }

    ui->labNumMatMat->setText( QString::number(NumMatRules) );
}

void AInterfaceRuleWin::updateVolGui()
{
    BulkUpdate = true; // -->

    ui->tabwVolumes->clear();
    ui->tabwVolumes->setColumnCount(3);
    ui->tabwVolumes->setRowCount(RuleHub.VolumeRules.size());

    ui->tabwVolumes->setHorizontalHeaderLabels({"Volume from", "Volume to", "Interface rule"});

    int iRow = 0;
    for (auto const & r : RuleHub.VolumeRules)
    {
        TString from = r.first.first;  ui->tabwVolumes->setItem(iRow, 0, new QTableWidgetItem(from.Data()));
        TString to   = r.first.second; ui->tabwVolumes->setItem(iRow, 1, new QTableWidgetItem(to  .Data()));

        AInterfaceRule * ov = r.second;
        QString text = ov->getAbbreviation();
        QTableWidgetItem * it = new QTableWidgetItem(text);
        it->setTextAlignment(Qt::AlignCenter);
        //it->setBackground(QBrush(Qt::lightGray));
        it->setToolTip(ov->getLongReportLine());
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        ui->tabwVolumes->setItem(iRow, 2, it);
        iRow++;
    }

    BulkUpdate = false; // <--

    ui->labNumVolVol->setText( QString::number(RuleHub.VolumeRules.size()) );
}

void AInterfaceRuleWin::onMatCellDoubleClicked()
{
    int iFrom = ui->tabwMat->currentRow();
    int iTo   = ui->tabwMat->currentColumn();

    AInterfaceRuleDialog * d = new AInterfaceRuleDialog(RuleHub.getMaterialRuleFast(iFrom, iTo), iFrom, iTo, this);
    //d->setAttribute(Qt::WA_DeleteOnClose);
    d->setWindowModality(Qt::WindowModal);
    int res = d->exec();
    if (res == QDialog::Accepted) RuleHub.MaterialRules[iFrom][iTo] = d->getRule();
    updateMatGui();
    delete d;
}

void AInterfaceRuleWin::onVolCellDoubleClicked()
{
    int iCol = ui->tabwVolumes->currentColumn();
    if (iCol != 2) return;
    int iRow = ui->tabwVolumes->currentRow();

    TString From(ui->tabwVolumes->item(iRow, 0)->text().toLatin1().data());
    TString To  (ui->tabwVolumes->item(iRow, 1)->text().toLatin1().data());

    AInterfaceRuleDialog * d = new AInterfaceRuleDialog(RuleHub.getVolumeRule(From, To), 0, 0, this);
    d->setWindowModality(Qt::WindowModal);
    int res = d->exec();
    if (res == QDialog::Accepted)
    {
        AInterfaceRule * newRule = d->getRule();
        if (newRule) RuleHub.setVolumeRule(From, To, newRule);
        else         RuleHub.removeVolumeRule(From, To);
    }
    delete d;

    updateVolGui();
}

void AInterfaceRuleWin::onVolCellChanged()
{
    if (BulkUpdate) return;

    int iCol = ui->tabwVolumes->currentColumn();
    if (iCol > 1) return;
    int iRow = ui->tabwVolumes->currentRow();

    TString OldFrom;
    TString OldTo;
    int iCounter = 0;
    for (auto const & r : RuleHub.VolumeRules)
    {
        OldFrom = r.first.first;
        OldTo   = r.first.second;
        if (iCounter == iRow) break;
        iCounter++;
    }

    if (iCol == 0)
    {
        const TString NewFrom(ui->tabwVolumes->item(iRow, 0)->text().toLatin1().data());
        if (NewFrom == OldFrom) return;
        RuleHub.moveVolumeRule(OldFrom, OldTo, NewFrom, OldTo);
    }
    else
    {
        const TString NewTo(ui->tabwVolumes->item(iRow, 1)->text().toLatin1().data());
        if (NewTo == OldTo) return;
        RuleHub.moveVolumeRule(OldFrom, OldTo, OldFrom, NewTo);
    }

    updateVolGui();
}

#include "abasicinterfacerule.h"
void AInterfaceRuleWin::on_pbAddNewVolumeRule_clicked()
{
    RuleHub.setVolumeRule("NameFrom", "NameTo", new ABasicInterfaceRule(0,0));
    updateVolGui();
}
