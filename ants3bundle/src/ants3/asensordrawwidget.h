#ifndef ASENSORDRAWWIDGET_H
#define ASENSORDRAWWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QGraphicsScene>

#include <vector>

class ASensorGView;
class QLabel;
class QPushButton;
class QSpinBox;

namespace Ui {
class ASensorDrawWidget;
}

class ASensorDrawWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ASensorDrawWidget(const std::vector<float> & sensorSignals, QWidget *parent = nullptr);
    ~ASensorDrawWidget();

    void updateGui();

private:
    const std::vector<float> & SensorSignals;

    Ui::ASensorDrawWidget    * ui = nullptr;

    ASensorGView   * gvOut = nullptr;
    QGraphicsScene * scene = nullptr;

    double GVscale = 10.0;
    QList<QGraphicsItem*> grItems;
    bool bForbidUpdate = false;

    std::vector<QLabel*> Labels;

private slots:
    void on_pbResetView_clicked();

private:
    void clearGrItems();
    void updateLegend(float MaxSignal);
    void resetViewport();
    void addSensorItems(float MaxSignal);
    void addTextItems(float MaxSignal);

};

#endif // ASENSORDRAWWIDGET_H
