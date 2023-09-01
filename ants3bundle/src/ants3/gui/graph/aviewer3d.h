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

    std::array<size_t, 3> NumBins;
    std::array<double, 3> Scaling_mmPerPixel;
    std::array<double, 3> Offset;

    std::array<double, 3> StartZeroBin;

    std::vector<std::vector<std::vector<double>>> Data; // Data[iz][iy][ix]

    enum EMaximumMode {IndividualMax, GlobalMax, FixedMax};
    EMaximumMode MaximumMode = IndividualMax;
    double FixedMaximum = 0;
    double GlobalMaximum = 0;

    double ScalingFactor = 1.0;

    std::vector<std::pair<QString,int>> Palettes;

    double binToEdgePosition(size_t iDimension, size_t iBin) const;
    double binToCenterPosition(size_t iDimension, size_t iBin) const;

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

    bool doLoadCastorImage(const QString & fileName);
    void createViewWidgets();
    void updateGui();
    bool extractDoubleFromPair(const QStringList & twoFields, const QString & identifierTxt, std::array<double, 3> & array); // false on error
};

#endif // AVIEWER3D_H
