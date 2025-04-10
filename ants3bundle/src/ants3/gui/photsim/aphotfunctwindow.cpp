#include "aphotfunctwindow.h"
#include "ui_aphotfunctwindow.h"
#include "aphotonfunctionalhub.h"
#include "aphotonfunctionalmodel.h"
#include "afunctionalmodelwidget.h"
#include "ageometryhub.h"
#include "guitools.h"

#include <QDebug>

//#include "TObject.h"

APhotFunctWindow::APhotFunctWindow(QWidget * parent) :
    AGuiWindow("PhotFun", parent),
    PhFunHub(APhotonFunctionalHub::getInstance()),
    GeoHub(AGeometryHub::getConstInstance()),
    ui(new Ui::APhotFunctWindow)
{
    ui->setupUi(this);

    ui->tabwConnections->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabwConnections->setSelectionBehavior(QAbstractItemView::SelectRows);
    //ui->tabwConnections->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ui->tabwConnections->horizontalHeader()->setStretchLastSection(true);
    //setSectionResizeMode(QHeaderView.Stretch)
    connect(ui->tabwConnections->horizontalHeader(), &QHeaderView::sectionClicked, this, &APhotFunctWindow::onHeaderClicked);

    APFM_Dummy dm;
    LastWidget = new AFunctionalModelWidget_Dummy(&dm, this);
    QVBoxLayout * lay = dynamic_cast<QVBoxLayout*>(ui->frConnectionDelegate->layout());
    if (lay) lay->insertWidget(2, LastWidget);

    RedCircle = guitools::createColorCirclePixmap({15,15}, Qt::red);

    updateGui();
    //onModelChanged();

    setModifiedStatus(false);

    ASortableTableWidgetItem item;
    DefaultBrush = item.foreground();
}

APhotFunctWindow::~APhotFunctWindow()
{
    delete ui;
}

void APhotFunctWindow::fillCell(int iRow, int iColumn, const QString & txt, bool markNotValid, bool bold)
{
    QTableWidgetItem * item = ui->tabwConnections->item(iRow, iColumn);
    if (!item)
    {
        item = new ASortableTableWidgetItem();
        ui->tabwConnections->setItem(iRow, iColumn, item);
    }

    item->setForeground(markNotValid ? Qt::red : DefaultBrush);
    item->setTextAlignment(Qt::AlignCenter);
    QFont font = item->font(); font.setBold(bold); item->setFont(font);
    item->setText(txt);
}

#include "ageoobject.h"
void APhotFunctWindow::updateGui()
{
    ui->tabwConnections->clearContents();

    const size_t numDefault = GeoHub.PhotonFunctionals.size();
    ui->tabwConnections->setRowCount(numDefault + PhFunHub.OverritenRecords.size());

    int iRow = 0;

    // filling default
    for (size_t iDR = 0; iDR < numDefault; iDR++)
    {
        APhotonFunctionalModel * Model = std::get<0>(GeoHub.PhotonFunctionals[iDR])->getDefaultPhotonFunctionalModel();
        if (!Model)
        {
            qWarning() << "Something went wrong: model is nullptr";
            continue;
        }
        bool isLink = Model->isLink();

        fillCell(iRow, 0, QString::number(iDR),           isLink, false);
        fillCell(iRow, 1, (isLink ? "?" : "-"),           isLink, false);
        fillCell(iRow, 2, Model->getType(),               isLink, false);
        fillCell(iRow, 3, Model->printSettingsToString(), isLink, false);

        iRow++;
    }

    for (const APhotonFunctionalRecord & rec : PhFunHub.OverritenRecords)
    {
        int index = rec.Index;
        APhotonFunctionalModel * Model = rec.Model;
        bool isLink = Model->isLink();
        QString error;
        bool highlight = !PhFunHub.isValidRecord(rec, error);

        int thisRow;
        if (index >= numDefault)
        {
            thisRow = iRow;
            iRow++;
        }
        else thisRow = index;

        fillCell(thisRow, 0, QString::number(rec.Index),                     highlight, true);
        fillCell(thisRow, 1, (isLink ? QString::number(rec.LinkedTo) : "-"), highlight, true);
        fillCell(thisRow, 2, rec.Model->getType()+ " (*)",                           highlight, true);
        fillCell(thisRow, 3, rec.Model->printSettingsToString(),             highlight, true);
    }

    ui->tabwConnections->setRowCount(iRow);

    ui->tabwConnections->sortByColumn(SortByColumnIndex, (AscendingSortOrder ? Qt::AscendingOrder : Qt::DescendingOrder));

    ui->frConnectionDelegate->setEnabled(false);
    delete LocalModel; LocalModel = new APFM_Dummy();
    onModelChanged();
}

void APhotFunctWindow::onHeaderClicked(int index)
{
    if (SortByColumnIndex == index) AscendingSortOrder = !AscendingSortOrder;
    else SortByColumnIndex = index;

    updateGui();
}

void APhotFunctWindow::onModelChanged()
{
    QString type;
    if (LocalModel) type = LocalModel->getType();
    ui->leModelTypeName->setText(type);

    QVBoxLayout * lay = dynamic_cast<QVBoxLayout*>(ui->frConnectionDelegate->layout());
    if (lay)
    {
        lay->removeWidget(LastWidget);
        delete LastWidget; LastWidget = nullptr;

        LastWidget = AFunctionalModelWidget::factory(LocalModel, this);
        connect(LastWidget, &AFunctionalModelWidget::modified, this, [this](){setModifiedStatus(true);});
        connect(LastWidget, &AFunctionalModelWidget::requestDraw, this, &APhotFunctWindow::requestDraw);
        lay->insertWidget(2, LastWidget);
    }

    bool bLink = LocalModel && LocalModel->isLink();
    ui->labLinked->setVisible(bLink);
    ui->sbTo->setVisible(bLink);
    ui->cbShowConnection->setEnabled(bLink);
}

void APhotFunctWindow::on_pbAddModify_clicked()
{
    if (!LocalModel || !LastWidget)
    {
        guitools::message("Select a model first!", this);
        return;
    }

    QString error = LastWidget->updateModel(LocalModel);
    if (!error.isEmpty())
    {
        guitools::message(error, this);
        return;
    }

    int from  = ui->sbFrom->value();
    int to    = ui->sbTo->value();
    if (!LocalModel->isLink()) to = from;

    QString err = PhFunHub.modifyOrAddRecord(from, to, APhotonFunctionalModel::clone(LocalModel));
    if (err.isEmpty())
    {
        updateGui();
        setModifiedStatus(false);
    }
    else guitools::message(err, this);
}

void APhotFunctWindow::on_pbResetToDefault_clicked()
{
    bool ok = guitools::confirm("Reset this record to default?", this);
    if (!ok) return;

    int index = ui->sbFrom->value();
    //int to = ui->sbTo->value();
    //if (!ui->sbTo->isVisible()) to = from;
    PhFunHub.removeRecord(index);
    updateGui();
    setModifiedStatus(false);
}

void APhotFunctWindow::on_tabwConnections_cellClicked(int row, int)
{
    int from = ui->tabwConnections->item(row, 0)->text().toInt();
    ui->sbFrom->setValue(from);

    LocalModel = APhotonFunctionalModel::clone( PhFunHub.findModel(from) );
    if (!LocalModel)
    {
        ui->frConnectionDelegate->setEnabled(false);
        return;
    }

    onModelChanged();

    int to;
    if (LocalModel && LocalModel->isLink())
    {
        to = ui->tabwConnections->item(row, 1)->text().toInt();
        ui->sbTo->setValue(to);
    }

    if (ui->cbShowConnection->isChecked())
    {
        if (LocalModel && LocalModel->isLink()) emit requestShowConnection(from, to);
        else emit requestShowConnection(-1, -1); // clear tracks
    }

    setModifiedStatus(false);
    ui->frConnectionDelegate->setEnabled(true);
}

/*
#include "atreedatabaseselectordialog.h"
void APhotFunctWindow::on_pbSelectModel_clicked()
{
    ATreeDatabaseSelectorDialog dialog("Select model", this);
    QString err = dialog.readData(AFunctionalModelWidget::getModelDatabase());
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    int res = dialog.exec();
    if (res == QDialog::Accepted)
    {
        APhotonFunctionalModel * model = APhotonFunctionalModel::factory(dialog.SelectedItem);
        if (model) LastModel = model;
        else guitools::message("Model selection resulted in unknown model name");
        onModelChanged();
        setModifiedStatus(true);
    }
}
*/

void APhotFunctWindow::on_actionShow_all_linked_pairs_triggered()
{
    emit requestShowAllConnections();
}

void APhotFunctWindow::on_cbShowConnection_clicked(bool checked)
{
    if (checked)
    {
        if (LocalModel && LocalModel->isLink()) emit requestShowConnection(ui->sbFrom->value(), ui->sbTo->value());
        else requestShowConnection(-1, -1); // to clear tracks
    }
}

void APhotFunctWindow::setModifiedStatus(bool flag)
{
    ui->pbAddModify->setIcon(flag ? RedCircle : QPixmap());
}

void APhotFunctWindow::on_sbTo_textChanged(const QString &)
{
    setModifiedStatus(true);
}
void APhotFunctWindow::on_sbFrom_textChanged(const QString &)
{
    setModifiedStatus(true);
}
void APhotFunctWindow::on_leModelTypeName_textChanged(const QString &)
{
    setModifiedStatus(true);
}

void APhotFunctWindow::on_actionReset_all_to_default_triggered()
{
    PhFunHub.clearAllRecords();
    updateGui();
}

void APhotFunctWindow::on_actionRemove_records_with_invalid_index_triggered()
{
    const size_t numDefault = GeoHub.PhotonFunctionals.size();

    auto it = PhFunHub.OverritenRecords.begin();
    while (it != PhFunHub.OverritenRecords.end())
    {
        if (it->Index >= numDefault) PhFunHub.OverritenRecords.erase(it);
        else ++it;
    }

    updateGui();
}

void APhotFunctWindow::on_actionCheck_all_records_triggered()
{
    QString error = PhFunHub.checkRecordsReadyForRun();
    QString txt = "No errors were detected";
    if (!error.isEmpty()) txt = "Error detected!\n" + error;
    guitools::message(txt, this);
}

void APhotFunctWindow::on_pbHelp_clicked()
{
    QString txt =
        "A functional model offers a possibility to directly modify optical photon properties such as direction, position, time and waveindex.\n"
        "A model can be attributed to any non-logical geometry volume:\n"
        "see 'Special role' control of the volume editor at the geometry design window or through 'geo' script.\n"
        "\n"
        "\n"
        "The model is triggered for each photon entering the corresponding volume, then the tracing continues normally.\n"
        "\n"
        "Currently the following models are implemented:\n"
        "Filter:\n\tgray of band pass filter;\n"
        "ThinLens:\n\tthin lense with the focal length which can depend on the photon wavelength;\n"
        "OpticalFiber:\n\tphoton 'teleporter', which transports photons from the entrance to the exit volume.\n"
        "\n"
        "At this window it is possible to override the default model (or modify the model parameters) for configurations in which the model "
        "is attributed to an array of volumes.\n"
        "Also, if an 'OpticalFiber' model is selected, the fiber enterance and exit volumes have to be connected here.";

    guitools::message1(txt, "Functional model help", this);
}

