#include "aphotontunnelwindow.h"
#include "ui_aphotontunnelwindow.h"
#include "aphotonfunctionalhub.h"
#include "aphotonfunctionalmodel.h"
#include "ageometryhub.h"
#include "guitools.h"

APhotonTunnelWindow::APhotonTunnelWindow(const QString & idStr, QWidget * parent) :
    AGuiWindow(idStr, parent),
    ui(new Ui::APhotonTunnelWindow),
    PhFunHub(APhotonFunctionalHub::getInstance()),
    GeoHub(AGeometryHub::getConstInstance())
{
    ui->setupUi(this);

    ui->tabwConnections->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabwConnections->setSelectionBehavior(QAbstractItemView::SelectRows);
    //ui->tabwConnections->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ui->tabwConnections->horizontalHeader()->setStretchLastSection(true);
    //setSectionResizeMode(QHeaderView.Stretch)
    connect(ui->tabwConnections->horizontalHeader(), &QHeaderView::sectionClicked, this, &APhotonTunnelWindow::onHeaderClicked);

    updateGui();
    onModelChanged();
}

APhotonTunnelWindow::~APhotonTunnelWindow()
{
    delete ui;
}

void APhotonTunnelWindow::fillCell(int iRow, int iColumn, const QString & txt, bool markNotValid)
{
    QTableWidgetItem * item = ui->tabwConnections->item(iRow, iColumn);
    if (!item)
    {
        item = new ASortableTableWidgetItem();
        ui->tabwConnections->setItem(iRow, iColumn, item);
    }

    if (markNotValid) item->setForeground(Qt::red);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(txt);
}

void APhotonTunnelWindow::updateGui()
{
    ui->tabwConnections->clearContents();

    int numRecords = PhFunHub.FunctionalRecords.size();
    ui->tabwConnections->setRowCount(numRecords);

    int iRow = 0;
    for (const APhotonFunctionalRecord & rec : PhFunHub.FunctionalRecords)
    {
        APhotonFunctionalModel * Model = rec.Model;
        bool isLink = Model->isLink();
        QString error;
        bool highlight = !PhFunHub.isValidRecord(rec, error);

        fillCell(iRow, 0, QString::number(rec.Trigger), highlight);
        fillCell(iRow, 1, (isLink ? QString::number(rec.Target) : "-"), highlight);
        fillCell(iRow, 2, rec.Model->getType(), highlight);
        //qDebug() << rec.Model->printSettingsToString();
        fillCell(iRow, 3, rec.Model->printSettingsToString(), highlight);

        iRow++;
    }

    ui->tabwConnections->sortByColumn(SortByColumnIndex, (AscendingSortOrder ? Qt::AscendingOrder : Qt::DescendingOrder));
}

void APhotonTunnelWindow::onHeaderClicked(int index)
{
    if (SortByColumnIndex == index) AscendingSortOrder = !AscendingSortOrder;
    else SortByColumnIndex = index;

    updateGui();
}

void APhotonTunnelWindow::onModelChanged()
{
    QString type;
    if (LastModel) type = LastModel->getType();
    ui->leModelTypeName->setText(type);

    bool bLink = LastModel && LastModel->isLink();
    ui->labLinked->setVisible(bLink);
    ui->sbTo->setVisible(bLink);
    ui->cbShowConnection->setEnabled(bLink);
}

void APhotonTunnelWindow::on_pbAddModify_clicked()
{
    if (!LastModel)
    {
        guitools::message("Select a model first!", this);
        return;
    }

    int from  = ui->sbFrom->value();
    int to    = ui->sbTo->value();
    if (!LastModel->isLink()) to = from;

    QString err = PhFunHub.addOrModifyRecord(from, to, LastModel);
    if (err.isEmpty()) updateGui();
    else guitools::message(err, this);
}

void APhotonTunnelWindow::on_pbRemove_clicked()
{
    PhFunHub.removeRecord(ui->sbFrom->value(), ui->sbTo->value());
    updateGui();
}

void APhotonTunnelWindow::on_tabwConnections_cellClicked(int row, int)
{
    int from = ui->tabwConnections->item(row, 0)->text().toInt();
    ui->sbFrom->setValue(from);
    int to   = ui->tabwConnections->item(row, 1)->text().toInt();
    ui->sbTo->setValue(to);

    LastModel = PhFunHub.findModel(from, to);
    onModelChanged();

    if (ui->cbShowConnection->isChecked())
    {
        if (LastModel && LastModel->isLink()) emit requestShowConnection(from, to);
        else emit requestShowConnection(-1, -1); // clear tracks
    }
}

void APhotonTunnelWindow::on_pbSelectModel_clicked()
{
    //LastModel = new APFM_OpticalFiber();
    LastModel = new APFM_ThinLens();
    onModelChanged();
}

void APhotonTunnelWindow::on_actionShow_all_linked_pairs_triggered()
{
    emit requestShowAllConnections();
}

void APhotonTunnelWindow::on_cbShowConnection_clicked(bool checked)
{
    if (checked)
    {
        if (LastModel && LastModel->isLink()) emit requestShowConnection(ui->sbFrom->value(), ui->sbTo->value());
        else requestShowConnection(-1, -1); // to clear tracks
    }
}

void APhotonTunnelWindow::on_pbCheck_clicked()
{
    QString error = PhFunHub.checkRecordsReadyForRun();
    QString txt = "No errors were detected";
    if (!error.isEmpty()) txt = "Error detected!\n" + error;
    guitools::message(txt, this);
}

