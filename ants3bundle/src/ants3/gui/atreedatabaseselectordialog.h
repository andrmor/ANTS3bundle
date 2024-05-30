#ifndef ATREEDATABASESELECTORDIALOG_H
#define ATREEDATABASESELECTORDIALOG_H

#include <QString>
#include <QDialog>

#include <vector>

class ATreeWidgetItem
{
public:
    QString Item;
    QString Description;
    std::vector<QString> Tags;
};

class ATreeWidgetBranch
{
public:
    ~ATreeWidgetBranch();

    QString Title;
    std::vector<ATreeWidgetItem> Items;

    ATreeWidgetBranch * Parent = nullptr;
    std::vector<ATreeWidgetBranch*> Children;

    void clear();
};


class QTreeWidgetItem;
class QTreeWidget;
class QPlainTextEdit;
class QPushButton;

class ATreeDatabaseSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ATreeDatabaseSelectorDialog(const QString & title, QWidget * parent = nullptr);
    virtual ~ATreeDatabaseSelectorDialog();

    QString readData(const QString & data); // return error or empty

    QString SelectedItem;

private slots:
    void onSelectClicked();

    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    ATreeWidgetBranch MainBranch;

    QTreeWidget    * TreeWidget = nullptr;
    QPlainTextEdit * TextField = nullptr;
    QPushButton    * SelectButton = nullptr;

    QString extractItem(const QString & line, ATreeWidgetBranch * currentBranch);
    QString extractBranch(const QString & line, ATreeWidgetBranch* & currentBranch, int & currentLevel);

    void updateTableWidget();
    void fillTableRecursively(ATreeWidgetBranch * branch, QTreeWidgetItem * item);

signals:
    void selectionCompleted(QString fileName);
};

#endif // ATREEDATABASESELECTORDIALOG_H
