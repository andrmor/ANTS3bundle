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

class ASensorDrawWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ASensorDrawWidget(QWidget * parent = nullptr);
    ~ASensorDrawWidget();

    void updateGui(const std::vector<float> & sensorSignals, const std::vector<int> & enabledSensors);
    void resetViewport();

private:
    Ui::ASensorDrawWidget * ui = nullptr;

    ASensorGView   * gvOut = nullptr;
    QGraphicsScene * scene = nullptr;

    double GVscale = 10.0;
    std::vector<QGraphicsItem*> grItems;
    bool bForbidUpdate = false;

    std::vector<QLabel*> Labels;

    std::vector<float> SensorSignals;
    std::vector<int>   EnabledSensors;

private slots:
    void on_pbResetView_clicked();
    void on_cobViewSelector_activated(int index);

private:
    void clearGrItems();
    void updateLegend(float MaxSignal);
    void addSensorItems(float MaxSignal);
    void addTextItems(float MaxSignal);
    void positionToSceneCoordinates(int iSens, double &x, double &y, double &z);
};

#endif // ASENSORDRAWWIDGET_H
