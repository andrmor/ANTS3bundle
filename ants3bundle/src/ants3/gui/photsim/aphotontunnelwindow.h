#ifndef APHOTONTUNNELWINDOW_H
#define APHOTONTUNNELWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class APhotonTunnelWindow;
}

class APhotonFunctionalHub;
class AGeometryHub;

class APhotonTunnelWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit APhotonTunnelWindow(const QString & idStr, QWidget * parent);
    ~APhotonTunnelWindow();

    void updateGui();

private slots:
    void on_rbSortByFrom_clicked(bool checked);
    void on_rbSortByTo_clicked(bool checked);
    void on_pbAddModify_clicked();
    void on_pbRemove_clicked();
    void on_tabwConnections_cellClicked(int row, int column);
    void on_pbShowAllConnections_clicked();

private:
    Ui::APhotonTunnelWindow *ui;
    APhotonFunctionalHub & PhTunHub;
    const AGeometryHub & GeoHub;

    void fillCell(int iRow, int iColumn, const QString & txt);
    void updateInfoLabels(); // !!!***

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
        lhs = text().toDouble();
        rhs = other.text().toDouble();
        return (lhs < rhs);
    }
};

#endif // APHOTONTUNNELWINDOW_H
