#ifndef AITEMSELECTIONDIALOG_H
#define AITEMSELECTIONDIALOG_H

#include <QWidget>
#include <QString>

namespace Ui { class AItemSelectionDialog; }

class AItemRecordDatabase;
class QListWidgetItem;

class AItemSelectionDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AItemSelectionDialog(const QString & databaseFileName, const QString & pathToItemFiles, const QString & title, QWidget * parent = nullptr);
    ~AItemSelectionDialog();

    void updateGui();

private:
    QString Path;
    AItemRecordDatabase * Data = nullptr;

    Ui::AItemSelectionDialog * ui = nullptr;

    bool BulkUpdate = false;

private slots:
    void on_pbLoad_clicked();
    void on_pbSelectAllTags_clicked();
    void on_pbUnselectAllTags_clicked();

    void clearSelection();
    void onExampleSelected(int row);

    void onTagStateChanged(QListWidgetItem * item);
    void onFindTextChanged(const QString & text);
    void on_lwTags_itemDoubleClicked(QListWidgetItem * item);
};

#include <QStringList>

class AItemRecord
{
public:
    bool readFromRecord(const QString & text);

    QString Name;
    QString FileName;
    QStringList Tags;
    QString Description;

    bool Selected = true;

    QString ErrorString;
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
