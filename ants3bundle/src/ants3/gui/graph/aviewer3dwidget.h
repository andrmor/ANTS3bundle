#ifndef AVIEWER3DWIDGET_H
#define AVIEWER3DWIDGET_H

#include "aviewer3d.h"

#include <QWidget>

class RasterWindowBaseClass;
class TH2D;

namespace Ui {
class AViewer3DWidget;
}

class AViewer3DWidget : public QWidget
{
    Q_OBJECT

public:
    enum EViewType {XY, XZ, YZ};

    explicit AViewer3DWidget(AViewer3D * viewer, EViewType viewType);
    ~AViewer3DWidget();

public slots:
    void redraw();

private slots:
    void on_pbRedraw_clicked();

    void on_pbMinus_clicked();

    void on_pbPlus_clicked();

    void on_hsPosition_sliderMoved(int position);

    void on_pbUnzoom_clicked();

    void on_sbPosition_valueChanged(int arg1);

private:
    AViewer3D * Viewer = nullptr;
    EViewType ViewType;

    Ui::AViewer3DWidget * ui = nullptr;

    RasterWindowBaseClass * RasterWindow = nullptr;

    TH2D * Hist = nullptr;

};

#endif // AVIEWER3DWIDGET_H
