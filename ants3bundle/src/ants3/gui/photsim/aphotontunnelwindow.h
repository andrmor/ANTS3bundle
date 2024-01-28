#ifndef APHOTONTUNNELWINDOW_H
#define APHOTONTUNNELWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class APhotonTunnelWindow;
}

class APhotonFunctionalHub;
class AGeometryHub;
class APhotonFunctionalModel;
class AFunctionalModelWidget;

class APhotonTunnelWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit APhotonTunnelWindow(const QString & idStr, QWidget * parent);
    ~APhotonTunnelWindow();

    void updateGui();

private slots:
    void on_pbAddModify_clicked();
    void on_pbRemove_clicked();
    void on_tabwConnections_cellClicked(int row, int column);

    void on_pbSelectModel_clicked();

    void onHeaderClicked(int index);

    void on_actionShow_all_linked_pairs_triggered();

    void on_cbShowConnection_clicked(bool checked);

    void on_pbCheck_clicked();

private:
    Ui::APhotonTunnelWindow *ui;
    APhotonFunctionalHub & PhFunHub;
    const AGeometryHub & GeoHub;

    APhotonFunctionalModel * LastModel = nullptr;
    AFunctionalModelWidget * LastWidget = nullptr;

    int SortByColumnIndex = 0;
    bool AscendingSortOrder = true;

    void fillCell(int iRow, int iColumn, const QString & txt, bool markNotValid);
    void onModelChanged();

signals:
    void requestShowConnection(int from, int to);
    void requestShowAllConnections();

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

#endif // APHOTONTUNNELWINDOW_H
