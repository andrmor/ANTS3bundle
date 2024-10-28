#ifndef ASCRIPTEXAMPLEEXPLORER_H
#define ASCRIPTEXAMPLEEXPLORER_H

#include <QMainWindow>

namespace Ui {
class AScriptExampleExplorer;
}

class AScriptExampleDatabase;
class QListWidgetItem;

class AScriptExampleExplorer : public QMainWindow
{
    Q_OBJECT

public:
    explicit AScriptExampleExplorer(QString recordsFileName, QString pathToExample, QWidget * parent);
    ~AScriptExampleExplorer();

    void updateGui();

private:
    QString Path;

    Ui::AScriptExampleExplorer * ui = nullptr;

    AScriptExampleDatabase * ExampleDatabase = nullptr;
    bool BulkUpdate = false;

    int SelectedExampleIndex = -1;

private slots:
    void clearSelection();
    void onExampleSelected(int row);

    void on_pbLoad_clicked();
    void on_pbSelectAllTags_clicked();
    void on_pbUnselectAllTags_clicked();

    void onTagStateChanged(QListWidgetItem * item);
    void onFindTextChanged(const QString & text);
    void on_lwTags_itemDoubleClicked(QListWidgetItem * item);

signals:
    void requestLoadScript(QString);
};

#endif // ASCRIPTEXAMPLEEXPLORER_H
