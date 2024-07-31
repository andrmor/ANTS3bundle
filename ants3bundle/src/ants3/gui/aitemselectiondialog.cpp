#include "aitemselectiondialog.h"
#include "ui_aitemselectiondialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QListWidgetItem>

AItemSelectionDialog::AItemSelectionDialog(const QString & databaseFileName, const QString & pathToItemFiles, const QString & title, QWidget * parent) :
    QWidget(parent), Path(pathToItemFiles),
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
    ui->leSelected->setText(ex.Name);
    ui->teDescription->setText(ex.Description);
}

void AItemSelectionDialog::on_pbLoad_clicked()
{
    QString fn = ui->leSelected->text();
    if (fn.isEmpty()) return;
    close();
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

bool AItemRecord::readFromRecord(const QString & text)
{
    ErrorString.clear();

    QStringList list = text.split('#');

    for (int i = 0; i < list.size(); i++)
    {
        QString & r = list[i];
        if (r.startsWith("name"))
        {
            QStringList fi = r.split('\n');
            if (fi.size() < 2)
            {
                ErrorString = "Bad format in name record";
                qWarning() << ErrorString;
                return false;
            }
            Name = fi[1];
        }
        else if (r.startsWith("file"))
        {
            QStringList fi = r.split('\n');
            if (fi.size() < 2)
            {
                ErrorString = "Bad format in file name record";
                qWarning() << ErrorString;
                return false;
            }
            Name = fi[1];
        }
        else if (r.startsWith("tags"))
        {
            QStringList t1 = r.split('\n');
            if (t1.size() > 0)
            {
                QStringList t2 = t1[1].split(':');
                for (int j = 0; j < t2.size(); j++)
                    Tags.push_back(t2[j]);
            }
        }
        else if (r.startsWith("info"))
        {
            r.remove("info\n");
            Description = r;
        }
    }

    return (!Name.isEmpty());
}

// -----------------------

bool AItemRecordDatabase::readFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open database file:" << fileName;
        return false;
    }

    QTextStream in(&file);
    QString records = in.readAll();

    const QStringList list = records.split("#end");

    for (size_t i = 0; i < list.size(); i++)
    {
        AItemRecord item;
        if (item.readFromRecord(list[i]))
        {
            Items.push_back(item);
            for (size_t j = 0; j < item.Tags.size(); j++)
                if (!Tags.contains(item.Tags[j])) Tags.append(item.Tags[j]);
        }
    }

    //qDebug() << "Extracted"<<Examples.size()<<"examples";
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
