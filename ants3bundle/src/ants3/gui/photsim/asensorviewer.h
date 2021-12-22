#ifndef ASENSORVIEWER_H
#define ASENSORVIEWER_H

#include <QFrame>
#include <QGraphicsScene>

#include <vector>

class ASensorGView;
class QLabel;
class QPushButton;
class QSpinBox;

class ASensorViewer : public QFrame
{
    Q_OBJECT

public:
    ASensorViewer(const std::vector<float> & sensorSignals, QWidget * parent);

protected:
    const std::vector<float> & SensorSignals;

    QGraphicsScene * scene = nullptr;
    ASensorGView   * gvScale = nullptr;

    QGraphicsScene * scaleScene = nullptr;
    ASensorGView   * gvOut = nullptr;

    double GVscale = 10.0;
    QList<QGraphicsItem*> grItems;
    bool bForbidUpdate = false;

    QLabel * labMax = nullptr;
    QLabel * labHighMid = nullptr;
    QLabel * labMid     = nullptr;
    QLabel * labLowMid  = nullptr;

    QPushButton * pbResetView = nullptr;
    QSpinBox    * sbDecimals  = nullptr;

private slots:
    void onResetViewportClicked();

private:
    void clearGrItems();
    void updateGraphScene();
    void updateSignalLabels(float MaxSignal);
    void updateSignalScale();
    void resetViewport();
    void addSensorItems(float MaxSignal);
    void addTextItems(float MaxSignal);

};

#endif // ASENSORVIEWER_H
