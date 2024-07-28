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

    void updateGeoWin(AGeometryWindow * newGeoWin);

public slots:
    void redraw();
    void showTracks(); // !!!*** update to new sync system TODO

    /*
    void BlockUpdates(bool on);

    void setZoom(int level);
    void setParallel(bool on);
    void updateView(); // !!!*** case of JSROOT


    void clearTracks();
    void clearMarkers();

    void saveImage(QString fileName);

//    int  AddTrack();
//    void AddNodeToTrack(int trk, float x, float y, float z);  // change to doubles
//    void DeleteAllTracks();

    void addMarkers(QVariantList XYZs, int color, int style, double size);
*/

private slots:
    void onWindowReportTaskCompleted();

private:
    AGeometryWindow * GeometryWindow = nullptr;
    bool WaitingForTaskCompleted = false;


signals:
    void requestRedraw();
    void requestShowTracks();

};

#endif // AGEOWIN_SI_H
