#include "aaddtobasketdialog.h"
#include "ui_aaddtobasketdialog.h"

AAddToBasketDialog::AAddToBasketDialog(const QStringList & groups, QWidget * parent) :
    QDialog(parent), ui(new Ui::AAddToBasketDialog)
{
    ui->setupUi(this);

    setWindowTitle("Add to basket");

    ui->cobGroups->addItems(groups);
}

AAddToBasketDialog::~AAddToBasketDialog()
{
    delete ui;
}

QString AAddToBasketDialog::getPath() const
{
    return ui->cobGroups->currentText();
}

bool AAddToBasketDialog::isBottom() const
{
    return ui->cobBottomTop->currentIndex() == 0;
}

QString AAddToBasketDialog::getTitle() const
{
    return ui->leTitle->text();
}

void AAddToBasketDialog::on_pbAccept_clicked()
{
    accept();
}

void AAddToBasketDialog::on_pcCancel_clicked()
{
    reject();
}

