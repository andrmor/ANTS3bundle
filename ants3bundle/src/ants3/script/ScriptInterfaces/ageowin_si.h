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

    AScriptInterface * cloneBase() const {return new AGeoWin_SI(GeometryWindow);}

public slots:
    void BlockUpdates(bool on);

    void setZoom(int level);
    void setParallel(bool on);
    void updateView();

    void showGeometry();
//    void showPMnumbers(); // need?
    void showTracks();

    void clearTracks();
    void clearMarkers();

    void saveImage(QString fileName);

//    int  AddTrack();
//    void AddNodeToTrack(int trk, float x, float y, float z);
//    void DeleteAllTracks();

    void addMarkers(QVariantList XYZs, int color, int style, double size);

private:
    AGeometryWindow * GeometryWindow = nullptr;
};

#endif // AGEOWIN_SI_H
