#ifndef ACONFIGEXAMPLEBROWSER_H
#define ACONFIGEXAMPLEBROWSER_H

#include <QMainWindow>
#include <QString>

namespace Ui {
class AConfigExampleBrowser;
}

class AConfigExampleItem
{
public:
    QString FileName;
    QString Description;
    std::vector<QString> Tags; // not implemented, seems no need
};

class AConfigExampleBranch
{
public:
    ~AConfigExampleBranch();

    QString Title;
    std::vector<AConfigExampleItem> Items;

    AConfigExampleBranch * ParentBranch;
    std::vector<AConfigExampleBranch*> SubBranches;

    void clear();
};

class QTreeWidgetItem;

class AConfigExampleBrowser : public QMainWindow
{
    Q_OBJECT

public:
    explicit AConfigExampleBrowser(QWidget * parent = nullptr);
    ~AConfigExampleBrowser();

    void expandAll(bool flag);

private slots:
    void on_pbLoadExample_clicked();

    void on_trwExamples_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_trwExamples_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::AConfigExampleBrowser * ui;

    AConfigExampleBranch MainBranch;

    QString readDatabase(QString fileName);
    QString extractItem(const QString & line, AConfigExampleBranch * currentBranch);
    QString extractBranch(const QString & line, AConfigExampleBranch* & currentBranch, int & currentLevel);

    void updateTableWidget();
    void fillTableRecursively(AConfigExampleBranch * branch, QTreeWidgetItem * item);

signals:
    void requestLoadFile(QString fileName);
};

#endif // ACONFIGEXAMPLEBROWSER_H
