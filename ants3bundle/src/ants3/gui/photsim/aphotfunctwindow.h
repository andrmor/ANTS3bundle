#ifndef APHOTFUNCTWINDOW_H
#define APHOTFUNCTWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class APhotFunctWindow;
}

class APhotonFunctionalHub;
class AGeometryHub;
class APhotonFunctionalModel;
class AFunctionalModelWidget;
class TObject;

class APhotFunctWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit APhotFunctWindow(QWidget * parent = nullptr);
    ~APhotFunctWindow();

    void updateGui();

private slots:
    void setModifiedStatus(bool flag);

    void onHeaderClicked(int index);

    void on_pbAddModify_clicked();
    void on_pbResetToDefault_clicked();
    void on_tabwConnections_cellClicked(int row, int column);
    void on_actionShow_all_linked_pairs_triggered();
    void on_cbShowConnection_clicked(bool checked);
    void on_sbTo_textChanged(const QString &arg1);
    void on_sbFrom_textChanged(const QString &arg1);
    void on_leModelTypeName_textChanged(const QString &arg1);
    void on_actionReset_all_to_default_triggered();
    void on_actionRemove_records_with_invalid_index_triggered();
    void on_actionCheck_all_records_triggered();
    void on_pbHelp_clicked();

private:
    APhotonFunctionalHub & PhFunHub;
    const AGeometryHub & GeoHub;

    Ui::APhotFunctWindow * ui = nullptr;

    APhotonFunctionalModel * LocalModel = nullptr; // always clone to/fromn this pointer!
    AFunctionalModelWidget * LastWidget = nullptr;

    QBrush DefaultBrush;

    int SortByColumnIndex = 0;
    bool AscendingSortOrder = true;

    QPixmap RedCircle;

    void fillCell(int iRow, int iColumn, const QString & txt, bool markNotValid, bool bold);
    void onModelChanged();

signals:
    void requestShowConnection(int from, int to);
    void requestShowAllConnections();
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);

};

#include <QTableWidgetItem>
class ASortableTableWidgetItem : public QTableWidgetItem
{
public:
    bool operator< (const QTableWidgetItem & other) const
    {
        int lhs = 0, rhs = 0;
        bool ok1, ok2;
        lhs = text().toDouble(&ok1);
        rhs = other.text().toDouble(&ok2);
        if (ok1 && ok2) return (lhs < rhs);
        else return (text() < other.text());
    }
};

#endif // APHOTFUNCTWINDOW_H
