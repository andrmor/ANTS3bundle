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

    bool  UseGlobalMaximum = true;
    float GlobalMaximum = 0;

private slots:
    void on_cbGlobalMaximum_clicked(bool checked);

private:
    Ui::AViewer3D * ui = nullptr;

    AViewer3DWidget * View1 = nullptr;
    AViewer3DWidget * View2 = nullptr;
    AViewer3DWidget * View3 = nullptr;

    bool loadCastorImage(const QString & fileName);
    void initWidgets();
    void updateGui();
};

#endif // AVIEWER3D_H
