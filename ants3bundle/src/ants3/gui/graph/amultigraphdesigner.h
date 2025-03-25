#ifndef AMULTIGRAPHDESIGNER_H
#define AMULTIGRAPHDESIGNER_H

#include "apadgeometry.h"
#include "apadproperties.h"

#include <QMainWindow>

#include <vector>

class ABasketManager;
class ARasterWindow;
class ABasketListWidget;
class ADrawObject;
class QJsonObject;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

namespace Ui {
class AMultiGraphDesigner;
}

class AMultiGraphDesigner : public QMainWindow
{
    Q_OBJECT

public:
    explicit AMultiGraphDesigner(ABasketManager & Basket, QWidget * parent = nullptr);
    ~AMultiGraphDesigner();

    void updateBasketGUI();  // triggered from GraphWindow::UpdatebasketGui()
    void requestAutoconfigureAndDraw(const std::vector<int> & basketItems);

    void updateGUI();

private slots:
    void on_pbRefactor_clicked();
    void on_pbClear_clicked();

    void on_actionAs_pdf_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();

    void onCoordItemDoubleClicked(QListWidgetItem *);
    void onBasketItemDoubleClicked(QListWidgetItem *);

    void on_cbEnforceMargins_clicked();
    void on_ledLeft_editingFinished();
    void on_ledRight_editingFinished();
    void on_ledTop_editingFinished();
    void on_ledBottom_editingFinished();

    void on_cbScaleLabels_clicked();

    void on_ledScaleFactorForlabel_editingFinished();

    void on_cbScaleXoffsets_clicked();

    void on_cbScaleYoffsets_clicked();

    void on_ledScaleFactorForXoffset_editingFinished();

    void on_ledScaleFactorForYoffset_editingFinished();

protected:
    bool event(QEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

signals:
    void basketChanged();

private:
    ABasketManager & Basket;

    Ui::AMultiGraphDesigner * ui;
    ARasterWindow   * RasterWindow = nullptr;
    ABasketListWidget       * lwBasket     = nullptr;

    std::vector<APadProperties> Pads;
    std::vector<int>            DrawOrder;
    bool bColdStart = true;

    void clearGraphs();

    void updateCanvas();
    void updateNumbers();

    void drawGraph(const std::vector<ADrawObject> & DrawObjects, APadProperties & pad);
    void fillOutBasicLayout(int numX, int numY);
    void writeToJson(QJsonObject &json);
    QString readFromJson(const QJsonObject &json);
    QString PadsToString();
    void addDraw(QListWidget * lw);
    void checkMargin(QLineEdit * le);
};

#endif // AMULTIGRAPHDESIGNER_H
