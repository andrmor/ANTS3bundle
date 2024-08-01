#ifndef AITEMSELECTIONDIALOG_H
#define AITEMSELECTIONDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui { class AItemSelectionDialog; }

class AItemRecordDatabase;
class QListWidgetItem;
class QJsonObject;

class AItemSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    AItemSelectionDialog(const QString & databaseFileName, const QString & pathToItemFiles, const QString & title, QWidget * parent = nullptr);
    ~AItemSelectionDialog();

    void updateGui();

    QString FileNameSelected;

private:
    QString Path;
    AItemRecordDatabase * Data = nullptr;

    Ui::AItemSelectionDialog * ui = nullptr;

    bool BulkUpdate = false;

private slots:
    void on_pbLoad_clicked();
    void on_pbSelectAllTags_clicked();
    void on_pbUnselectAllTags_clicked();
    void on_lwTags_itemDoubleClicked(QListWidgetItem * item);

    void clearSelection();
    void onExampleSelected(int row);

    void onTagStateChanged(QListWidgetItem * item);
    void onFindTextChanged(const QString & text);
};

#include <QStringList>

class AItemRecord
{
public:
    QString     Name;
    QString     File;
    QStringList Tags;
    QString     Description;

    bool        Selected = true;

    void readFromJson(const QJsonObject & json);
};


class AItemRecordDatabase
{
public:
    bool readFromFile(const QString & fileName);

    void select(const QStringList & tags);
    void unselectAll();
    int  size() const;
    int  findIndexByName(const QString & name) const;

    std::vector<AItemRecord> Items;
    QStringList Tags;
};

#endif // AITEMSELECTIONDIALOG_H
