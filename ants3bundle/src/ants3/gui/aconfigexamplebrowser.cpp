#include "aconfigexamplebrowser.h"
#include "ui_aconfigexamplebrowser.h"
#include "afiletools.h"
#include "guitools.h"
#include "a3global.h"

AConfigExampleBranch::~AConfigExampleBranch()
{
    clear();
}

void AConfigExampleBranch::clear()
{
    Title.clear();
    Items.clear();

    ParentBranch = nullptr;
    for (AConfigExampleBranch * b : SubBranches) delete b;
    SubBranches.clear();
}

AConfigExampleBrowser::AConfigExampleBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AConfigExampleBrowser)
{
    ui->setupUi(this);
    setWindowTitle("Config example browser");

    const A3Global & GlobSet = A3Global::getConstInstance();
    QString fn = GlobSet.ExamplesDir + "/ConfigExamples.txt";
    QString err = readDatabase(fn);
    if (!err.isEmpty()) qWarning() << err;
    else
    {
        updateTableWidget();
        ui->trwExamples->expandAll();
    }
}

AConfigExampleBrowser::~AConfigExampleBrowser()
{
    delete ui;
}

QString AConfigExampleBrowser::readDatabase(QString fileName)
{
    QString txt;
    bool ok = ftools::loadTextFromFile(txt, fileName);
    if (!ok) return "Failed to read database file: " + fileName;

    MainBranch.clear();
    AConfigExampleBranch * currentBranch = &MainBranch;
    int currentLevel = 1; // '*' --> 1; '**' --> 2 etc

    const QStringList sl = txt.split('\n');
    for (const QString & str : sl)
    {
        QString line = str.simplified();
        if (line.isEmpty()) continue;
        if (!line.startsWith('*') && !line.startsWith('@')) return "Format error in database file: records should start with * or @ symbol";

        if (line.startsWith('@'))
        {
            QString err = extractItem(line, currentBranch);
            if (!err.isEmpty()) return err;
        }

        if (line.startsWith('*'))
        {
            QString err = extractBranch(line, currentBranch, currentLevel);
            if (!err.isEmpty()) return err;
        }
    }

    return "";
}

QString AConfigExampleBrowser::extractItem(const QString &line, AConfigExampleBranch * currentBranch)
{
    int iChar = 1;
    while (iChar < line.size() && line[iChar] == ' ') iChar++;
    if (iChar >= line.size()) return "Cannot find filename in record: " + line;

    AConfigExampleItem item;

    while (iChar < line.size() && line[iChar] != ' ')
    {
        item.FileName += line[iChar];
        iChar++;
    }

    iChar++;
    if (iChar >= line.size()) return "Cannot find example description in record: " + line;

    while (iChar < line.size() && line[iChar] != '#')
    {
        item.Description += line[iChar];
        iChar++;
    }

    if (item.Description.simplified().isEmpty()) return "Cannot find example description in record: " + line;

    while (iChar < line.size())
        if (line[iChar] == '#')
        {
            QString tag;
            iChar++;
            while (iChar < line.size() && line[iChar] != '#')
            {
                tag += line[iChar];
                iChar++;
            }
            if (tag.endsWith(' ')) tag.chop(1);
            if (!tag.isEmpty()) item.Tags.push_back(tag); // !!!*** check uniqness?
        }

    //qDebug() << item.FileName << item.Description << item.Tags;
    currentBranch->Items.push_back(item);

    return "";
}

QString AConfigExampleBrowser::extractBranch(const QString & line, AConfigExampleBranch* & currentBranch, int & currentLevel)
{
    int iChar = 0;
    int extractedLevel = 0;
    while (iChar < line.size() && line[iChar] == '*')
    {
        extractedLevel++;
        iChar++;
    }
    if (iChar >= line.size()) return "Cannot find branch title in record: " + line;

    while (iChar < line.size() && line[iChar] == ' ') iChar++;
    if (iChar >= line.size()) return "Cannot find branch title in record: " + line;

    QString title;
    while (iChar < line.size())
    {
        title += line[iChar];
        iChar++;
    }
    if (title.isEmpty()) return "Branch title is empty in record: " + line;

    if (extractedLevel < 1) return "Branch level cannot be less than 1 (at least one '*' char in record)";
    if (extractedLevel > currentLevel+1) return "Branch level cannot exceed the previous one by more than 1";

    AConfigExampleBranch * newBranch = new AConfigExampleBranch();
    newBranch->Title = title;

    AConfigExampleBranch * containerBranch = nullptr;
    if (extractedLevel < currentLevel)
    {
        containerBranch = currentBranch->ParentBranch;
        for (int i = 0; i < (currentLevel - extractedLevel); i++)
            containerBranch = containerBranch->ParentBranch;
    }
    else if (extractedLevel == currentLevel)
    {
        if (extractedLevel == 1)
            containerBranch = &MainBranch;
        else
            containerBranch = currentBranch->ParentBranch;
    }
    else // extractedLevel is currentLevel+1
    {
        containerBranch = currentBranch;
    }

    if (!containerBranch) return "Something went wrong in parent branch search algorithm";
    newBranch->ParentBranch = containerBranch;
    containerBranch->SubBranches.push_back(newBranch);

    currentBranch = newBranch;
    currentLevel = extractedLevel;
    return "";
}

void AConfigExampleBrowser::updateTableWidget()
{
    ui->trwExamples->clear();

    for (AConfigExampleBranch * br : MainBranch.SubBranches)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        //QFont font = item->font(0); font.setBold(true); item->setFont(0, font);
        QFont font = item->font(0); font.setPointSize(font.pointSize() + 2); item->setFont(0, font);
        ui->trwExamples->addTopLevelItem(item);
        fillTableRecursively(br, item);
    }
}

void AConfigExampleBrowser::fillTableRecursively(AConfigExampleBranch * branch, QTreeWidgetItem * item)
{
    item->setText(0, branch->Title);

    for (AConfigExampleBranch * subBranch : branch->SubBranches)
    {
        QTreeWidgetItem * newBranchItem = new QTreeWidgetItem();
        //QFont font = newBranchItem->font(0); font.setBold(true); newBranchItem->setFont(0, font);
        item->addChild(newBranchItem);
        fillTableRecursively(subBranch, newBranchItem);
    }

    for (const AConfigExampleItem & example : branch->Items)
    {
        QTreeWidgetItem * exampleItem = new QTreeWidgetItem({example.FileName, example.Description});
        QFont font = exampleItem->font(0); font.setBold(true); exampleItem->setFont(0, font);
        item->addChild(exampleItem);
    }
}

void AConfigExampleBrowser::on_trwExamples_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    ui->pteDescription->clear();
    ui->pbLoadExample->setEnabled(false);
    if (!current) return;

    QString description = current->text(1);
    bool ExampleSelected = !description.isEmpty();
    ui->pbLoadExample->setEnabled(ExampleSelected);
    if (ExampleSelected) ui->pteDescription->setPlainText(current->text(0) + ":\n" + description);
}

void AConfigExampleBrowser::on_pbLoadExample_clicked()
{
    QTreeWidgetItem * item = ui->trwExamples->currentItem();
    if (!item) return;
    if (item->text(1).isEmpty()) return;

    bool ok = guitools::confirm("Load this example?\nUnsaved changes in the current configuration will be lost!", this);
    if (!ok) return;

    QString fileName = item->text(0);
    close();
    emit requestLoadFile(fileName);
}

void AConfigExampleBrowser::on_trwExamples_itemDoubleClicked(QTreeWidgetItem * item, int)
{
    if (!item) return;
    if (item->text(1).isEmpty()) return;
    on_pbLoadExample_clicked();
}

