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

    connect(&MatHub, &AMaterialHub::materialsChanged, this, &AInterfaceRuleWin::updateGui);
    connect(&RuleHub, &AInterfaceRuleHub::rulesLoaded, this, &AInterfaceRuleWin::updateGui);

    connect(ui->tabwMat,     &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onMatCellDoubleClicked);
    connect(ui->tabwVolumes, &QTableWidget::itemDoubleClicked, this, &AInterfaceRuleWin::onVolCellDoubleClicked);

    updateGui();
}

AInterfaceRuleWin::~AInterfaceRuleWin()
{
    delete ui;
}

void AInterfaceRuleWin::updateGui()
{
    NumMatRules = 0;
    updateMatGui();
    updateVolGui();

    ui->labNumMatMat->setText( QString::number(NumMatRules) );
    ui->labNumVolVol->setText( QString::number(RuleHub.VolumeRules.size()) );
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
                NumMatRules++;
            }
            it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            ui->tabwMat->setItem(ifrom, ito, it);
        } 
}

void AInterfaceRuleWin::updateVolGui()
{
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

#include "abasicinterfacerule.h"
void AInterfaceRuleWin::on_pbAddNewVolumeRule_clicked()
{
    RuleHub.setVolumeRule("NameFrom", "NameTo", new ABasicInterfaceRule(0,0));
    updateGui();
}

void AInterfaceRuleWin::onVolCellDoubleClicked()
{
    int iRow = ui->tabwMat->currentRow();
    int iCol = ui->tabwMat->currentColumn();

    if (iCol == 2)
    {
//        AInterfaceRuleDialog * d = new AInterfaceRuleDialog(0, 0, this);
//        d->setAttribute(Qt::WA_DeleteOnClose);
//        d->setWindowModality(Qt::WindowModal);
//        QObject::connect(d, &AInterfaceRuleDialog::accepted, this, &AInterfaceRuleWin::onMatDialogAccepted);
//        d->show();
    }
}

