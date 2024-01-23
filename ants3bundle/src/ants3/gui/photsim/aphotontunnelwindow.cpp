#include "aphotontunnelwindow.h"
#include "ui_aphotontunnelwindow.h"
#include "aphotontunnelhub.h"
#include "ageometryhub.h"
#include "guitools.h"

APhotonTunnelWindow::APhotonTunnelWindow(const QString & idStr, QWidget * parent) :
    AGuiWindow(idStr, parent),
    ui(new Ui::APhotonTunnelWindow),
    PhTunHub(APhotonTunnelHub::getInstance()),
    GeoHub(AGeometryHub::getConstInstance())
{
    ui->setupUi(this);

    ui->tabwConnections->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabwConnections->setSelectionBehavior(QAbstractItemView::SelectRows);

    updateGui();
}

APhotonTunnelWindow::~APhotonTunnelWindow()
{
    delete ui;
}

void APhotonTunnelWindow::fillCell(int iRow, int iColumn, const QString & txt)
{
    QTableWidgetItem * item = ui->tabwConnections->item(iRow, iColumn);
    if (!item)
    {
       item = new ASortableTableWidgetItem();
       ui->tabwConnections->setItem(iRow, iColumn, item);
    }
    item->setText(txt);
}

void APhotonTunnelWindow::updateGui()
{
    ui->tabwConnections->clearContents();

    int numRecords = PhTunHub.Connections.size();
    qDebug() << "Number of connections:" << numRecords;

    ui->tabwConnections->setRowCount(numRecords);

    int iRow = 0;
    for (const ATunnelRecord & rec : PhTunHub.Connections)
    {
        qDebug() << rec.From << rec.To << rec.ModelIndex << rec.Settings;

        fillCell(iRow, 0, QString::number(rec.From));
        fillCell(iRow, 1, QString::number(rec.To));
        fillCell(iRow, 2, QString::number(rec.ModelIndex));
        fillCell(iRow, 3, rec.Settings);
        fillCell(iRow, 4, ( PhTunHub.isValidConnection(rec, false) ? "Yes" : "") );

        iRow++;
    }

    ui->tabwConnections->sortByColumn( (ui->rbSortByFrom->isChecked() ? 0 : 1), Qt::AscendingOrder);

    updateInfoLabels();
}

void APhotonTunnelWindow::updateInfoLabels()
{
    // !!!*** check overlap with updateRuntimeProperties() of APhotonTunnelHub
    int numEntrances = GeoHub.PhotonTunnelsIn.size();
    int numExits = GeoHub.PhotonTunnelsOut.size();

    int numNotConnectedEntrances = 0;
    int numMultipleEntrances = 0;
    for (int i = 0; i < numEntrances; i++)
    {
        int seenTimes = 0;
        for (const ATunnelRecord & rec : PhTunHub.Connections)
            if (rec.From == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedEntrances++;
        if (seenTimes > 1) numMultipleEntrances++;
    }

    int numNotConnectedExits = 0;
    int numMultipleExits = 0;
    for (int i = 0; i < numExits; i++)
    {
        int seenTimes = 0;
        for (const ATunnelRecord & rec : PhTunHub.Connections)
            if (rec.To == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedExits++;
        if (seenTimes > 1) numMultipleExits++;
    }

    ui->labNumEntrances->setText(QString::number(numEntrances));
    ui->labNumExits->setText(QString::number(numExits));

    ui->labNumBadEntrances->setText(QString::number(numNotConnectedEntrances));
    ui->labNumBadExits->setText(QString::number(numNotConnectedExits));

    ui->labError->setVisible(numNotConnectedEntrances > 0 || numNotConnectedExits > 0);

    ui->labNumWithMultipleEntrances->setText(QString::number(numMultipleEntrances));
    if (numMultipleExits > 0) qCritical() << "Something is went wrong: currently it is impossible to have several exits for the same tunnel entrance!";
}

void APhotonTunnelWindow::on_rbSortByFrom_clicked(bool checked)
{
    ui->rbSortByTo->setChecked(!checked);
    updateGui();
}

void APhotonTunnelWindow::on_rbSortByTo_clicked(bool checked)
{
    ui->rbSortByFrom->setChecked(!checked);
    updateGui();
}

void APhotonTunnelWindow::on_pbAddModify_clicked()
{
    int from  = ui->sbFrom->value();
    int to    = ui->sbTo->value();
    int model = ui->sbModel->value();
    QString settings = ui->leSettings->text();

    QString err = PhTunHub.addOrModifyConnection(from, to, model, settings);
    if (err.isEmpty()) updateGui();
    else guitools::message(err, this);
}

void APhotonTunnelWindow::on_pbRemove_clicked()
{
    PhTunHub.removeConnection(ui->sbFrom->value(), ui->sbTo->value());
    updateGui();
}

void APhotonTunnelWindow::on_tabwConnections_cellClicked(int row, int)
{
    int from = ui->tabwConnections->item(row, 0)->text().toInt();
    ui->sbFrom->setValue(from);
    int to   = ui->tabwConnections->item(row, 1)->text().toInt();
    ui->sbTo->setValue(to);

    ui->sbModel->setValue(  ui->tabwConnections->item(row, 2)->text().toInt());
    ui->leSettings->setText(ui->tabwConnections->item(row, 3)->text());

    if (ui->cbShowConnection->isChecked()) emit requestShowConnection(from, to);
}

void APhotonTunnelWindow::on_pbShowAllConnections_clicked()
{
    emit requestShowAllConnections();
}

