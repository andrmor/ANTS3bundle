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

    connect(&RuleHub, &AInterfaceRuleHub::rulesLoaded, this, &AInterfaceRuleWin::updateGui);

    connect(ui->tabwMat,     &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onMatCellDoubleClicked);

    connect(ui->tabwVolumes, &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onVolCellDoubleClicked);
    connect(ui->tabwVolumes, &QTableWidget::itemChanged, this, &AInterfaceRuleWin::onVolCellChanged);
    connect(ui->tabwVolumes->horizontalHeader(), &QHeaderView::sectionClicked, this, &AInterfaceRuleWin::onVolColumnClicked);

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

    delete RuleDialog; RuleDialog = nullptr;
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
/*
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
        QTableWidgetItem * item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(ov->getLongReportLine());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        ui->tabwVolumes->setItem(iRow, 2, item);
        iRow++;
    }

    BulkUpdate = false; // <--

    ui->labNumVolVol->setText( QString::number(RuleHub.VolumeRules.size()) );
}
*/

void AInterfaceRuleWin::updateVolGui()
{
    BulkUpdate = true; // -->

    ui->tabwVolumes->clear();
    ui->tabwVolumes->setColumnCount(3);
    ui->tabwVolumes->setRowCount(RuleHub.VolumeRules.size());

    ui->tabwVolumes->setHorizontalHeaderLabels({"Volume from", "Volume to", "Interface rule"});

    typedef std::tuple< TString,TString,AInterfaceRule* > rec;  // from, to, rule*
    std::vector<rec> records;
    for (auto const & r : RuleHub.VolumeRules) records.push_back({r.first.first, r.first.second, r.second});

    std::sort(records.begin(), records.end(), [this](const rec & lhs, const rec & rhs)
    {
        if (sortByColumn == 0)
        {
            return std::get<0>(lhs) < std::get<0>(rhs);
        }
        else if (sortByColumn == 1)
        {
            return std::get<1>(lhs) < std::get<1>(rhs);
        }
        else
        {
            return std::get<2>(lhs)->getAbbreviation() < std::get<2>(rhs)->getAbbreviation();
        }
    });

    for (size_t iRow = 0; iRow < records.size(); iRow++)
    {
        const rec r = records[iRow];

        TString from = std::get<0>(r); ui->tabwVolumes->setItem(iRow, 0, new QTableWidgetItem(from.Data()));
        TString to   = std::get<1>(r); ui->tabwVolumes->setItem(iRow, 1, new QTableWidgetItem(to  .Data()));

        AInterfaceRule * ov = std::get<2>(r);
        QString text = ov->getAbbreviation();
        QTableWidgetItem * item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(ov->getLongReportLine());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        ui->tabwVolumes->setItem(iRow, 2, item);
    }

    BulkUpdate = false; // <--

    ui->labNumVolVol->setText( QString::number(RuleHub.VolumeRules.size()) );
}

void AInterfaceRuleWin::onMatCellDoubleClicked()
{
    int iFrom = ui->tabwMat->currentRow();
    int iTo   = ui->tabwMat->currentColumn();

    delete RuleDialog; RuleDialog = new AInterfaceRuleDialog(RuleHub.getMaterialRuleFast(iFrom, iTo), iFrom, iTo, this);
    configureInterfaceDialog();
    connect(RuleDialog, &AInterfaceRuleDialog::accepted, this, &AInterfaceRuleWin::OnRuleDialogAccepted_Mat);
    RuleDialog->show();
}

#include "guitools.h"
void AInterfaceRuleWin::OnRuleDialogAccepted_Mat()
{
    if (!RuleDialog) return;

    AInterfaceRule * rule = RuleDialog->getRule();
    RuleHub.setMaterialRule(RuleDialog->MatFrom, RuleDialog->MatTo, rule);

    if (RuleDialog->MatFrom != RuleDialog->MatTo)
    {
        if (RuleDialog->isSetSymmetric())
        {
            AInterfaceRule * ruleCopy = nullptr;
            if (rule)
            {
                // cloning the rule
                QJsonObject json;
                rule->writeToJson(json);
                ruleCopy = AInterfaceRule::interfaceRuleFactory(rule->getType(), RuleDialog->MatTo, RuleDialog->MatFrom); // reversed materials!
                bool ok = false;
                if (ruleCopy) ok = ruleCopy->readFromJson(json);
                if (!ok)
                {
                    guitools::message("Failed to clone the interface rule!", this);
                    return;
                }
            }
            RuleHub.setMaterialRule(RuleDialog->MatTo, RuleDialog->MatFrom, ruleCopy); // reversed materials!
        }
        else
        {
            // synchronize not-symmetric in case the reversed one has this flag on
            AInterfaceRule * reverseRule = RuleHub.getMaterialRuleFast(RuleDialog->MatTo, RuleDialog->MatFrom);
            if (reverseRule) reverseRule->Symmetric = false;
        }
    }

    updateMatGui();
    delete RuleDialog; RuleDialog = nullptr;
}

#include "ageometryhub.h"
#include "ageoobject.h"
void AInterfaceRuleWin::onVolCellDoubleClicked()
{
    int iCol = ui->tabwVolumes->currentColumn();
    if (iCol != 2) return;
    int iRow = ui->tabwVolumes->currentRow();

    LastFrom = ui->tabwVolumes->item(iRow, 0)->text().toLatin1().data();
    LastTo   = ui->tabwVolumes->item(iRow, 1)->text().toLatin1().data();

    AGeoObject * world = AGeometryHub::getConstInstance().World;
    AGeoObject * obFrom = world->findObjectByName(LastFrom.Data());
    int iFrom = 0;
    if (obFrom) iFrom = obFrom->Material;
    int iTo = 0;
    AGeoObject * obTo = world->findObjectByName(LastTo.Data());
    if (obTo) iTo = obTo->Material;

    delete RuleDialog; RuleDialog = new AInterfaceRuleDialog(RuleHub.getVolumeRule(LastFrom, LastTo), iFrom, iTo, this, LastFrom.Data(), LastTo.Data());
    if (obFrom && obTo) RuleDialog->VolumesExist = true;

    configureInterfaceDialog();
    itemDoubleClicked = ui->tabwVolumes->item(iRow, iCol);
    connect(RuleDialog, &AInterfaceRuleDialog::accepted, this, &AInterfaceRuleWin::OnRuleDialogAccepted_Vol);
    RuleDialog->show();
}

void AInterfaceRuleWin::OnRuleDialogAccepted_Vol()
{
    if (!RuleDialog) return;

    AInterfaceRule * newRule = RuleDialog->getRule();
    if (newRule)
    {
        RuleHub.setVolumeRule(LastFrom, LastTo, newRule);
        if (itemDoubleClicked) itemDoubleClicked->setText(newRule->getAbbreviation());
        else updateVolGui();
    }
    else
    {
        RuleHub.removeVolumeRule(LastFrom, LastTo);
        updateVolGui();
    }
}

#include "TObject.h"
void AInterfaceRuleWin::configureInterfaceDialog()
{
    //RuleDialog->setWindowModality(Qt::WindowModal);
    //RuleDialog->setWindowModality(Qt::NonModal);
    connect(RuleDialog, &AInterfaceRuleDialog::requestClearGeometryViewer, this, &AInterfaceRuleWin::requestClearGeometryViewer);
    connect(RuleDialog, &AInterfaceRuleDialog::requestDraw,                this, &AInterfaceRuleWin::requestDraw);
    connect(RuleDialog, &AInterfaceRuleDialog::requestDrawLegend,          this, &AInterfaceRuleWin::requestDrawLegend);
    connect(RuleDialog, &AInterfaceRuleDialog::requestShowTracks,          this, &AInterfaceRuleWin::requestShowTracks);
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

    //updateVolGui();
}

void AInterfaceRuleWin::onVolColumnClicked(int index)
{
    sortByColumn = index;
    updateVolGui();
}

#include "abasicinterfacerule.h"
void AInterfaceRuleWin::on_pbAddNewVolumeRule_clicked()
{
    const QString Fr = "_NameFrom_";
    const QString To = "_NameTo_";
    RuleHub.setVolumeRule(Fr.toLatin1().data(), To.toLatin1().data(), new ABasicInterfaceRule(0,0));
    updateVolGui();

    for (int iRow = 0; iRow < ui->tabwVolumes->rowCount(); iRow++)
    {
        if (Fr != ui->tabwVolumes->item(iRow, 0)->text()) continue;
        if (To != ui->tabwVolumes->item(iRow, 1)->text()) continue;
        ui->tabwVolumes->selectRow(iRow);
        break;
    }
}
