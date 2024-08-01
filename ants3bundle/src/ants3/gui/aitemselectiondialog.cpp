#include "aitemselectiondialog.h"
#include "ui_aitemselectiondialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QListWidgetItem>

AItemSelectionDialog::AItemSelectionDialog(const QString & databaseFileName, const QString & pathToItemFiles, const QString & title, QWidget * parent) :
    QDialog(parent), Path(pathToItemFiles),
    ui(new Ui::AItemSelectionDialog)
{
    ui->setupUi(this);

    setWindowTitle(title);

    Data = new AItemRecordDatabase();
    Data->readFromFile(databaseFileName);

    QObject::connect(ui->lwExamples, &QListWidget::currentRowChanged, this, &AItemSelectionDialog::onExampleSelected);
    QObject::connect(ui->lwExamples, &QListWidget::itemDoubleClicked, this, &AItemSelectionDialog::on_pbLoad_clicked);

    //showing tags - they will not be modified, only checked/unchecked
    BulkUpdate = true;
    for (int i = 0; i < Data->Tags.size(); i++)
    {
        QListWidgetItem * item = new QListWidgetItem(Data->Tags.at(i), ui->lwTags);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
    BulkUpdate = false;

    //connect(ui->lwTags, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onTagStateChanged(QListWidgetItem*)));
    connect(ui->lwTags, &QListWidget::itemChanged, this, &AItemSelectionDialog::onTagStateChanged);
    connect(ui->leFind, &QLineEdit::textChanged, this, &AItemSelectionDialog::onFindTextChanged);
    updateGui();

    QTimer::singleShot(100, this, &AItemSelectionDialog::clearSelection);
}

AItemSelectionDialog::~AItemSelectionDialog()
{
    delete Data;
    delete ui;
}

void AItemSelectionDialog::updateGui()
{
    QStringList SelectedTags;
    for (int i = 0; i < ui->lwTags->count(); i++)
        if (ui->lwTags->item(i)->checkState() == Qt::Checked) SelectedTags.append(ui->lwTags->item(i)->text());
    Data->select(SelectedTags);

    ui->lwExamples->clear();
    for (int i = 0; i < Data->size(); i++)
    {
        if (Data->Items[i].Selected)
            ui->lwExamples->addItem(Data->Items[i].Name);
    }
}

void AItemSelectionDialog::clearSelection()
{
    ui->lwExamples->clearSelection();
    ui->lwExamples->setCurrentRow(-1);
    ui->leSelected->clear();
    ui->teDescription->clear();
}

void AItemSelectionDialog::onExampleSelected(int row)
{
    if (row < 0)
    {
        clearSelection();
        return;
    }

    QString fn = ui->lwExamples->currentItem()->text();
    int index = Data->findIndexByName(fn);
    if (index < 0)
    {
        clearSelection();
        qWarning() << "Example not found!";
        return;
    }

    const AItemRecord & ex = Data->Items[index];
    FileNameSelected = ex.File;
    ui->leSelected->setText(ex.Name);
    ui->teDescription->setText(ex.Description);
}

void AItemSelectionDialog::on_pbLoad_clicked()
{
    QString fn = ui->leSelected->text();
    if (fn.isEmpty()) return;
    accept();
}

void AItemSelectionDialog::on_pbSelectAllTags_clicked()
{
    for (int i=0; i<ui->lwTags->count(); i++)
        ui->lwTags->item(i)->setCheckState(Qt::Checked);
}

void AItemSelectionDialog::on_pbUnselectAllTags_clicked()
{
    for (int i=0; i<ui->lwTags->count(); i++)
        ui->lwTags->item(i)->setCheckState(Qt::Unchecked);
}

void AItemSelectionDialog::onTagStateChanged(QListWidgetItem *)
{
    if (BulkUpdate) return;
    updateGui();
}

void AItemSelectionDialog::onFindTextChanged(const QString & text)
{
    const bool bEmpty = text.isEmpty();

    for (int i=0; i<ui->lwTags->count(); i++)
    {
        bool bVisible = bEmpty || ui->lwTags->item(i)->text().contains(text, Qt::CaseInsensitive);

        ui->lwTags->item(i)->setHidden( !bVisible );
        ui->lwTags->item(i)->setCheckState( bVisible ? Qt::Checked : Qt::Unchecked );
    }
}

void AItemSelectionDialog::on_lwTags_itemDoubleClicked(QListWidgetItem * item)
{
    for (int i=0; i<ui->lwTags->count(); i++)
        if (ui->lwTags->item(i) == item)
            ui->lwTags->item(i)->setCheckState(Qt::Checked);
        else
            ui->lwTags->item(i)->setCheckState(Qt::Unchecked);
}

// --------------------------
#include "ajsontools.h"
void AItemRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Name", Name);
    jstools::parseJson(json, "File", File);
    QJsonArray ar;
    jstools::parseJson(json, "Tags", ar);
    jstools::parseJson(json, "Description", Description);

    Tags.clear();
    for (int i = 0; i < ar.size(); i++)
        Tags << ar[i].toString();
}

// -----------------------
#include "ajsontools.h"
bool AItemRecordDatabase::readFromFile(const QString &fileName)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        qWarning() << "Failed to open database file:" << fileName;
        return false;
    }

    QJsonArray ar;
    jstools::parseJson(json, "Materials", ar);
    if (ar.empty())
    {
        qWarning() << "Database is empty after reading from file:" << fileName;
        return false;
    }

    for (int i = 0; i < ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();

        AItemRecord item;
        item.readFromJson(js);
        Items.push_back(item);
        for (const QString & tag : item.Tags)
            if (!Tags.contains(tag)) Tags.append(tag);
    }

    Tags.sort();
    return true;
}

void AItemRecordDatabase::select(const QStringList & tags)
{
    if (tags.isEmpty())
    {
        unselectAll();
        return;
    }

    for (auto & item : Items)
    {
        item.Selected = false;
        for (const auto & tag : tags)
            if (item.Tags.contains(tag))
            {
                item.Selected = true;
                break;
            }
    }
}

void AItemRecordDatabase::unselectAll()
{
    for (auto & r : Items) r.Selected = false;
}

int AItemRecordDatabase::size() const
{
    return Items.size();
}

int AItemRecordDatabase::findIndexByName(const QString & name) const
{
    for (size_t i = 0; i < Items.size(); i++)
    {
        if (name == Items[i].Name)
            return i;
    }
    return -1;
}
