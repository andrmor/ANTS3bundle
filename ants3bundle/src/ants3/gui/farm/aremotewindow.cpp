#include "aremotewindow.h"
#include "ui_aremotewindow.h"
#include "afarmhub.h"
#include "aserverdelegate.h"
#include "guitools.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QDebug>

ARemoteWindow::ARemoteWindow(QWidget * parent) :
    AGuiWindow("Farm", parent),
    FarmHub(AFarmHub::getInstance()),
    ui(new Ui::ARemoteWindow)
{
    ui->setupUi(this);

    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    this->setWindowFlags( windowFlags );

    ui->lwServers->setViewMode(QListView::ListMode);
    ui->lwServers->setSpacing(1);

    QIntValidator* intVal = new QIntValidator(this);
    intVal->setRange(1, 100000000);
    ui->leiTimeout->setValidator(intVal);

    updateGui();
}

ARemoteWindow::~ARemoteWindow()
{
    clear();
    delete ui;
}

void ARemoteWindow::updateGui()
{
    clear();

    for (AFarmNodeRecord * node : FarmHub.getNodes())
        addNewNodeDelegate(node);

    ui->cbUseLocal->setChecked(FarmHub.UseLocal);
    ui->sbLocalProcesses->setValue(FarmHub.LocalProcesses);
    ui->cbUseFarm ->setChecked(FarmHub.UseFarm);
    ui->leiTimeout->setText(QString::number(FarmHub.TimeoutMs));
}

void ARemoteWindow::onBusy(bool flag)
{
    ui->pbAdd->setDisabled(flag);
    ui->pbRemove->setDisabled(flag);
    ui->pbStatus->setDisabled(flag);
    ui->pbRateServers->setDisabled(flag);
    ui->lwServers->setDisabled(flag);
}

void ARemoteWindow::addNewNodeDelegate(AFarmNodeRecord * record)
{
    AServerDelegate* delegate = new AServerDelegate(record);
    Delegates.push_back(delegate);

    QListWidgetItem* item = new QListWidgetItem();
    ui->lwServers->addItem(item);    
    ui->lwServers->setItemWidget(item, delegate);
    item->setSizeHint(delegate->sizeHint());
    QObject::connect(delegate, &AServerDelegate::updateSizeHint, this, &ARemoteWindow::onUpdateSizeHint);
    ui->lwServers->updateGeometry();
}

void ARemoteWindow::on_pbAdd_clicked()
{
    FarmHub.addNewNode();
    addNewNodeDelegate(FarmHub.getNodes().back());
}

void ARemoteWindow::onUpdateSizeHint(AServerDelegate * del)
{
    for (size_t i=0; i<Delegates.size(); i++)
    {
        if (del == Delegates.at(i))
        {
            del->updateGeometry();
            if (i < (size_t)ui->lwServers->count())
                ui->lwServers->item(i)->setSizeHint(del->sizeHint());
            del->updateGeometry();
            return;
        }
    }
}

void ARemoteWindow::clear()
{
    for (AServerDelegate* d : Delegates) delete d;
    Delegates.clear();

    ui->lwServers->clear();
}

void ARemoteWindow::onGuiUpdate()
{
    for (AServerDelegate* d : Delegates)
        d->updateGui();
}

void ARemoteWindow::on_pbStatus_clicked()
{
    onBusy(true);
    qApp->processEvents();
    FarmHub.checkFarmStatus();
    onBusy(false);

    updateGui();

//    ui->pbRateServers->setEnabled(true);
}

void ARemoteWindow::on_pbRateServers_clicked()
{

}

void ARemoteWindow::on_pbRemove_clicked()
{
    int index = ui->lwServers->currentRow();
    if (index < 0) return;

    bool ok = guitools::confirm("Remove this farm node?", this);
    if (!ok) return;

    FarmHub.removeNode(index);

    delete Delegates.at(index);
    Delegates.erase(Delegates.begin() + index);
    delete ui->lwServers->takeItem(index);
}

void ARemoteWindow::on_cbUseLocal_clicked(bool checked)
{
    FarmHub.UseLocal = checked;
}

void ARemoteWindow::on_sbLocalProcesses_editingFinished()
{
    FarmHub.LocalProcesses = ui->sbLocalProcesses->value();
}

void ARemoteWindow::on_cbUseFarm_clicked(bool checked)
{
    FarmHub.UseFarm = checked;
}

void ARemoteWindow::on_leiTimeout_editingFinished()
{
    FarmHub.TimeoutMs = ui->leiTimeout->text().toDouble();
}
