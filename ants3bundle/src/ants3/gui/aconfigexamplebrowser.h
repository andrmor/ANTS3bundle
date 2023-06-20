#ifndef ACONFIGEXAMPLEBROWSER_H
#define ACONFIGEXAMPLEBROWSER_H

#include <QMainWindow>

namespace Ui {
class AConfigExampleBrowser;
}

class AConfigExampleItem
{
public:
    QString FileName;
    QString Description;
    std::vector<QString> Tags;
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

class AConfigExampleBrowser : public QMainWindow
{
    Q_OBJECT

public:
    explicit AConfigExampleBrowser(QWidget * parent = nullptr);
    ~AConfigExampleBrowser();

private slots:
    void on_pbReadDatabase_clicked();
    void on_pbLoadExample_clicked();

private:
    Ui::AConfigExampleBrowser * ui;

    AConfigExampleBranch MainBranch;

    QString readDatabase(QString fileName);
    QString extractItem(const QString & line, AConfigExampleBranch * currentBranch);
    QString extractBranch(const QString & line, AConfigExampleBranch * currentBranch, int & currentlevel);
};

#endif // ACONFIGEXAMPLEBROWSER_H
