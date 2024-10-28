#include "ascriptexampleexplorer.h"
#include "ui_ascriptexampleexplorer.h"
#include "ascriptexampledatabase.h"
#include "guitools.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QListWidgetItem>

AScriptExampleExplorer::AScriptExampleExplorer(QString recordsFileName, QString pathToExample, QWidget * parent) :
    QMainWindow(parent), Path(pathToExample),
    ui(new Ui::AScriptExampleExplorer)
{
    ui->setupUi(this);
    setWindowTitle("Script example explorer");

    QFile file(recordsFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open example list file:\n"+recordsFileName;
        close();
        return;
    }

    QTextStream in(&file);
    QString Records = in.readAll();
    ExampleDatabase = new AScriptExampleDatabase(Records);

    QObject::connect(ui->lwExamples, &QListWidget::currentRowChanged, this, &AScriptExampleExplorer::onExampleSelected);
    QObject::connect(ui->lwExamples, &QListWidget::itemDoubleClicked, this, &AScriptExampleExplorer::on_pbLoad_clicked);

    //showing tags - they will not be modified, only checked/unchecked
    BulkUpdate = true;
    for (int i = 0; i < ExampleDatabase->Tags.size(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(ExampleDatabase->Tags.at(i), ui->lwTags);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);        
    }
    BulkUpdate = false;

    connect(ui->lwTags, &QListWidget::itemChanged, this, &AScriptExampleExplorer::onTagStateChanged);
    connect(ui->leFind, &QLineEdit::textChanged, this, &AScriptExampleExplorer::onFindTextChanged);
    updateGui();

    QTimer::singleShot(100, this, &AScriptExampleExplorer::clearSelection);
}

AScriptExampleExplorer::~AScriptExampleExplorer()
{
    delete ui;
    delete ExampleDatabase;
}

void AScriptExampleExplorer::updateGui()
{    
    QStringList SelectedTags;
    for (int i = 0; i < ui->lwTags->count(); i++)
        if (ui->lwTags->item(i)->checkState() == Qt::Checked) SelectedTags.append(ui->lwTags->item(i)->text());
    ExampleDatabase->select(SelectedTags);

    ui->lwExamples->clear();

    for (int i=0; i<ExampleDatabase->size(); i++)
    {
        if (ExampleDatabase->Examples.at(i).Selected)
            ui->lwExamples->addItem(ExampleDatabase->Examples.at(i).FileName);
    }
}

void AScriptExampleExplorer::clearSelection()
{
    ui->lwExamples->clearSelection();
    ui->lwExamples->setCurrentRow(-1);
    ui->leFileName->clear();
    ui->teDescription->clear();
    ui->tePreview->clear();

    SelectedExampleIndex = -1;
}

void AScriptExampleExplorer::onExampleSelected(int row)
{
    if (row < 0)
    {
        clearSelection();
        return;
    }

    QString fn = ui->lwExamples->currentItem()->text();
    SelectedExampleIndex = ExampleDatabase->find(fn);
    if (SelectedExampleIndex < 0)
    {
        clearSelection();
        qWarning() << "Example not found!";
        return;
    }

    const AScriptExample & ex = ExampleDatabase->Examples[SelectedExampleIndex];
    ui->leFileName->setText(ex.FileName);
    ui->teDescription->setText(ex.Description);

    QFileInfo fileInfo(ex.FileName);
    if (fileInfo.suffix().toLower() == "json")
    {
        ui->tePreview->setText("This is a scrit book, cannot show preview");
    }
    else
    {
        QString script;
        QFile file(Path + ex.FileName);
        if (!file.open(QIODevice::ReadOnly))
            script = "Failed to open example file";
        else
        {
            QTextStream in(&file);
            script = in.readAll();
        }
        ui->tePreview->setText(script);
    }
}

void AScriptExampleExplorer::on_pbLoad_clicked()
{
    if (SelectedExampleIndex < 0)
    {
        guitools::message("Select an example to load", this);
        return;
    }

    emit requestLoadScript(Path + ExampleDatabase->Examples[SelectedExampleIndex].FileName);
    close();
}

void AScriptExampleExplorer::on_pbSelectAllTags_clicked()
{
    for (int i = 0; i < ui->lwTags->count(); i++)
        ui->lwTags->item(i)->setCheckState(Qt::Checked);
}

void AScriptExampleExplorer::on_pbUnselectAllTags_clicked()
{
    for (int i = 0; i < ui->lwTags->count(); i++)
        ui->lwTags->item(i)->setCheckState(Qt::Unchecked);
}

void AScriptExampleExplorer::onTagStateChanged(QListWidgetItem* /*item*/)
{
    if (BulkUpdate) return;
    updateGui();
}

void AScriptExampleExplorer::onFindTextChanged(const QString &text)
{
    const bool bEmpty = text.isEmpty();

    for (int i = 0; i < ui->lwTags->count(); i++)
    {
        bool bVisible = bEmpty || ui->lwTags->item(i)->text().contains(text, Qt::CaseInsensitive);

        ui->lwTags->item(i)->setHidden( !bVisible );
        ui->lwTags->item(i)->setCheckState( bVisible ? Qt::Checked : Qt::Unchecked );
    }
}

void AScriptExampleExplorer::on_lwTags_itemDoubleClicked(QListWidgetItem *item)
{
    for (int i = 0; i < ui->lwTags->count(); i++)
        if (ui->lwTags->item(i) == item)
            ui->lwTags->item(i)->setCheckState(Qt::Checked);
        else
            ui->lwTags->item(i)->setCheckState(Qt::Unchecked);
}
