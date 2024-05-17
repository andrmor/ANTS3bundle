#ifndef AVIEWER3DWIDGET_H
#define AVIEWER3DWIDGET_H

#include "aviewer3d.h"

#include <QWidget>

//class RasterWindowBaseClass;
class RasterWindowGraphClass;
class TH2D;

namespace Ui {
class AViewer3DWidget;
}

class AViewer3DWidget : public QWidget
{
    Q_OBJECT

public:
    enum EViewType {XY, XZ, YZ};

    AViewer3DWidget(AViewer3D * viewer, EViewType viewType);
    ~AViewer3DWidget();

    bool init();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

public slots:
    void redraw();
    void requestShowCrossHair(double x, double y, double z);

private slots:
    void onRasterCursorPositionChanged(double x, double y, bool bOn);
    void onCursorLeftRaster();

    void on_sbPosition_valueChanged(int arg1);

    void on_pbRedraw_clicked();
    void on_pbMinus_clicked();
    void on_pbPlus_clicked();
    void on_hsPosition_sliderMoved(int position);
    void on_pbUnzoom_clicked();

    void on_ledPosition_editingFinished();

    void on_pbZoom_clicked();

private:
    AViewer3D * Viewer = nullptr;
    const AViewer3DSettings & Settings;
    EViewType ViewType;

    Ui::AViewer3DWidget * ui = nullptr;

    //RasterWindowBaseClass * RasterWindow = nullptr;
    RasterWindowGraphClass * RasterWindow = nullptr;

    TH2D * Hist = nullptr;

    void showCrossHair(double hor, double vert);
    void getShownHistRange(double & xfrom, double & yfrom, double & xto, double & yto) const;
    void applyShownHistRange(double & xfrom, double & yfrom, double & xto, double & yto);
    double getAveragedValue(int ixCenter, int iyCenter, int izCenter);

signals:
    void cursorPositionChanged(double x, double y, double z, double val);
    void cursorLeftVisibleArea();

};

#endif // AVIEWER3DWIDGET_H
