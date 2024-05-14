#ifndef AVIEWER3D_H
#define AVIEWER3D_H

#include "aviewer3dsettings.h"

#include <QMainWindow>

#include <array>
#include <vector>

namespace Ui {
class AViewer3D;
}

class AViewer3DWidget;
class QJsonObject;

class AViewer3D : public QMainWindow
{
    Q_OBJECT

public:
    explicit AViewer3D(QWidget * parent);
    ~AViewer3D();

    bool loadCastorImage(const QString & castorFileName);

    void initViewers();
    void updateGui();

    QString getTitle();
    void setTitle(const QString & title);

    AViewer3DSettings Settings;

    QString ErrorString;

    int NumBinsX, NumBinsY, NumBinsZ;
    double mmPerPixelX, mmPerPixelY, mmPerPixelZ;
    double OffsetX, OffsetY, OffsetZ;

    double StartZeroBinX, StartZeroBinY, StartZeroBinZ;

    std::vector<std::vector<std::vector<double>>> Data; // Data[ix][iy][iz]
    double GlobalMaximum = 0; // calculated by calculateGlobalMaximum()

    enum EAxis {Xaxis, Yaxis, Zaxis};
    double binToEdgePosition(EAxis axis, size_t iBin) const;
    double binToCenterPosition(EAxis axis, size_t iBin) const;

    void writeDataToJson(QJsonObject & json) const;
    void readDataFromJson(const QJsonObject & json);

    void writeViewersToJson(QJsonObject & json) const;
    void readViewersFromJson(const QJsonObject & json);

private slots:
    void on_actionShow_title_toggled(bool arg1);
    void on_actionMake_a_copy_triggered();

    void showSettings();

private:
    Ui::AViewer3D * ui = nullptr;

    AViewer3DWidget * View1 = nullptr;
    AViewer3DWidget * View2 = nullptr;
    AViewer3DWidget * View3 = nullptr;

    bool doLoadCastorImage(const QString & fileName);
    void createViewWidgets();
    void calculateGlobalMaximum();
    void configureConnections(AViewer3DWidget * from, AViewer3DWidget * to1, AViewer3DWidget * to2);

signals:
    void requestMakeCopy(AViewer3D * ptr);
};

#endif // AVIEWER3D_H
