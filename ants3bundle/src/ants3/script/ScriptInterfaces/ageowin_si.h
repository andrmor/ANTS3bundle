#ifndef AGEOWIN_SI_H
#define AGEOWIN_SI_H

#include "awindowinterfacebase.h"

#include <QVariantList>
#include <QString>

class AScriptHub;
class AGeometryWindow;
class AGeoMarkerClass;
class TGeoTrack;

class AGeoWin_SI : public AWindowInterfaceBase
{
    Q_OBJECT

public:
    AGeoWin_SI(AGeometryWindow * geoWin);

    AScriptInterface * cloneBase() const {return new AGeoWin_SI(GeometryWindow);}

    void updateGeoWin(AGeometryWindow * newGeoWin);

public slots:
    void redraw();

    void showTracksAndMarkers();

    void clearTracks();
    void clearMarkers();

    void saveImage(QString fileName);

    void addMarkers(QVariantList XYZs, int color, int style, double size);
    void addTrack(QVariantList XYZs, int color, int style, int width);

    /*
    void setZoom(int level);
    void setParallel(bool on);
    void updateView(); // !!!*** case of JSROOT
    */

private:
    AScriptHub      & ScrHub;
    AGeometryWindow * GeometryWindow = nullptr;

};

#endif // AGEOWIN_SI_H
