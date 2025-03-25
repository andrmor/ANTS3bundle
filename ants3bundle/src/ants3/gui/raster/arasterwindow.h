#ifndef ARASTERWINDOW_H
#define ARASTERWINDOW_H

#include <QWidget>

class TCanvas;
class QMainWindow;
class TView;

struct AGeoViewParameters
{
    double RangeLL[3];
    double RangeUR[3];

    double RotCenter[3];

    double WinX, WinY, WinW, WinH;

    double Long, Lat, Psi;

    void read(TCanvas * Canvas);
    void apply(TCanvas * Canvas) const;
};

class ARasterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ARasterWindow(QMainWindow * MasterWindow);
    virtual ~ARasterWindow();

    TCanvas * fCanvas = nullptr;
    AGeoViewParameters ViewParameters;

    void setAsActiveRootWindow();
    void updateRootCanvas();
    void clearRootCanvas();
    void forceResize();
    void setBlockEvents(bool flag) {BlockEvents = flag;}
    void saveAs(const QString & filename);
    void setWindowProperties();
    void onViewChanged();

signals:
    void LeftMouseButtonReleased();
    void userChangedWindow();

protected:
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event) override;

    void paintEvent(QPaintEvent * event ) override;
    void resizeEvent(QResizeEvent * event ) override;

    QPaintEngine * paintEngine() const override {return nullptr;}  // added due to setAttribute(Qt::WA_PaintOnScreen, true);

protected:
    bool   PressEventRegistered = false; //to avoid "leaking" events to another window
    int    LastX = 0;
    int    LastY = 0;
    double LastCenterX = 0;
    double LastCenterY = 0;
    bool   BlockEvents = false;
    bool   InvertedXYforDrag = false; // obsolete?
    bool   BlockZoom = false;

    void releaseZoomBlock() {BlockZoom = false;}
};

#endif // ARASTERWINDOW_H
