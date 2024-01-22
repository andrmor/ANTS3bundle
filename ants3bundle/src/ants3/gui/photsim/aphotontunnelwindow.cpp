#include "aphotontunnelwindow.h"
#include "ui_aphotontunnelwindow.h"
#include "aphotontunnelhub.h"

APhotonTunnelWindow::APhotonTunnelWindow(const QString & idStr, QWidget * parent) :
    AGuiWindow(idStr, parent),
    ui(new Ui::APhotonTunnelWindow),
    PhTunHub(APhotonTunnelHub::getInstance())
{
    ui->setupUi(this);

    ui->tabwConnections->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

        iRow++;
    }

    ui->tabwConnections->sortByColumn( (ui->rbSortByFrom->isChecked() ? 0 : 1), Qt::AscendingOrder);
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

#include "guitools.h"
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

