#ifndef AGEOWIN_SI_H
#define AGEOWIN_SI_H

#include "awindowinterfacebase.h"

#include <QVariantList>
#include <QString>

class AGeometryWindow;

class AGeoWin_SI : public AWindowInterfaceBase
{
    Q_OBJECT

public:
    AGeoWin_SI(AGeometryWindow * geoWin);

public slots:
    void BlockUpdates(bool on); //forbids updates

    void setZoom(int level);
    void setParallel(bool on);
    void updateView();

    void showGeometry();
    void showPMnumbers();
//    void ShowTracks(int num, int OnlyColor = -1);

    void clearTracks();
    void clearMarkers();

    void saveImage(QString fileName);

//    int  AddTrack();
//    void AddNodeToTrack(int trk, float x, float y, float z);
//    void DeleteAllTracks();

    void addMarkers(QVariantList XYZs, int color);

private:
    AGeometryWindow * GeometryWindow = nullptr;
};

#endif // AGEOWIN_SI_H
