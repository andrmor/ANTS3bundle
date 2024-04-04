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
    explicit AViewer3D(QWidget * parent);
    ~AViewer3D();

    bool loadCastorImage(const QString & castorFileName);

    QString ErrorString;

    size_t NumBinsX, NumBinsY, NumBinsZ;
    double mmPerPixelX, mmPerPixelY, mmPerPixelZ;
    double OffsetX, OffsetY, OffsetZ;

    double StartZeroBinX, StartZeroBinY, StartZeroBinZ;

    std::vector<std::vector<std::vector<double>>> Data; // Data[ix][iy][iz]

    enum EMaximumMode {IndividualMax, GlobalMax, FixedMax};
    EMaximumMode MaximumMode = IndividualMax;
    double FixedMaximum = 0;
    double GlobalMaximum = 0;
    int    PercentFieldOfView = 90;

    double ScalingFactor = 1.0;

    std::vector<std::pair<QString,int>> Palettes;

    enum EAxis {Xaxis, Yaxis, Zaxis};
    double binToEdgePosition(EAxis axis, size_t iBin) const;
    double binToCenterPosition(EAxis axis, size_t iBin) const;

private slots:
    void on_cobPalette_currentTextChanged(const QString &arg1);

    void on_cobMaximum_activated(int index);

    void on_ledMaximum_editingFinished();

    void on_ledScaling_editingFinished();

    void on_sbMaxInFractionFoV_editingFinished();

private:
    Ui::AViewer3D * ui = nullptr;

    bool StartUp = true;

    AViewer3DWidget * View1 = nullptr;
    AViewer3DWidget * View2 = nullptr;
    AViewer3DWidget * View3 = nullptr;

    bool doLoadCastorImage(const QString & fileName);
    void createViewWidgets();
    void updateGui();
    void calculateGlobalMaximum();
    void configureConnections(AViewer3DWidget * from, AViewer3DWidget * to1, AViewer3DWidget * to2);
};

#endif // AVIEWER3D_H
