#ifndef AVIEWER3D_H
#define AVIEWER3D_H

#include <QMainWindow>

#include <array>
#include <vector>

namespace Ui {
class AViewer3D;
}

class AViewer3DWidget;

class AViewer3D : public QMainWindow
{
    Q_OBJECT

public:
    explicit AViewer3D(QWidget * parent, const QString & castorFileName);
    ~AViewer3D();

    QString ErrorString;

    std::array<size_t, 3> NumBins;
    std::array<double, 3> Scaling_mmPerPixel;
    std::array<double, 3> Offset;

    std::vector<std::vector<std::vector<double>>> Data; // Data[iz][iy][ix]

    enum EMaximumMode {IndividualMax, GlobalMax, FixedMax};
    EMaximumMode MaximumMode = IndividualMax;
    double FixedMaximum = 0;
    double GlobalMaximum = 0;

    double ScalingFactor = 1.0;

    std::vector<std::pair<QString,int>> Palettes;

private slots:
    void on_cobPalette_currentTextChanged(const QString &arg1);

    void on_cobMaximum_activated(int index);

    void on_ledMaximum_editingFinished();

    void on_ledScaling_editingFinished();

private:
    Ui::AViewer3D * ui = nullptr;

    bool StartUp = true;

    AViewer3DWidget * View1 = nullptr;
    AViewer3DWidget * View2 = nullptr;
    AViewer3DWidget * View3 = nullptr;

    bool loadCastorImage(const QString & fileName);
    void initWidgets();
    void updateGui();
};

#endif // AVIEWER3D_H
