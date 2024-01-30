#include "atreedatabaseselectordialog.h"
#include "guitools.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QPushButton>

ATreeWidgetBranch::~ATreeWidgetBranch()
{
    clear();
}

void ATreeWidgetBranch::clear()
{
    Title.clear();
    Items.clear();

    Parent = nullptr;
    for (ATreeWidgetBranch * b : Children) delete b;
    Children.clear();
}

ATreeDatabaseSelectorDialog::ATreeDatabaseSelectorDialog(const QString & title, QWidget * parent) :
    QDialog(parent)
{
    setModal(true);
    setWindowTitle(title);
    resize(800, 600);

    QVBoxLayout * main = new QVBoxLayout(this);

    TreeWidget = new QTreeWidget(this);
    main->addWidget(TreeWidget);

    TextField = new QPlainTextEdit(this);
    TextField->appendPlainText("Data not loaded");
    main->addWidget(TextField);

    SelectButton = new QPushButton("Select", this);
    main->addWidget(SelectButton);

    connect(TreeWidget, &QTreeWidget::currentItemChanged, this, &ATreeDatabaseSelectorDialog::onCurrentItemChanged);
    connect(TreeWidget, &QTreeWidget::itemDoubleClicked, this, &ATreeDatabaseSelectorDialog::onItemDoubleClicked);
    connect(SelectButton, &QPushButton::clicked, this, &ATreeDatabaseSelectorDialog::onSelectClicked);
}

ATreeDatabaseSelectorDialog::~ATreeDatabaseSelectorDialog()
{
    MainBranch.clear();
}

QString ATreeDatabaseSelectorDialog::readData(const QString & data)
{
    MainBranch.clear();

    ATreeWidgetBranch * currentBranch = &MainBranch;
    int currentLevel = 1; // '*' --> 1; '**' --> 2 etc

    const QStringList sl = data.split('\n');
    for (const QString & str : sl)
    {
        QString line = str.simplified();
        if (line.isEmpty()) continue;
        if (!line.startsWith('*') && !line.startsWith('@')) return "Format error in data: records should start with * or @ symbol";

        if (line.startsWith('@'))
        {
            QString err = extractItem(line, currentBranch);
            if (!err.isEmpty()) return err;
        }
        else // if (line.startsWith('*'))
        {
            QString err = extractBranch(line, currentBranch, currentLevel);
            if (!err.isEmpty()) return err;
        }
    }

    updateTableWidget();
    TreeWidget->expandAll();

    return "";
}

QString ATreeDatabaseSelectorDialog::extractItem(const QString & line, ATreeWidgetBranch * currentBranch)
{
    int iChar = 1;
    while (iChar < line.size() && line[iChar] == ' ') iChar++;
    if (iChar >= line.size()) return "Cannot find item text in record: " + line;

    ATreeWidgetItem item;

    while (iChar < line.size() && line[iChar] != ' ')
    {
        item.Item += line[iChar];
        iChar++;
    }

    iChar++;
    if (iChar >= line.size()) return "Cannot find description in record: " + line;

    while (iChar < line.size() && line[iChar] != '#')
    {
        item.Description += line[iChar];
        iChar++;
    }

    if (item.Description.simplified().isEmpty()) return "Cannot find description in record: " + line;

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

    currentBranch->Items.push_back(item);

    return "";
}

QString ATreeDatabaseSelectorDialog::extractBranch(const QString & line, ATreeWidgetBranch* & currentBranch, int & currentLevel)
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

    ATreeWidgetBranch * newBranch = new ATreeWidgetBranch();
    newBranch->Title = title;

    ATreeWidgetBranch * containerBranch = nullptr;
    if (extractedLevel < currentLevel)
    {
        containerBranch = currentBranch->Parent;
        for (int i = 0; i < (currentLevel - extractedLevel); i++)
            containerBranch = containerBranch->Parent;
    }
    else if (extractedLevel == currentLevel)
    {
        if (extractedLevel == 1)
            containerBranch = &MainBranch;
        else
            containerBranch = currentBranch->Parent;
    }
    else // extractedLevel is currentLevel+1
    {
        containerBranch = currentBranch;
    }

    if (!containerBranch) return "Something went wrong in parent branch search algorithm";
    newBranch->Parent = containerBranch;
    containerBranch->Children.push_back(newBranch);

    currentBranch = newBranch;
    currentLevel = extractedLevel;
    return "";
}

void ATreeDatabaseSelectorDialog::updateTableWidget()
{
    TreeWidget->clear();

    for (ATreeWidgetBranch * br : MainBranch.Children)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        //QFont font = item->font(0); font.setBold(true); item->setFont(0, font);
        QFont font = item->font(0); font.setPointSize(font.pointSize() + 2); item->setFont(0, font);
        TreeWidget->addTopLevelItem(item);
        fillTableRecursively(br, item);
    }
}

void ATreeDatabaseSelectorDialog::fillTableRecursively(ATreeWidgetBranch * branch, QTreeWidgetItem * item)
{
    item->setText(0, branch->Title);

    for (ATreeWidgetBranch * subBranch : branch->Children)
    {
        QTreeWidgetItem * newBranchItem = new QTreeWidgetItem();
        //QFont font = newBranchItem->font(0); font.setBold(true); newBranchItem->setFont(0, font);
        item->addChild(newBranchItem);
        fillTableRecursively(subBranch, newBranchItem);
    }

    for (const ATreeWidgetItem & branchItem : branch->Items)
    {
        QTreeWidgetItem * qItem = new QTreeWidgetItem({branchItem.Item, branchItem.Description});
        QFont font = qItem->font(0); font.setBold(true); qItem->setFont(0, font);
        item->addChild(qItem);
    }
}

void ATreeDatabaseSelectorDialog::onCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem *)
{
    TextField->clear();
    SelectButton->setEnabled(false);
    if (!current) return;

    QString description = current->text(1);
    bool ExampleSelected = !description.isEmpty();
    SelectButton->setEnabled(ExampleSelected);
    if (ExampleSelected) TextField->setPlainText(current->text(0) + ":\n" + description);
}

void ATreeDatabaseSelectorDialog::onSelectClicked()
{
    QTreeWidgetItem * item = TreeWidget->currentItem();
    if (!item) return;
    if (item->text(1).isEmpty()) return;

    SelectedItem = item->text(0);
    accept();
}

void ATreeDatabaseSelectorDialog::onItemDoubleClicked(QTreeWidgetItem * item, int)
{
    if (!item) return;
    if (item->text(1).isEmpty()) return;
    onSelectClicked();
}

